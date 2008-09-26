/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#define PACKAGE "msim"
#define VERSION "1.3.5"

#define MAX_CPU 31

/**< Input and output buffer size for communication with gdb */
#define GDB_BUFFER_SIZE 512
#define SETUP_BUF_SIZE  65536

#define ERR_OK     0  /**< OK */
#define ERR_IO     1  /**< I/O error occures */
#define ERR_MEM    2  /**< Not enough memory */
#define ERR_INIT   3  /**< Initial script fails */
#define ERR_PARM   4  /**< Invalid parameter */
#define ERR_INTERN 5  /**< Internal error */

#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

#endif /* MAIN_H_ */
