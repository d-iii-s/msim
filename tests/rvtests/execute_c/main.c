
#define ebreak __asm__("ebreak")

#define PRINTER_ADDR 0x90000000

#define bool _Bool
#define false 0
#define true 1

void memset(char *p, char val, unsigned long count){
    for(int i = 0; i < count; ++i){
        p[i] = val;
    }
}

void putchar(char c) {
    volatile char* p = (char*)PRINTER_ADDR;
    *p = c;
}

void puts(char* s){
    for(char* p = s; *p != 0; ++p){
        putchar(*p);
    }
}

bool try_int2s(unsigned int num, char* output, int len){
    
    if(len < 2) return false;
    
    if(num == 0){
        output[0] = '0';
        output[1] = 0;
        return true;
    }

    char buffer[len];
    int i;
    for(i = 0; i < len; i++){
        buffer[i] = '0' + num % 10;
        num /= 10;
        if(num == 0)
            break;
    }
    // The last written index is i
    // We need to write i chars + 1 '\0', so we need i+1 bytes total
    if(i >= len - 1)
        return false;

    for(int j = 0; j <= i; ++j){
        output[j] = buffer[i - j];
    }
    output[i + 1] = 0;
    return true;
}

#define print_int_buf_len 32

void print_int(unsigned int num){
    char buffer[print_int_buf_len] = {0};
    if(try_int2s(num, buffer, print_int_buf_len)){
        puts(buffer);
        putchar('\n');
    }
}

unsigned int factorial_rec(unsigned int n){
    if(n == 0) return 1;
    return n * factorial_rec(n - 1);
}

unsigned int fib_rec(unsigned int n){
    if(n == 0 || n == 1) return n;
    return fib_rec(n-1) + fib_rec(n-2);
}

unsigned int fib_loop(unsigned int n){
    if(n == 0 || n == 1) return n;
    unsigned int prev = 0;
    unsigned int next = 1;
    for(int i = 2; i <= n; ++i){
        unsigned int temp = next;
        next = next + prev;
        prev = temp;
    }
    return next;
}

unsigned int global_int = 5;

void main(void) {
    puts("Hello world!\n");
    print_int(42);

    puts("\nFact\n");
    for(int i = 0; i < 10; ++i){
        print_int(factorial_rec(i));
    }
    puts("\nFib rec\n");
    for(int i = 0; i < 20; ++i){
        print_int(fib_rec(i));
    }

    puts("\nFib loop\n");
    for(int i = 0; i < 20; ++i){
        print_int(fib_loop(i));
    }

    puts("\nGlobal\n");
    print_int(global_int);
    global_int += 1;
    print_int(global_int);
}