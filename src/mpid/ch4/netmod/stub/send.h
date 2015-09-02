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
#ifndef NETMOD_STUB_SEND_H_INCLUDED
#define NETMOD_STUB_SEND_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_send(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_send(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_rsend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_rsend(buf, count, datatype, rank, tag, comm, context_offset, request);
}



static inline int MPIDI_netmod_irsend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_irsend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_ssend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_startall(int count, MPID_Request * requests[])
{
    return MPIDI_CH4U_startall(count, requests);
}

static inline int MPIDI_netmod_send_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_send_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_ssend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_ssend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_bsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_bsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_rsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_rsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_isend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_issend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_issend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_netmod_cancel_send(MPID_Request * sreq)
{
    return MPIDI_CH4U_cancel_send(sreq);
}

#endif /* NETMOD_STUB_SEND_H_INCLUDED */
