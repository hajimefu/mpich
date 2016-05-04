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
#ifndef SHM_SIMPLE_RECV_H_INCLUDED
#define SHM_SIMPLE_RECV_H_INCLUDED

#include "impl.h"
#include "ch4_impl.h"

/* ---------------------------------------------------- */
/* general queues                                       */
/* ---------------------------------------------------- */
extern MPIDI_SIMPLE_request_queue_t MPIDI_SIMPLE_recvq_posted;
extern MPIDI_SIMPLE_request_queue_t MPIDI_SIMPLE_recvq_unexpected;

/* ---------------------------------------------------- */
/* MPIDI_SIMPLE_do_irecv                                             */
/* ---------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SIMPLE_do_irecv)
static inline int MPIDI_SIMPLE_do_irecv(void *buf,
                                                 int count,
                                                 MPI_Datatype datatype,
                                                 int rank,
                                                 int tag,
                                                 MPIR_Comm *comm, int context_offset, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS, dt_contig;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPIR_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_SHM_DO_IRECV);

    MPIDI_FUNC_ENTER(MPID_STATE_SHM_DO_IRECV);

    MPIDI_SIMPLE_REQUEST_CREATE_RREQ(rreq);

    if(unlikely(rank == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(&(rreq->status));
        *request = rreq;
        goto fn_exit;
    }

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(rreq), rank, tag, comm->context_id + context_offset);
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_SIMPLE_REQUEST(rreq)->user_buf = (char *)buf + dt_true_lb;
    MPIDI_SIMPLE_REQUEST(rreq)->user_count = count;
    MPIDI_SIMPLE_REQUEST(rreq)->datatype = datatype;
    MPIDI_SIMPLE_REQUEST(rreq)->data_sz = data_sz;
    MPIDI_SIMPLE_REQUEST(rreq)->next = NULL;
    MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr = NULL;
    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq) = NULL;
    MPIR_STATUS_SET_COUNT(rreq->status, 0);

    if(!dt_contig) {
        MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1((MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char *)buf, MPIDI_SIMPLE_REQUEST(rreq)->user_count, MPIDI_SIMPLE_REQUEST(rreq)->datatype, MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr, 0);
        MPIDI_SIMPLE_REQUEST(rreq)->segment_first = 0;
        MPIDI_SIMPLE_REQUEST(rreq)->segment_size = data_sz;
    }

    dtype_add_ref_if_not_builtin(datatype);
    /* enqueue rreq */
    MPIDI_SIMPLE_REQUEST_ENQUEUE(rreq, MPIDI_SIMPLE_recvq_posted);
    MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                    (MPL_DBG_FDEST, "Enqueued from grank %d to %d (comm_kind %d) in recv %d,%d,%d\n",
                     MPIDI_CH4U_rank_to_lpid(rank, comm), MPIDI_SIMPLE_mem_region.rank, comm->comm_kind,
                     rank, tag, comm->context_id + context_offset));
    *request = rreq;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_DO_IRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_recv)
static inline int MPIDI_CH4_SHM_recv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPIR_Comm *comm,
                                     int context_offset, MPI_Status *status, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS, dt_contig __attribute__((__unused__));
    size_t data_sz __attribute__((__unused__));
    MPIDI_STATE_DECL(MPIDI_SHM_RECV);

    MPIDI_FUNC_ENTER(MPIDI_SHM_RECV);

    /* create a request */
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    mpi_errno = MPIDI_SIMPLE_do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPIDI_SHM_RECV);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_recv)
static inline int MPIDI_CH4_SHM_recv_init(void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPIR_Comm *comm, int context_offset, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_RECV_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_RECV_INIT);

    MPIDI_SIMPLE_REQUEST_CREATE_RREQ(rreq);
    MPIU_Object_set_ref(rreq, 1);
    MPIR_cc_set(&rreq->cc, 0);
    rreq->kind = MPIR_REQUEST_KIND__PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(rreq), rank, tag, comm->context_id + context_offset);
    MPIDI_SIMPLE_REQUEST(rreq)->user_buf = (char *)buf;
    MPIDI_SIMPLE_REQUEST(rreq)->user_count = count;
    MPIDI_SIMPLE_REQUEST(rreq)->datatype = datatype;
    *request = rreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_RECV_INIT);
    return mpi_errno;
}


