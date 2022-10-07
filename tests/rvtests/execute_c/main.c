
#define ehalt __asm__(".word 0x00200073")
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

unsigned int global_int = 5;

void main(void) {
    puts("Hello world!\n");
    print_int(42);
    print_int(factorial_rec(12));

    print_int(global_int);
    global_int += 1;
    print_int(global_int);
}