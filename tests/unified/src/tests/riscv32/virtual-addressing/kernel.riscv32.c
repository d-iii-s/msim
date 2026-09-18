#include <kernel.h>
#include <msim.h>

#include "addresses.h"

/* Sv32 PTE bits (bits 9:0), see the RISC-V privileged spec. */
#define PTE_V (1u << 0)
#define PTE_R (1u << 1)
#define PTE_W (1u << 2)
#define PTE_X (1u << 3)
#define PTE_U (1u << 4)
#define PTE_A (1u << 6)
#define PTE_D (1u << 7)

#define PAGE_SIZE 4096u

#define KERNEL_BASE  0x80000000u
#define STACK_BASE   0x8D000000u
#define PRINTER_BASE 0x90000000u

/* The rest of the "interesting" pages, all inside one dedicated megapage
 * together with APP_ADDRESS/SUPERVISOR_ONLY_PAGE_ADDR/USER_ACCESSIBLE_PAGE_ADDR
 * (addresses.h) so that they all share one leaf table (test_leaf_table). */
#define TEST_MEGAPAGE_BASE         0x10000000u
#define READ_ONLY_PAGE_ADDR        0x10001000u
#define EXECUTE_ONLY_PAGE_ADDR     0x10002000u
#define NX_PAGE_ADDR               0x10004000u
#define RESUME_PAGE_ADDR           0x10005000u
#define NX_USER_PAGE_ADDR          0x10006000u
#define RESUME_USER_PAGE_ADDR      0x10007000u
#define NON_LEAF_SECOND_LEVEL_ADDR 0x103FE000u
#define INVALID_SECOND_LEVEL_ADDR  0x103FF000u

/* Two more deliberately broken mappings, each in its own otherwise-unused
 * megapage. */
#define UNALIGNED_MEGAPAGE_ADDR  0x20000000u
#define INVALID_FIRST_LEVEL_ADDR 0x20400000u

#define MSTATUS_MPP_S (1u << 11)
#define SSTATUS_SPP   (1u << 8)
#define SSTATUS_SUM   (1u << 18)
#define SSTATUS_MXR   (1u << 19)

/* The "memory" clobber matters here: without it, nothing stops the
 * compiler from reordering the page-table stores in setup_paging()
 * relative to the csrw/mret below that make them take effect. */
#define csr_write(csr, val) __asm__ volatile("csrw " #csr ", %0" ::"r"((unsigned int) (val)) : "memory")
#define csr_set(csr, val) __asm__ volatile("csrs " #csr ", %0" ::"r"((unsigned int) (val)) : "memory")
#define csr_clear(csr, val) __asm__ volatile("csrc " #csr ", %0" ::"r"((unsigned int) (val)) : "memory")

/*
 * Every trap the tests below deliberately cause is routed here. It just
 * reports the trap and resumes right after the faulting instruction, so
 * a test reads as "this must trap" / "this must not trap" rather than
 * "this must trap to exactly this handler". It only touches t0/t1, which
 * it saves/restores itself -- unlike hand-written assembly, the C code
 * calling into a trap site doesn't tell us which registers are free.
 */
extern void resuming_trap_handler(void);

__asm__(
        ".global resuming_trap_handler\n"
        "resuming_trap_handler:\n"
        "    addi sp, sp, -8\n"
        "    sw t0, 0(sp)\n"
        "    sw t1, 4(sp)\n"
        "    li t0, 0x90000000\n"
        "    li t1, 'T'\n"
        "    sw t1, 0(t0)\n"
        "    csrr t0, mepc\n"
        "    addi t0, t0, 4\n"
        "    csrw mepc, t0\n"
        "    lw t1, 4(sp)\n"
        "    lw t0, 0(sp)\n"
        "    addi sp, sp, 8\n"
        "    mret\n");

/* Provided by kernel.riscv32.lds: end of the whole linked kernel image. */
extern char __kernel_end[];

