
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
        
    }
}

void print_int_ln(unsigned int num){
    print_int(num);
    putchar('\n');
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

// MAT_DIM * MAT_DIM * 4 = size of one matrix
// We store 3 matrices on the stack (which is ~16 KiB)
#define MAT_DIM 32

typedef int matrix[MAT_DIM][MAT_DIM];

void print_mat(matrix *m){
    for(int i = 0; i < MAT_DIM; ++i){
        for(int j = 0; j < MAT_DIM; ++j){
            print_int((*m)[i][j]);
            puts(" ");
        }
        puts("\n");
    }
}

void get_id_mat(matrix *m){
    
    for(int i = 0; i < MAT_DIM; ++i){
        for(int j = 0; j < MAT_DIM; ++j){
            (*m)[i][j] = (i == j ? 1 : 0);
        }
    }
}

void get_default_mat(matrix *m){
    for(int i = 0; i < MAT_DIM; ++i){
        for(int j = 0; j < MAT_DIM; ++j){
            (*m)[i][j] = i + j;
        }
    }
}

void mat_mul(matrix *a, matrix *b, matrix *res){
    for(int i = 0; i < MAT_DIM; ++i){
        for(int j = 0; j < MAT_DIM; ++j){
            (*res)[i][j] = 0;
            for(int k = 0; k < MAT_DIM; ++k){
                (*res)[i][j] += (*a)[i][k] * (*b)[k][j];
            }
        }
    }
}


void demonstration(void){
    puts("Hello world!\n");
    print_int_ln(42);

    puts("\nFact\n");
    for(int i = 0; i < 10; ++i){
        print_int_ln(factorial_rec(i));
    }
    puts("\nFib rec\n");
    for(int i = 0; i < 20; ++i){
        print_int_ln(fib_rec(i));
    }

    puts("\nFib loop\n");
    for(int i = 0; i < 20; ++i){
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

void stress_test(void){

    // Times for both fib and matmul measured on AMD Ryzen 5 3600 (4.15 GHz at time of measurement) WIN 11 WSL 2

    // Run recursive fib, because it is exponential
   
    // Measured times for different n of fib_rec(n)
    //      n          t (s)    t (min)               cycles     cycles/s
    //     30            8.2                      61 732 461    7 528 348
    //     31           13.1                      99 884 970    7 624 806
    //     32           21.9                     161 616 926    7 379 768
    //     33           35.3                     261 501 329    7 407 969
    //     34           54.7                     423 117 688    7 735 241
    //     35           88.1       1:28          684 618 450    7 770 924
    //     36          142.9       2:23        1 107 735 633    7 751 823
    //     37          239.9       4:00        1 792 353 516    7 471 252
    //     38          389.9       6:30        2 900 088 520    7 438 031
    //     39          619.7      10:20        4 692 441 407    7 572 117

    // The precompiled version is configured to run the fib calculation for around 10 minutes (n = 39)

    print_int_ln(fib_rec(39));

    // Run 32x32 matrix multiplication

    // Measured times for different number of repetetion of the multiplication
    //        iters         t (s)  t (min)           cycles    cycles/s
    //           10           1.8                13 341 256    7 411 808
    //          300          54.1               399 205 396    7 379 027
    //         3500         617.6    10:17    4 657 016 597    7 540 506

    matrix m1, m2, m3;
    int iters = 3500;

    get_default_mat(&m1);
    get_default_mat(&m2);

    for (int i = 0; i < iters; ++i){
        mat_mul(&m1, &m2, &m3);
    }

    // 20:28

    // Measured times for both (n = 39, iters = 3500)
    // t (s)    t (min)          cycles    machine    measurement
    //  1316      21:56   9 349 457 980        WSL           wsl1
    //  1328      22:08   9 349 457 980        WSL           wsl1
    //  1329      22:09   9 349 457 980        WSL           wsl1
    //  1318      21:58   9 349 457 980        WSL           wsl1
    //  1327      22:07   9 349 457 980        WSL           wsl1
    //  1314      21:54   9 349 457 980        WSL           wsl1


    // Machines:
    // WSL - AMD Ryzen 5 3600 WIN 11 WSL 2 (4.05 GHz)

    // Measurements:
    // wsl1 - 6 tests ran at once
}

void main(void) {
    //demonstration();
    stress_test();
    
}