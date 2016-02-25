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
#ifndef NETMOD_UCX_PROBE_H_INCLUDED
#define NETMOD_UCX_PROBE_H_INCLUDED

#include "impl.h"
#include "mpidch4.h"

static inline int ucx_do_iprobe(int source,
                                 int tag,
                                  MPID_Comm * comm, int context_offset, int *flag,  MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t ucp_tag, tag_mask;
    int count;
    ucp_tag_recv_info_t info;
    ucp_tag_message_h  message_handler;
    tag_mask = MPIDI_CH4_NMI_UCX_tag_mask(tag, source);
    ucp_tag = MPIDI_CH4_NMI_UCX_recv_tag(tag, source, comm->recvcontext_id + context_offset);

    message_handler = ucp_tag_probe_nb(MPIDI_CH4_NMI_UCX_Global.worker, ucp_tag,
                                        tag_mask, 0, &info);
    if(message_handler == NULL) {
        *flag = 0;
        goto fn_exit;
    }
    *flag = 1;
     status->MPI_ERROR = MPI_SUCCESS;
     status->MPI_SOURCE = MPIDI_CH4_NMI_UCX_get_source(info.sender_tag);
     status->MPI_TAG = MPIDI_CH4_NMI_UCX_get_tag(info.sender_tag);
     count = info.length;
     MPIR_STATUS_SET_COUNT(*status, count);

fn_exit:
    return mpi_errno;

}

static inline int MPIDI_netmod_probe(int source,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;
    int flag = 0;
    while (!flag) {
        mpi_errno = ucx_do_iprobe(source, tag, comm, context_offset, &flag, status);
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
         mpi_errno = MPIDI_Progress_test();
	    if (mpi_errno!=MPI_SUCCESS)
            MPIR_ERR_POP(mpi_errno);

    }

fn_exit:
    return mpi_errno;
fn_fail:
   goto fn_exit;
}

static inline int MPIDI_CH4_NM_improbe(int source,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset,
                                       int *flag, MPID_Request ** message, MPI_Status * status)
{


    int mpi_errno = MPI_SUCCESS;
    uint64_t ucp_tag, tag_mask;
    int count;
    ucp_tag_recv_info_t info;
    ucp_tag_message_h  message_handler;
    MPID_Request *req;

    tag_mask = MPIDI_CH4_NMI_UCX_tag_mask(tag, source);
    ucp_tag = MPIDI_CH4_NMI_UCX_recv_tag(tag, source, comm->recvcontext_id + context_offset);

    message_handler = ucp_tag_probe_nb(MPIDI_CH4_NMI_UCX_Global.worker, ucp_tag,
                                        tag_mask, 1, &info);
    if(message_handler == NULL) {
        *flag = 0;
        goto fn_exit;
    }
    *flag = 1;
     req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
     MPIU_Assert(req);
     MPIDI_CH4_NMI_UCX_REQ(req).message_handler = message_handler;


     status->MPI_SOURCE = MPIDI_CH4_NMI_UCX_get_source(info.sender_tag);
     status->MPI_TAG = MPIDI_CH4_NMI_UCX_get_tag(info.sender_tag);
     count = info.length;
     MPIR_STATUS_SET_COUNT(*status, count);
     req->kind = MPID_REQUEST_MPROBE;
fn_exit:
    *message = req;
    return mpi_errno;
fn_fail:
   goto fn_exit;
}


static inline int MPIDI_CH4_NM_iprobe(int source,
                                      int tag,
                                      MPID_Comm * comm,
                                      int context_offset, int *flag, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = ucx_do_iprobe(source, tag, comm, context_offset, flag,  status);
    return mpi_errno;
}

#endif /* NETMOD_AM_OFI_PROBE_H_INCLUDED */
