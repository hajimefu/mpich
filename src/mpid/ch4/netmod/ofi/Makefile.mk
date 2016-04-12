## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2011 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##
##  Portions of this code were written by Intel Corporation.
##  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
##  to Argonne National Laboratory subject to Software Grant and Corporate
##  Contributor License Agreement dated February 8, 2012.
##
if BUILD_CH4_NETMOD_OFI

noinst_HEADERS     +=
mpi_core_sources   += src/mpid/ch4/netmod/ofi/func_table.c \
                      src/mpid/ch4/netmod/ofi/globals.c \
                      src/mpid/ch4/netmod/ofi/util.cc
errnames_txt_files += src/mpid/ch4/netmod/ofi/errnames.txt
external_ldflags   += -ldl -lpthread
external_subdirs   += @ofisrcdir@
pmpi_convenience_libs += @ofilib@
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions

endif
