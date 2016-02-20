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
#ifndef SHM_SEND_H_INCLUDED
#define SHM_SEND_H_INCLUDED

#include "ch4_shm_impl.h"
#include "ch4_impl.h"
#include <mpibsend.h>
#include <../mpi/pt2pt/bsendutil.h>
/* ---------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_impl.h */
/* ---------------------------------------------------- */
/* assumes value!=0 means the fbox is full.  Contains acquire barrier to
 * ensure that later operations that are dependent on this check don't
 * escape earlier than this check. */
#define MPIDI_CH4_SHMI_SIMPLE_Fbox_is_full(pbox_) (OPA_load_acquire_int(&(pbox_)->flag.value))

/* ---------------------------------------------------- */
/* MPIDI_CH4_SHMI_SIMPLE_Do_isend                                             */
/* ---------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHMI_SIMPLE_Do_isend)
static inline int MPIDI_CH4_SHMI_SIMPLE_Do_isend(const void *buf,
                                                 int count,
                                                 MPI_Datatype datatype,
                                                 int rank,
                                                 int tag,
                                                 MPID_Comm *comm, int context_offset, MPID_Request **request, int type)
{
    int dt_contig, mpi_errno = MPI_SUCCESS;
    MPI_Aint dt_true_lb;
    MPID_Request *sreq = NULL;
    size_t data_sz;
    MPID_Datatype *dt_ptr;
    MPIDI_STATE_DECL(MPID_STATE_SHM_DO_ISEND);

    MPIDI_FUNC_ENTER(MPID_STATE_SHM_DO_ISEND);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_CREATE_SREQ(sreq);
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq), comm->rank, tag, comm->context_id + context_offset);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_buf = (char *) buf + dt_true_lb;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count = count;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype = datatype;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->data_sz = data_sz;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->type = type;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->dest = rank;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->next = NULL;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->pending = NULL;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_ptr = NULL;

    if(!dt_contig) {
        MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1((MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char *)buf, MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count, MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype, MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_ptr, 0);
        MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_first = 0;
        MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->segment_size = data_sz;
    }

    dtype_add_ref_if_not_builtin(datatype);
    /* enqueue sreq */
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_ENQUEUE(sreq, MPIDI_CH4_SHMI_SIMPLE_Sendq);
    *request = sreq;
    MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                    (MPL_DBG_FDEST, "Enqueued to grank %d from %d (comm_kind %d) in recv %d,%d,%d\n",
                     MPIDI_CH4R_rank_to_lpid(rank, comm), MPIDI_CH4_SHMI_SIMPLE_mem_region.rank, comm->comm_kind,
                     comm->rank, tag, comm->context_id + context_offset));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_DO_ISEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SEND)
static inline int MPIDI_CH4_SHM_send(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int dt_contig __attribute__((__unused__)), mpi_errno = MPI_SUCCESS;
    MPI_Aint dt_true_lb;
    size_t data_sz;
    MPID_Datatype *dt_ptr;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* try to send immediately, contig, short message */
    if(dt_contig && data_sz <= MPIDI_CH4_SHMI_SIMPLE_EAGER_THRESHOLD) {
        /* eager message */
        int grank = MPIDI_CH4R_rank_to_lpid(rank, comm);

        /* Try freeQ */
        if(!MPIDI_CH4_SHMI_SIMPLE_Queue_empty(MPIDI_CH4_SHMI_SIMPLE_mem_region.my_freeQ)) {
            MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t cell;
            MPIDI_CH4_SHMI_SIMPLE_Queue_dequeue(MPIDI_CH4_SHMI_SIMPLE_mem_region.my_freeQ, &cell);
            MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(cell, comm->rank, tag, comm->context_id + context_offset);
            cell->pkt.mpich.datalen = data_sz;
            cell->pkt.mpich.type = MPIDI_CH4_SHMI_SIMPLE_TYPEEAGER;
            MPIU_Memcpy((void *) cell->pkt.mpich.p.payload, (char *)buf+dt_true_lb, data_sz);
            cell->pending = NULL;
            MPIDI_CH4_SHMI_SIMPLE_Queue_enqueue(MPIDI_CH4_SHMI_SIMPLE_mem_region.RecvQ[grank], cell);
            *request = NULL;
            MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                            (MPL_DBG_FDEST, "Sent to grank %d from %d in send %d,%d,%d\n", grank, cell->my_rank, cell->rank, cell->tag,
                             cell->context_id));
            goto fn_exit;
        }
    }

    /* Long message or */
    /* Failed to send immediately - create and return request */
    mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, MPIDI_CH4_SHMI_SIMPLE_TYPESTANDARD);

fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND);
    return mpi_errno;
}

static inline int MPIDI_CH4_SHM_rsend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}



