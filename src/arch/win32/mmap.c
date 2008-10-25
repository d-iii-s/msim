/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../mmap.h"

#ifdef __WIN32__

#include <windows.h>
#include <errno.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	HANDLE fh = _get_osfhandle(fd);
	if (fh == INVALID_HANDLE_VALUE) {
		errno = EBADF;
		return MAP_FAILED
	}
	
	DWORD protect = 0;
	DWORD access = 0;
	if (prot & PROT_EXEC == PROT_EXEC) {
		if (prot & PROT_READ == PROT_READ) {
			if (prot & PROT_WRITE == PROT_WRITE) {
				protect = PAGE_EXECUTE_READWRITE;
				access = FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE;
			} else {
				protect = PAGE_EXECUTE_READONLY;
				access = FILE_MAP_READ | FILE_MAP_EXECUTE;
			}
		} else {
			if (prot & PROT_WRITE == PROT_WRITE)
				access = FILE_MAP_WRITE | FILE_MAP_EXECUTE;
		}
	} else {
		if (prot & PROT_READ == PROT_READ) {
			if (prot & PROT_WRITE == PROT_WRITE) {
				protect = PAGE_READWRITE;
				access = FILE_MAP_ALL_ACCESS;
			} else {
				protect = PAGE_READONLY;
				access = FILE_MAP_READ;
			}
		} else {
			if (prot & PROT_WRITE == PROT_WRITE)
				access = FILE_MAP_WRITE;
		}
	}
	
	HANDLE handle = CreateFileMapping(fh, NULL, protect, length >> 32, length & 0xffffffff, NULL);
	if (handle == NULL) {
		errno = EPERM;
		return MAP_FAILED;
	}
	
	void *map = MapViewOfFile(handle, access, offset >> 32, offset & 0xffffffff, length & 0xffffffff);
	if (map == NULL) {
		errno = ENOMEM;
		return MAP_FAILED;
	}
	
	CloseHandle(handle);
	return map;
}


int munmap(void *addr, size_t length)
{
	if (!UnmapViewOfFile(addr))
		return -1;
	
	return 0;
}

#endif /* __WIN32__ */
