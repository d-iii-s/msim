/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  String constants
 *
 */

#ifndef TEXT_H_
#define TEXT_H_

extern const char *const txt_file_open_err;
extern const char *const txt_file_read_err;
extern const char *const txt_file_close_err;
extern const char *const txt_filename_expected;
extern const char *const txt_file_create_err;
extern const char *const txt_file_write_err;
extern const char *const txt_file_seek_err;
extern const char *const txt_file_tell_err;
extern const char *const txt_file_map_fail;
extern const char *const txt_file_unmap_fail;
extern const char *const txt_devname_expected;
extern const char *const txt_duplicate_devname;
extern const char *const txt_devaddr_expected;
extern const char *const txt_devaddr_error;
extern const char *const txt_no_more_parms;
extern const char *const txt_not_en_mem;
extern const char *const txt_intnum_expected;
extern const char *const txt_intnum_range;
extern const char *const txt_cmd_expected;
extern const char *const txt_unknown_cmd;

extern const char *exc_text[];

extern const char txt_version[];
extern const char txt_help[];

extern const char hexchar[];

#endif /* TEXT_H_ */