static inline int MPIDI_CH4_SHM_irsend(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SSEND)
static inline int MPIDI_CH4_SHM_ssend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISSEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISSEND);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, MPIDI_CH4_SHMI_SIMPLE_TYPESYNC);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISSEND);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_STARTALL)
static inline int MPIDI_CH4_SHM_startall(int count, MPID_Request *requests[])
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_STARTALL);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_STARTALL);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);

    for(i = 0; i < count; i++) {
        MPID_Request *preq = requests[i];

        if(preq->kind == MPID_PREQUEST_SEND) {
            if(MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->type != MPIDI_CH4_SHMI_SIMPLE_TYPEBUFFERED) {
                mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_isend(MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_buf, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_count,
                                                           MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->datatype, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->dest, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->tag,
                                                           preq->comm, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->context_id - preq->comm->context_id,
                                                           &preq->partner_request, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->type);
            } else {
                MPI_Request sreq_handle;
                mpi_errno = MPIR_Ibsend_impl(MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_buf, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_count,
                                             MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->datatype, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->dest, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->tag, preq->comm,
                                             &sreq_handle);

                if(mpi_errno == MPI_SUCCESS)
                    MPID_Request_get_ptr(sreq_handle, preq->partner_request);
            }
        } else if(preq->kind == MPID_PREQUEST_RECV) {
            mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_irecv(MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_buf, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->user_count,
                                                       MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->datatype, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->rank, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->tag,
                                                       preq->comm, MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->context_id - preq->comm->context_id,
                                                       &preq->partner_request);
        } else {
            MPIU_Assert(0);
        }

        if(mpi_errno == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if(MPIDI_CH4_SHMI_SIMPLE_REQUEST(preq)->type == MPIDI_CH4_SHMI_SIMPLE_TYPEBUFFERED) {
                preq->cc_ptr = &preq->cc;
                MPIR_cc_set(&preq->cc, 0);
            } else
                preq->cc_ptr = &preq->partner_request->cc;
        } else {
            preq->partner_request = NULL;
            preq->status.MPI_ERROR = mpi_errno;
            preq->cc_ptr = &preq->cc;
            MPIR_cc_set(&preq->cc, 0);
        }
    }

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_STARTALL);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SEND_INIT)
static inline int MPIDI_CH4_SHM_send_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_CREATE_SREQ(sreq);
    MPIU_Object_set_ref(sreq, 1);
    MPIR_cc_set(&(sreq)->cc, 0);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq), comm->rank, tag, comm->context_id + context_offset);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_buf = (char *) buf;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count = count;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->dest = rank;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype = datatype;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->type = MPIDI_CH4_SHMI_SIMPLE_TYPESTANDARD;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SSEND_INIT)
static inline int MPIDI_CH4_SHM_ssend_init(const void *buf,
                                           int count,
                                           MPI_Datatype datatype,
                                           int rank,
                                           int tag,
                                           MPID_Comm *comm,
                                           int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_CREATE_SREQ(sreq);
    MPIU_Object_set_ref(sreq, 1);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq), comm->rank, tag, comm->context_id + context_offset);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_buf = (char *) buf;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count = count;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->dest = rank;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype = datatype;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->type = MPIDI_CH4_SHMI_SIMPLE_TYPESYNC;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

static inline int MPIDI_CH4_SHM_bsend_init(const void *buf,
                                           int count,
                                           MPI_Datatype datatype,
                                           int rank,
                                           int tag,
                                           MPID_Comm *comm,
                                           int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_CREATE_SREQ(sreq);
    MPIU_Object_set_ref(sreq, 1);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq), comm->rank, tag, comm->context_id + context_offset);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_buf = (char *) buf;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count = count;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->dest = rank;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype = datatype;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->type = MPIDI_CH4_SHMI_SIMPLE_TYPEBUFFERED;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

static inline int MPIDI_CH4_SHM_rsend_init(const void *buf,
                                           int count,
                                           MPI_Datatype datatype,
                                           int rank,
                                           int tag,
                                           MPID_Comm *comm,
                                           int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_RSEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_RSEND_INIT);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST_CREATE_SREQ(sreq);
    MPIU_Object_set_ref(sreq, 1);
    MPIR_cc_set(&(sreq)->cc, 0);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    MPIDI_CH4_SHMI_SIMPLE_ENVELOPE_SET(MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq), comm->rank, tag, comm->context_id + context_offset);
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_buf = (char *) buf;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->user_count = count;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->dest = rank;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->datatype = datatype;
    MPIDI_CH4_SHMI_SIMPLE_REQUEST(sreq)->type = MPIDI_CH4_SHMI_SIMPLE_TYPEREADY;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_RSEND_INIT);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_isend)
static inline int MPIDI_CH4_SHM_isend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISEND);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, MPIDI_CH4_SHMI_SIMPLE_TYPESTANDARD);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISEND);
    return mpi_errno;
}

static inline int MPIDI_CH4_SHM_issend(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISSEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISSEND);
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    mpi_errno = MPIDI_CH4_SHMI_SIMPLE_Do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, MPIDI_CH4_SHMI_SIMPLE_TYPESYNC);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISSEND);
    return mpi_errno;
}

static inline int MPIDI_CH4_SHM_cancel_send(MPID_Request *sreq)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    MPID_Request *req = MPIDI_CH4_SHMI_SIMPLE_Sendq.head;
    MPID_Request *prev_req = NULL;
    int mpi_errno = MPI_SUCCESS;

    while(req) {

        if(req == sreq) {
            MPIR_STATUS_SET_CANCEL_BIT(sreq->status, TRUE);
            MPIR_STATUS_SET_COUNT(sreq->status, 0);
            MPIDI_CH4_SHMI_SIMPLE_REQUEST_COMPLETE(sreq);
            MPIDI_CH4_SHMI_SIMPLE_REQUEST_DEQUEUE_AND_SET_ERROR(&sreq,prev_req,MPIDI_CH4_SHMI_SIMPLE_Sendq,mpi_errno);
            break;
        }

        prev_req = req;
        req = MPIDI_CH4_SHMI_SIMPLE_REQUEST(req)->next;
    }

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX);
    return mpi_errno;
}

#endif /* SHM_SEND_H_INCLUDED */