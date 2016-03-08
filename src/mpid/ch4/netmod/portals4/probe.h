/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_PORTALS4_PROBE_H_INCLUDED
#define NETMOD_PORTALS4_PROBE_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_probe(int source,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    return MPIDI_CH4R_probe(source, tag, comm, context_offset, status);
}

static inline int MPIDI_CH4_NM_improbe(int source,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset,
                                       int *flag, MPID_Request ** message, MPI_Status * status)
{
    return MPIDI_CH4R_improbe(source, tag, comm, context_offset, flag, message, status);
}

static inline int MPIDI_CH4_NM_iprobe(int source,
                                      int tag,
                                      MPID_Comm * comm,
                                      int context_offset, int *flag, MPI_Status * status)
{
    return MPIDI_CH4R_iprobe(source, tag, comm, context_offset, flag, status);
}

#endif /* NETMOD_PORTALS4_PROBE_H_INCLUDED */
