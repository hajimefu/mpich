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
#ifndef NETMOD_AM_OFI_SEND_H_INCLUDED
#define NETMOD_AM_OFI_SEND_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_send(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_send(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_rsend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_rsend(buf, count, datatype, rank, tag, comm, context_offset, request);
}



static inline int MPIDI_CH4_NM_irsend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_irsend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_ssend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_startall(int count, MPID_Request *requests[])
{
    return MPIDI_CH4R_startall(count, requests);
}

static inline int MPIDI_CH4_NM_send_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm *comm,
                                         int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_send_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_ssend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm *comm,
                                          int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_ssend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_bsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm *comm,
                                          int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_bsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_rsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm *comm,
                                          int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_rsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_isend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_issend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    return MPIDI_CH4R_issend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_cancel_send(MPID_Request *sreq)
{
    return MPIDI_CH4R_cancel_send(sreq);
}

#endif /* NETMOD_AM_OFI_SEND_H_INCLUDED */