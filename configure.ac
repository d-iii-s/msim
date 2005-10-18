AC_INIT
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE(msim, 1.3)
AC_PREFIX_DEFAULT(/usr/local)

AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_INSTALL

AC_CHECK_PROGS(LNK, ld wlink, linker)
AC_CHECK_LIB(readline, readline,,,[-lcurses])
AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(curses, main)

AC_HEADER_STDC([])
AC_CHECK_HEADERS([errno.h wait.h signal.h syslog.h time.h sys/stat.h\
	sys/ioctl.h sys/time.h sys/types.h sys/termios.h fcntl.h getopt.h\
	stdint.h stdbool.h\
	readline/readline.h readline/history.h])

AC_C_BIGENDIAN()

AC_CHECK_TYPES([socklen_t])

AC_CHECK_FUNCS([getopt_long])

AM_PROG_CC_STDC([])

AM_CONFIG_HEADER(config.h)

AC_CONFIG_FILES([Makefile src/Makefile examples/Makefile])

AC_OUTPUT