/* Root table, and the one leaf table backing the kernel's own identity
 * mapping (the whole kernel image is well under 4 MiB, so it always
 * fits in a single megapage). */
static volatile unsigned int root_table[1024] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned int leaf_table[1024] __attribute__((aligned(PAGE_SIZE)));

/* Leaf table for TEST_MEGAPAGE_BASE: one entry per "interesting" page. */
static volatile unsigned int test_leaf_table[1024] __attribute__((aligned(PAGE_SIZE)));

/* Backing storage for the individually-permissioned test pages. Their own
 * (identity-mapped) address is irrelevant -- what matters is which fixed
 * virtual address each gets mapped to in test_leaf_table, below. */
static volatile unsigned char rwx_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char ro_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char xo_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char u_rwx_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char nx_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char resume_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char nx_user_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static volatile unsigned char resume_user_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

static inline unsigned int mega_index(unsigned int va)
{
    return va >> 22;
}

static inline unsigned int leaf_index(unsigned int va)
{
    return (va >> 12) & 0x3FFu;
}

static inline unsigned int leaf_pte(unsigned int phys_addr, unsigned int flags)
{
    return ((phys_addr >> 12) << 10) | PTE_V | flags;
}

static inline unsigned int nonleaf_pte(unsigned int phys_addr)
{
    return ((phys_addr >> 12) << 10) | PTE_V;
}

static inline void kputc(char c)
{
    /* The printer page is marked U (so app.S can use it too), so S-mode
     * needs SUM set to write to it -- set it unconditionally here rather
     * than relying on every subtest leaving SUM the way it found it. */
    csr_set(sstatus, SSTATUS_SUM);
    *(volatile char *) PRINTER_BASE = c;
}

static __attribute__((noreturn)) void fail(void)
{
    kputc('F');
    msim_halt();
}

static void setup_paging(void)
{
    /* "ret", i.e. jalr zero, 0(ra): executed only after a deliberate
     * instruction-fetch fault resumes one page later, see
     * execute_from_non_executable()/supervisor_fetch_from_user_page(). */
    *(volatile unsigned int *) resume_page = 0x00008067u;
    *(volatile unsigned int *) resume_user_page = 0x00008067u;

    /* Identity-map the kernel's own code/data/bss with plain RWX -- paging
     * must never get in the way of the kernel running itself. */
    unsigned int kernel_end = (unsigned int) __kernel_end;
    for (unsigned int va = KERNEL_BASE; va < kernel_end; va += PAGE_SIZE) {
        leaf_table[leaf_index(va)] = leaf_pte(va, PTE_R | PTE_W | PTE_X);
    }
    root_table[mega_index(KERNEL_BASE)] = nonleaf_pte((unsigned int) leaf_table);

    /* Stack, identity-mapped. Printer, identity-mapped and reachable from
     * U-mode too, so the app can report its own results. */
    root_table[mega_index(STACK_BASE)] = leaf_pte(STACK_BASE, PTE_R | PTE_W);
    root_table[mega_index(PRINTER_BASE)] = leaf_pte(PRINTER_BASE, PTE_R | PTE_W | PTE_U);

    /* The pages the tests below actually care about. */
    test_leaf_table[leaf_index(SUPERVISOR_ONLY_PAGE_ADDR)] = leaf_pte((unsigned int) rwx_page, PTE_R | PTE_W | PTE_X);
    test_leaf_table[leaf_index(READ_ONLY_PAGE_ADDR)] = leaf_pte((unsigned int) ro_page, PTE_R);
    test_leaf_table[leaf_index(EXECUTE_ONLY_PAGE_ADDR)] = leaf_pte((unsigned int) xo_page, PTE_X);
    test_leaf_table[leaf_index(USER_ACCESSIBLE_PAGE_ADDR)] = leaf_pte((unsigned int) u_rwx_page, PTE_R | PTE_W | PTE_X | PTE_U);
    test_leaf_table[leaf_index(NX_PAGE_ADDR)] = leaf_pte((unsigned int) nx_page, PTE_R);
    test_leaf_table[leaf_index(RESUME_PAGE_ADDR)] = leaf_pte((unsigned int) resume_page, PTE_X);
    test_leaf_table[leaf_index(NX_USER_PAGE_ADDR)] = leaf_pte((unsigned int) nx_user_page, PTE_X | PTE_U);
    test_leaf_table[leaf_index(RESUME_USER_PAGE_ADDR)] = leaf_pte((unsigned int) resume_user_page, PTE_X);
    test_leaf_table[leaf_index(APP_ADDRESS)] = leaf_pte(APP_ADDRESS, PTE_X | PTE_U);

    /* A non-leaf entry pointing at a non-existent 3rd level (Sv32 only
     * has two), and one left "not present" -- both deliberately broken. */
    test_leaf_table[leaf_index(NON_LEAF_SECOND_LEVEL_ADDR)] = PTE_V;
    /* INVALID_SECOND_LEVEL_ADDR is intentionally never written: an
     * all-zero entry already means "not present". */

    root_table[mega_index(TEST_MEGAPAGE_BASE)] = nonleaf_pte((unsigned int) test_leaf_table);

    /* A present leaf megapage whose low PPN bits are non-zero: a
     * misaligned superpage. */
    root_table[mega_index(UNALIGNED_MEGAPAGE_ADDR)] = leaf_pte(0x00001000u, PTE_R);

    /* INVALID_FIRST_LEVEL_ADDR and address 0 are also intentionally left
     * unmapped, for the same "not present" reason as above. */
}

