# arg/Makefile.am
CLEANFILES = *~ */*~
pkginclude_HEADERS = arg.hh val.hh

lib_LTLIBRARIES = libarg.la
libarg_la_SOURCES = arg.cc val.cc

LDADD = libarg.la

noinst_PROGRAMS = arg_ex0
arg_ex0_SOURCES = arg_ex0.cc

noinst_PROGRAMS += arg_ex1
arg_ex1_SOURCES = arg_ex1.cc

noinst_PROGRAMS += arg_ex2
arg_ex2_SOURCES = arg_ex2.cc
