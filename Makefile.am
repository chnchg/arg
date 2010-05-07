# arg/Makefile.am
ACLOCAL_AMFLAGS = -I m4
CLEANFILES = *~ */*~
argincludedir = $(includedir)/arg-1.0
arginclude_HEADERS = arg.hh val.hh

lib_LTLIBRARIES = libarg.la
libarg_la_SOURCES = arg.cc val.cc
libarg_la_LDFLAGS = -version-info 1:0:0

LDADD = libarg.la

noinst_PROGRAMS = arg_ex0
arg_ex0_SOURCES = arg_ex0.cc

noinst_PROGRAMS += arg_ex1
arg_ex1_SOURCES = arg_ex1.cc

noinst_PROGRAMS += arg_ex2
arg_ex2_SOURCES = arg_ex2.cc
