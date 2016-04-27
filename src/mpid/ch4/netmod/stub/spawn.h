/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_STUB_DYNPROC_H_INCLUDED
#define NETMOD_STUB_DYNPROC_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_comm_connect(const char *port_name,
                                            MPIR_Info * info,
                                            int root, MPIR_Comm * comm, MPIR_Comm ** newcomm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_comm_disconnect(MPIR_Comm * comm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_open_port(MPIR_Info * info_ptr, char *port_name)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_close_port(const char *port_name)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_comm_accept(const char *port_name,
                                           MPIR_Info * info,
                                           int root, MPIR_Comm * comm, MPIR_Comm ** newcomm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* NETMOD_STUB_WIN_H_INCLUDED */