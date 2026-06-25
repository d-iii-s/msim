
#define ebreak __asm__("ebreak")

#define PRINTER_ADDR 0x90000000

#define bool _Bool
#define false 0
#define true 1

void memset(char *p, char val, unsigned long count)
{
    for (int i = 0; i < count; ++i) {
        p[i] = val;
    }
}

void putchar(char c)
{
    volatile char *p = (char *) PRINTER_ADDR;
    *p = c;
}

void puts(char *s)
{
    for (char *p = s; *p != 0; ++p) {
        putchar(*p);
    }
}

bool try_int2s(unsigned int num, char *output, int len)
{

    if (len < 2) {
        return false;
    }

    if (num == 0) {
        output[0] = '0';
        output[1] = 0;
        return true;
    }

    char buffer[len];
    int i;
    for (i = 0; i < len; i++) {
        buffer[i] = '0' + num % 10;
        num /= 10;
        if (num == 0) {
            break;
        }
    }
    // The last written index is i
    // We need to write i chars + 1 '\0', so we need i+1 bytes total
    if (i >= len - 1) {
        return false;
    }

    for (int j = 0; j <= i; ++j) {
        output[j] = buffer[i - j];
    }
    output[i + 1] = 0;
    return true;
}

#define print_int_buf_len 32

void print_int(unsigned int num)
{
    char buffer[print_int_buf_len] = { 0 };
    if (try_int2s(num, buffer, print_int_buf_len)) {
        puts(buffer);
    }
}

void print_int_ln(unsigned int num)
{
    print_int(num);
    putchar('\n');
}

unsigned int factorial_rec(unsigned int n)
{
    if (n == 0) {
        return 1;
    }
    return n * factorial_rec(n - 1);
}

unsigned int fib_rec(unsigned int n)
{
    if (n == 0 || n == 1) {
        return n;
    }
    return fib_rec(n - 1) + fib_rec(n - 2);
}

unsigned int fib_loop(unsigned int n)
{
    if (n == 0 || n == 1) {
        return n;
    }
    unsigned int prev = 0;
    unsigned int next = 1;
    for (int i = 2; i <= n; ++i) {
        unsigned int temp = next;
        next = next + prev;
        prev = temp;
    }
    return next;
}

unsigned int global_int = 5;

// MAT_DIM * MAT_DIM * 4 = size of one matrix
// We store 3 matrices on the stack (which is ~16 KiB)
#define MAT_DIM 32

typedef int matrix[MAT_DIM][MAT_DIM];

void print_mat(matrix *m)
{
    for (int i = 0; i < MAT_DIM; ++i) {
        for (int j = 0; j < MAT_DIM; ++j) {
            print_int((*m)[i][j]);
            puts(" ");
        }
        puts("\n");
    }
}

void get_id_mat(matrix *m)
{

    for (int i = 0; i < MAT_DIM; ++i) {
        for (int j = 0; j < MAT_DIM; ++j) {
            (*m)[i][j] = (i == j ? 1 : 0);
        }
    }
}

void get_default_mat(matrix *m)
{
    for (int i = 0; i < MAT_DIM; ++i) {
        for (int j = 0; j < MAT_DIM; ++j) {
            (*m)[i][j] = i + j;
        }
    }
}

void mat_mul(matrix *a, matrix *b, matrix *res)
{
    for (int i = 0; i < MAT_DIM; ++i) {
        for (int j = 0; j < MAT_DIM; ++j) {
            (*res)[i][j] = 0;
            for (int k = 0; k < MAT_DIM; ++k) {
                (*res)[i][j] += (*a)[i][k] * (*b)[k][j];
            }
        }
    }
}

void demonstration(void)
{
    puts("Hello world!\n");
    print_int_ln(42);

    puts("\nFact\n");
    for (int i = 0; i < 10; ++i) {
        print_int_ln(factorial_rec(i));
    }
    puts("\nFib rec\n");
    for (int i = 0; i < 20; ++i) {
        print_int_ln(fib_rec(i));
    }

    puts("\nFib loop\n");
    for (int i = 0; i < 20; ++i) {
        print_int_ln(fib_loop(i));
    }

    puts("\nGlobal\n");
    print_int_ln(global_int);
    global_int += 1;
    print_int_ln(global_int);

    puts("\nMatrix\n");

    matrix m1, m2, m3;

    /*
        1 0   ...   0
        0 1   ...   0
        .
        .
        .
        0     ...   1

    */
    get_id_mat(&m1);

    /*
      0 1        ...    MAT_DIM - 1
      1
      .                    .
      .                    .
      .                    .

      MAT_DIM - 1 ...   2 * MAT_DIM - 2

    */
    get_default_mat(&m2);

    // m3 = m1 * m2 = m2

    mat_mul(&m1, &m2, &m3);

    print_mat(&m3);
}

void main(void)
{
    demonstration();
}