static void access_dirty(void)
{
    volatile unsigned int *pte = &test_leaf_table[leaf_index(SUPERVISOR_ONLY_PAGE_ADDR)];
    volatile unsigned char *page = (volatile unsigned char *) SUPERVISOR_ONLY_PAGE_ADDR;

    if ((*pte & (PTE_A | PTE_D)) != 0)
        fail();

    (void) page[0]; /* a read must set Accessed */
    if ((*pte & (PTE_A | PTE_D)) != PTE_A)
        fail();

    page[0] = 0; /* a write must set Dirty too */
    if ((*pte & (PTE_A | PTE_D)) != (PTE_A | PTE_D))
        fail();

    kputc('S');
}

static void write_read_only(void)
{
    volatile unsigned char *page = (volatile unsigned char *) READ_ONLY_PAGE_ADDR;
    page[0] = 0; /* must trap */
}

static void read_execute_only_non_mxr(void)
{
    volatile unsigned char *page = (volatile unsigned char *) EXECUTE_ONLY_PAGE_ADDR;
    (void) page[0]; /* must trap: execute-only, MXR clear */
}

static void read_execute_only_set_mxr(void)
{
    volatile unsigned char *page = (volatile unsigned char *) EXECUTE_ONLY_PAGE_ADDR;
    csr_set(sstatus, SSTATUS_MXR);
    (void) page[0]; /* must not trap: MXR makes X pages readable */
    csr_clear(sstatus, SSTATUS_MXR);
}

static void read_u_page_not_sum(void)
{
    volatile unsigned char *page = (volatile unsigned char *) USER_ACCESSIBLE_PAGE_ADDR;
    csr_clear(sstatus, SSTATUS_SUM);
    (void) page[0]; /* must trap: S-mode, U-page, SUM clear */
}

static void read_u_page_set_sum(void)
{
    volatile unsigned char *page = (volatile unsigned char *) USER_ACCESSIBLE_PAGE_ADDR;
    csr_set(sstatus, SSTATUS_SUM);
    (void) page[0]; /* must not trap */
}

static void execute_from_non_executable(void)
{
    /* Jumps to the last word of a non-executable page. That must trap on
     * fetch; the handler resumes one page later, at RESUME_PAGE_ADDR,
     * whose "ret" returns here via the same `ra` this call sets up. */
    void (*fn)(void) = (void (*)(void)) (NX_PAGE_ADDR + PAGE_SIZE - 4);
    fn();
}

