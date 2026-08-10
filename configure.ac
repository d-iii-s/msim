
AC_INIT([msim], [3.2.0], [github.com/d-iii-s/msim], [https://d3s.mff.cuni.cz/software/msim/])

AC_PROG_CC
if test "$GCC" = "yes" ; then
	CFLAGS="-g -O3 -Wall -Wextra -Wno-unused-parameter -Wmissing-prototypes $CFLAGS"
fi

AC_CHECK_PROG(CP, cp, cp, no)
if test "$CP" = "no" ; then
	AC_MSG_ERROR(Utility cp not found (usually part of coreutils).)
fi

AC_CHECK_PROG(MV, mv, mv, no)
if test "$MV" = "no" ; then
	AC_MSG_ERROR(Utility mv not found (usually part of coreutils).)
fi

AC_CHECK_PROG(RM, rm, rm, no)
if test "$RM" = "no" ; then
	AC_MSG_ERROR(Utility rm not found (usually part of coreutils).)
fi

AC_CHECK_PROG(DIFF, diff, diff, no)
if test "$DIFF" = "no" ; then
	AC_MSG_ERROR(Utility diff not found (usually part of diffutils).)
fi

AC_CHECK_PROG(MAKEDEPEND, makedepend, makedepend, no)
if test "$MAKEDEPEND" = "no" ; then
	AC_MSG_ERROR(Utility makedepend not found (usually part of imake).)
fi

AC_PROG_INSTALL

AC_CHECK_LIB(readline, readline,, [AC_MSG_FAILURE(Library readline not found.)])
AC_CHECK_LIB(m, fmaf,, [AC_MSG_FAILURE(Math library not found.)])
AC_CHECK_LIB(wsock32, main)

AC_CHECK_INCLUDES_DEFAULT
AC_CHECK_HEADERS([ \
	ctype.h \
	errno.h \
	fcntl.h \
	getopt.h \
	inttypes.h \
	math.h \
	readline/history.h \
	readline/readline.h \
	signal.h \
	stdarg.h \
	stdbool.h \
	stdint.h \
	stdio.h \
	stdlib.h \
	string.h \
	sys/stat.h \
	sys/time.h \
	sys/types.h \
	time.h \
	unistd.h \
],, [AC_MSG_FAILURE(Standard header not found.)])

AC_C_BIGENDIAN
AC_C_CONST
AC_TYPE_SIZE_T

AC_CHECK_FUNCS([getopt_long],, [AC_MSG_FAILURE(Function getopt_long not defined.)])
AC_SYS_LARGEFILE

AC_SUBST(BUILDLIBS)
AC_SUBST(AFTERBUILD)
AC_SUBST(INSTALL_PREFIX)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