static inline int MPIDI_CH4_SHM_imrecv(void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       MPIR_Request *message, MPIR_Request **rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    int dt_contig;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPIR_Request *rreq = NULL, *sreq = NULL;
    int rank, tag, context_id;

    MPIDI_STATE_DECL(MPIDI_SHM_IMRECV);
    MPIDI_FUNC_ENTER(MPIDI_SHM_IMRECV);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);

    if(message == NULL) {
        MPIDI_Request_create_null_rreq(rreq, mpi_errno, goto fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

    MPIU_Assert(message != NULL);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_SIMPLE_REQUEST_CREATE_RREQ(rreq);
    MPIU_Object_set_ref(rreq, 1);
    MPIR_cc_set(&rreq->cc, 0);
    MPIDI_SIMPLE_ENVELOPE_GET(MPIDI_SIMPLE_REQUEST(message), rank, tag, context_id);
    MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(rreq), rank, tag, context_id);
    rreq->comm = message->comm;
    MPIR_Comm_add_ref(message->comm);
    MPIDI_SIMPLE_REQUEST(rreq)->user_buf = (char *)buf + dt_true_lb;
    MPIDI_SIMPLE_REQUEST(rreq)->user_count = count;
    MPIDI_SIMPLE_REQUEST(rreq)->datatype = datatype;
    MPIDI_SIMPLE_REQUEST(rreq)->next = NULL;
    MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr = NULL;
    MPIR_STATUS_SET_COUNT(rreq->status, 0);

    if(!dt_contig) {
        MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1((MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char *)buf, MPIDI_SIMPLE_REQUEST(rreq)->user_count, MPIDI_SIMPLE_REQUEST(rreq)->datatype, MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr, 0);
        MPIDI_SIMPLE_REQUEST(rreq)->segment_first = 0;
        MPIDI_SIMPLE_REQUEST(rreq)->segment_size = data_sz;
    }

    if(MPIDI_SIMPLE_REQUEST(message)->pending) {
        /* Sync send - we must send ACK */
        int srank = message->status.MPI_SOURCE;
        MPIR_Request *req_ack = NULL;
        MPIDI_SIMPLE_REQUEST_CREATE_SREQ(req_ack);
        MPIU_Object_set_ref(req_ack, 1);
        req_ack->comm = message->comm;
        MPIR_Comm_add_ref(message->comm);
        MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(req_ack), message->comm->rank, tag, context_id);
        MPIDI_SIMPLE_REQUEST(req_ack)->user_buf = NULL;
        MPIDI_SIMPLE_REQUEST(req_ack)->user_count = 0;
        MPIDI_SIMPLE_REQUEST(req_ack)->datatype = MPI_BYTE;
        MPIDI_SIMPLE_REQUEST(req_ack)->data_sz = 0;
        MPIDI_SIMPLE_REQUEST(req_ack)->type = MPIDI_SIMPLE_TYPEACK;
        MPIDI_SIMPLE_REQUEST(req_ack)->dest = srank;
        MPIDI_SIMPLE_REQUEST(req_ack)->next = NULL;
        MPIDI_SIMPLE_REQUEST(req_ack)->segment_ptr = NULL;
        MPIDI_SIMPLE_REQUEST(req_ack)->pending = MPIDI_SIMPLE_REQUEST(message)->pending;
        /* enqueue req_ack */
        MPIDI_SIMPLE_REQUEST_ENQUEUE(req_ack, MPIDI_SIMPLE_sendq);
    }

    for(sreq = message; sreq;) {
        MPIR_Request *next_req = NULL;
        char *send_buffer = MPIDI_SIMPLE_REQUEST(sreq)->user_buf;
        char *recv_buffer = (char *) MPIDI_SIMPLE_REQUEST(rreq)->user_buf;

        if(MPIDI_SIMPLE_REQUEST(sreq)->type == MPIDI_SIMPLE_TYPEEAGER) {
            /* eager message */
            data_sz = MPIDI_SIMPLE_REQUEST(sreq)->data_sz;

            if(MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr) {
                /* non-contig */
                size_t last = MPIDI_SIMPLE_REQUEST(rreq)->segment_first + data_sz;
                MPID_Segment_unpack(MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr, MPIDI_SIMPLE_REQUEST(rreq)->segment_first, (MPI_Aint *)&last, send_buffer);
                MPID_Segment_free(MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr);
            } else

                /* contig */
                if(send_buffer)
                    MPIU_Memcpy(recv_buffer, (void *) send_buffer, data_sz);

            /* set status */
            rreq->status.MPI_SOURCE = sreq->status.MPI_SOURCE;
            rreq->status.MPI_TAG = sreq->status.MPI_TAG;
            count = MPIR_STATUS_GET_COUNT(rreq->status) + (MPI_Count)data_sz;
            MPIR_STATUS_SET_COUNT(rreq->status,count);
        } else if(MPIDI_SIMPLE_REQUEST(sreq)->type == MPIDI_SIMPLE_TYPELMT) {
            /* long message */
            if(MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr) {
                /* non-contig */
                size_t last = MPIDI_SIMPLE_REQUEST(rreq)->segment_first + MPIDI_SIMPLE_EAGER_THRESHOLD;
                MPID_Segment_unpack(MPIDI_SIMPLE_REQUEST(rreq)->segment_ptr, MPIDI_SIMPLE_REQUEST(rreq)->segment_first, (MPI_Aint *)&last, send_buffer);
                MPIDI_SIMPLE_REQUEST(rreq)->segment_first = last;
            } else

                /* contig */
                if(send_buffer)
                    MPIU_Memcpy(recv_buffer, (void *) send_buffer, MPIDI_SIMPLE_EAGER_THRESHOLD);

            MPIDI_SIMPLE_REQUEST(rreq)->data_sz -= MPIDI_SIMPLE_EAGER_THRESHOLD;
            MPIDI_SIMPLE_REQUEST(rreq)->user_buf += MPIDI_SIMPLE_EAGER_THRESHOLD;
            count = MPIR_STATUS_GET_COUNT(rreq->status) + (MPI_Count)MPIDI_SIMPLE_EAGER_THRESHOLD;
            MPIR_STATUS_SET_COUNT(rreq->status,count);
        }

        /* destroy unexpected req */
        MPIDI_SIMPLE_REQUEST(sreq)->pending = NULL;
        MPL_free(MPIDI_SIMPLE_REQUEST(sreq)->user_buf);
        next_req = MPIDI_SIMPLE_REQUEST(sreq)->next;
        MPIDI_SIMPLE_REQUEST_COMPLETE(sreq);
        sreq = next_req;
    }

    *rreqp = rreq;

fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPIDI_SHM_IMRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_irecv)
static inline int MPIDI_CH4_SHM_irecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPIR_Comm *comm, int context_offset, MPIR_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPIDI_SHM_IRECV);

    MPIDI_FUNC_ENTER(MPIDI_SHM_IRECV);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    mpi_errno = MPIDI_SIMPLE_do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPIDI_SHM_IRECV);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_cancel_recv)
static inline int MPIDI_CH4_SHM_cancel_recv(MPIR_Request *rreq)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    MPIR_Request *req = MPIDI_SIMPLE_recvq_posted.head;
    MPIR_Request *prev_req = NULL;

    while(req) {

        if(req == rreq) {
            /* Remove request from shm posted receive queue */

            if(prev_req) {
                MPIDI_SIMPLE_REQUEST(prev_req)->next = MPIDI_SIMPLE_REQUEST(req)->next;
            } else {
                MPIDI_SIMPLE_recvq_posted.head = MPIDI_SIMPLE_REQUEST(req)->next;
            }

            if(req == MPIDI_SIMPLE_recvq_posted.tail) {
                MPIDI_SIMPLE_recvq_posted.tail = prev_req;
            }

            MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
            MPIR_STATUS_SET_COUNT(req->status, 0);
            MPIDI_SIMPLE_REQUEST_COMPLETE(req);

            break;
        }

        prev_req = req;
        req = MPIDI_SIMPLE_REQUEST(req)->next;
    }

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
    return MPI_SUCCESS;
}

#endif /* SHM_SIMPLE_RECV_H_INCLUDED */