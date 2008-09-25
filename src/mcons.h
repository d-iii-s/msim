/*
 * Copyright (c) 2000-2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Some constants
 *
 */

#ifndef MCONS_H_
#define MCONS_H_


#define MAXPROC        31
#define SETUP_BUF_SIZE 65536

#define ERR_OK     0  /**< OK */
#define ERR_IO     1  /**< I/O error occures */
#define ERR_MEM    2  /**< Not enough memory */
#define ERR_INIT   3  /**< Initial script fails */
#define ERR_PARM   4  /**< Invalid parameter */
#define ERR_INTERN 5  /**< Internal error */

/**< Input and output buffer size for communication with gdb */
#define GDB_BUFFER_SIZE 512

#endif /* MCONS_H_ */