static void supervisor_fetch_from_user_page(void)
{
    /* Same idea, but the non-executable page is also marked user: SUM
     * only ever relaxes data access, never instruction fetch, so this
     * must trap regardless. */
    csr_set(sstatus, SSTATUS_SUM);
    void (*fn)(void) = (void (*)(void)) (NX_USER_PAGE_ADDR + PAGE_SIZE - 4);
    fn();
    csr_clear(sstatus, SSTATUS_SUM);
}

static void read_from_unaligned_megapage(void)
{
    volatile unsigned int *p = (volatile unsigned int *) UNALIGNED_MEGAPAGE_ADDR;
    (void) *p; /* must trap: misaligned superpage */
}

static void read_from_invalid_first_level_pte(void)
{
    volatile unsigned int *p = (volatile unsigned int *) INVALID_FIRST_LEVEL_ADDR;
    (void) *p; /* must trap: root entry not present */
}

static void read_from_invalid_second_level_pte(void)
{
    volatile unsigned int *p = (volatile unsigned int *) INVALID_SECOND_LEVEL_ADDR;
    (void) *p; /* must trap: leaf entry not present */
}

static void read_from_non_leaf_second_level_pte(void)
{
    volatile unsigned int *p = (volatile unsigned int *) NON_LEAF_SECOND_LEVEL_ADDR;
    (void) *p; /* must trap: leaf entry points to a non-existent 3rd level */
}

static void read_from_non_mapped(void)
{
    volatile unsigned int *p = (volatile unsigned int *) 0;
    (void) *p; /* must trap: nothing mapped at address 0 */
}

/*
 * Runs entirely in S-mode. This has to be a real, separate, noinline
 * function rather than a `&&label` jumped to via mret: a label's address
 * can be taken, but the label itself is not an instruction-scheduling
 * barrier, since nothing ever reaches it through a real `goto` -- at
 * -O2, GCC is free to interleave kernel_main()'s other code (notably
 * setup_paging(), if inlined) around it, which is silently wrong once
 * hardware (not the compiler) is what transfers control there. A real
 * function has a well-defined entry address nothing else gets scheduled
 * into.
 */
static __attribute__((noinline)) void run_in_smode(void)
{
    access_dirty();
    kputc('\n');
    write_read_only();
    kputc('\n');
    read_execute_only_non_mxr();
    kputc('\n');
    read_execute_only_set_mxr();
    kputc('\n');
    read_u_page_not_sum();
    kputc('\n');
    read_u_page_set_sum();
    kputc('\n');
    execute_from_non_executable();
    kputc('\n');
    supervisor_fetch_from_user_page();
    kputc('\n');
    read_from_unaligned_megapage();
    kputc('\n');
    read_from_invalid_first_level_pte();
    kputc('\n');
    read_from_invalid_second_level_pte();
    kputc('\n');
    read_from_non_leaf_second_level_pte();
    kputc('\n');
    read_from_non_mapped();
    kputc('\n');

    /* Drop to U-mode to run app.bin; it halts the machine itself. */
    csr_clear(sstatus, SSTATUS_SPP);
    csr_write(sepc, APP_ADDRESS);
    __asm__ volatile("sret" ::: "memory");

    fail(); /* unreachable if app.bin behaved as expected */
}

void kernel_main(void)
{
    setup_paging();
    csr_write(satp, (1u << 31) | ((unsigned int) root_table >> 12));

    /* Drop to S-mode, entering run_in_smode(). */
    csr_write(mstatus, MSTATUS_MPP_S);
    csr_write(mtvec, (unsigned int) resuming_trap_handler);
    csr_write(mepc, (unsigned int) run_in_smode);
    __asm__ volatile("mret" ::: "memory");

    fail(); /* unreachable */
}
