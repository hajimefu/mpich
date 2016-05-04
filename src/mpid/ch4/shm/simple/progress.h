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
#ifndef SHM_SIMPLE_PROGRESS_H_INCLUDED
#define SHM_SIMPLE_PROGRESS_H_INCLUDED

#include "impl.h"

/* ----------------------------------------------------- */
/* MPIDI_SIMPLE_progress_recv                     */
/* ----------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SIMPLE_progress_recv)
static inline int MPIDI_SIMPLE_progress_recv(int blocking, int *completion_count)
{
    int mpi_errno = MPI_SUCCESS;
    size_t data_sz;
    int in_cell = 0;
    MPIDI_SIMPLE_cell_ptr_t cell = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    /* try to match with unexpected */
    MPIR_Request *sreq = MPIDI_SIMPLE_recvq_unexpected.head;
    MPIR_Request *prev_sreq = NULL;
unexpected_l:

    if(sreq != NULL) {
        goto match_l;
    }

    /* try to receive from recvq */
    if(MPIDI_SIMPLE_mem_region.my_recvQ && !MPIDI_SIMPLE_queue_empty(MPIDI_SIMPLE_mem_region.my_recvQ)) {
        MPIDI_SIMPLE_queue_dequeue(MPIDI_SIMPLE_mem_region.my_recvQ, &cell);
        in_cell = 1;
        goto match_l;
    }

    goto fn_exit;
match_l: {
        /* traverse posted receive queue */
        MPIR_Request *req = MPIDI_SIMPLE_recvq_posted.head;
        MPIR_Request *prev_req = NULL;
        int continue_matching = 1;
        char *send_buffer = in_cell ? (char *) cell->pkt.mpich.p.payload : (char *) MPIDI_SIMPLE_REQUEST(sreq)->user_buf;
        int type = in_cell ? cell->pkt.mpich.type : MPIDI_SIMPLE_REQUEST(sreq)->type;
        MPIR_Request *pending = in_cell ? cell->pending : MPIDI_SIMPLE_REQUEST(sreq)->pending;

        if(type == MPIDI_SIMPLE_TYPEACK) {
            /* ACK message doesn't have a matching receive! */
            int c;
            MPIU_Assert(in_cell);
            MPIU_Assert(pending);
            MPIR_cc_decr(pending->cc_ptr, &c);
            MPIDI_CH4U_request_release(pending);
            goto release_cell_l;
        }

        while(req) {
            int sender_rank, tag, context_id;
            MPI_Count count;
            MPIDI_SIMPLE_ENVELOPE_GET(MPIDI_SIMPLE_REQUEST(req), sender_rank, tag, context_id);
            MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                            (MPL_DBG_FDEST, "Posted from grank %d to %d in progress %d,%d,%d\n",
                             MPIDI_CH4U_rank_to_lpid(sender_rank, req->comm), MPIDI_SIMPLE_mem_region.rank,
                             sender_rank, tag, context_id));

            if((in_cell && MPIDI_SIMPLE_ENVELOPE_MATCH(cell, sender_rank, tag, context_id)) ||
               (sreq && MPIDI_SIMPLE_ENVELOPE_MATCH(MPIDI_SIMPLE_REQUEST(sreq), sender_rank, tag, context_id))) {

                /* Request matched */

                continue_matching = 1;

                if(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)) {
                    MPIDI_CH4R_anysource_matched(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req),
                                                 MPIDI_CH4R_SHM, &continue_matching);
                    MPIDI_CH4U_request_release(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req));

                    /* Decouple requests */
                    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)) = NULL;
                    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req) = NULL;

                    if(continue_matching) break;
                }

                char *recv_buffer = (char *) MPIDI_SIMPLE_REQUEST(req)->user_buf;

                if(pending) {
                    /* we must send ACK */
                    int srank = in_cell ? cell->rank : sreq->status.MPI_SOURCE;
                    MPIR_Request *req_ack = NULL;
                    MPIDI_SIMPLE_REQUEST_CREATE_SREQ(req_ack);
                    MPIU_Object_set_ref(req_ack, 1);
                    req_ack->comm = req->comm;
                    MPIR_Comm_add_ref(req->comm);

                    MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(req_ack), req->comm->rank, tag, context_id);
                    MPIDI_SIMPLE_REQUEST(req_ack)->user_buf = NULL;
                    MPIDI_SIMPLE_REQUEST(req_ack)->user_count = 0;
                    MPIDI_SIMPLE_REQUEST(req_ack)->datatype = MPI_BYTE;
                    MPIDI_SIMPLE_REQUEST(req_ack)->data_sz = 0;
                    MPIDI_SIMPLE_REQUEST(req_ack)->type = MPIDI_SIMPLE_TYPEACK;
                    MPIDI_SIMPLE_REQUEST(req_ack)->dest = srank;
                    MPIDI_SIMPLE_REQUEST(req_ack)->next = NULL;
                    MPIDI_SIMPLE_REQUEST(req_ack)->segment_ptr = NULL;
                    MPIDI_SIMPLE_REQUEST(req_ack)->pending = pending;
                    /* enqueue req_ack */
                    MPIDI_SIMPLE_REQUEST_ENQUEUE(req_ack, MPIDI_SIMPLE_sendq);
                }

                if(type == MPIDI_SIMPLE_TYPEEAGER)
                    /* eager message */
                    data_sz = in_cell ? cell->pkt.mpich.datalen : MPIDI_SIMPLE_REQUEST(sreq)->data_sz;
                else if(type == MPIDI_SIMPLE_TYPELMT)
                    data_sz = MPIDI_SIMPLE_EAGER_THRESHOLD;
                else
                    MPIU_Assert(0);

                /* check for user buffer overflow */
                size_t user_data_sz = MPIDI_SIMPLE_REQUEST(req)->data_sz;
                if(user_data_sz < data_sz) {
                    req->status.MPI_ERROR = MPI_ERR_TRUNCATE;
                    data_sz = user_data_sz;
                }

                /* copy to user buffer */
                if(MPIDI_SIMPLE_REQUEST(req)->segment_ptr) {
                    /* non-contig */
                    size_t last = MPIDI_SIMPLE_REQUEST(req)->segment_first + data_sz;
                    MPID_Segment_unpack(MPIDI_SIMPLE_REQUEST(req)->segment_ptr, MPIDI_SIMPLE_REQUEST(req)->segment_first, (MPI_Aint *)&last, send_buffer);
                    if (last != MPIDI_SIMPLE_REQUEST(req)->segment_first + data_sz )
                        req->status.MPI_ERROR = MPI_ERR_TYPE;
                    if(type == MPIDI_SIMPLE_TYPEEAGER)
                        MPID_Segment_free(MPIDI_SIMPLE_REQUEST(req)->segment_ptr);
                    else
                        MPIDI_SIMPLE_REQUEST(req)->segment_first = last;
                } else
                    /* contig */
                    if(send_buffer) MPIU_Memcpy(recv_buffer, (void *) send_buffer, data_sz);
                MPIDI_SIMPLE_REQUEST(req)->data_sz -= data_sz;
                MPIDI_SIMPLE_REQUEST(req)->user_buf += data_sz;

                /* set status and dequeue receive request if done */
                count = MPIR_STATUS_GET_COUNT(req->status) + (MPI_Count)data_sz;
                MPIR_STATUS_SET_COUNT(req->status,count);
                if(type == MPIDI_SIMPLE_TYPEEAGER) {
                    if(in_cell) {
                        req->status.MPI_SOURCE = cell->rank;
                        req->status.MPI_TAG = cell->tag;
                    } else {
                        req->status.MPI_SOURCE = sreq->status.MPI_SOURCE;
                        req->status.MPI_TAG = sreq->status.MPI_TAG;
                    }
                    MPIDI_SIMPLE_REQUEST_DEQUEUE_AND_SET_ERROR(&req, prev_req, MPIDI_SIMPLE_recvq_posted,
                                                                        req->status.MPI_ERROR);
                }

                goto release_cell_l;
            }              /* if matched  */

            prev_req = req;
            req = MPIDI_SIMPLE_REQUEST(req)->next;
        }

        /* unexpected message, no posted matching req */
        if(in_cell) {
            /* free the cell, move to unexpected queue */
            MPIR_Request *rreq;
            MPIDI_SIMPLE_REQUEST_CREATE_RREQ(rreq);
            MPIU_Object_set_ref(rreq, 1);
            /* set status */
            rreq->status.MPI_SOURCE = cell->rank;
            rreq->status.MPI_TAG = cell->tag;
            MPIR_STATUS_SET_COUNT(rreq->status, cell->pkt.mpich.datalen);
            MPIDI_SIMPLE_ENVELOPE_SET(MPIDI_SIMPLE_REQUEST(rreq), cell->rank, cell->tag, cell->context_id);
            data_sz = cell->pkt.mpich.datalen;
            MPIDI_SIMPLE_REQUEST(rreq)->data_sz = data_sz;
            MPIDI_SIMPLE_REQUEST(rreq)->type = cell->pkt.mpich.type;

            if(data_sz > 0) {
                MPIDI_SIMPLE_REQUEST(rreq)->user_buf = (char *)MPL_malloc(data_sz);
                MPIU_Memcpy(MPIDI_SIMPLE_REQUEST(rreq)->user_buf, (void *) cell->pkt.mpich.p.payload, data_sz);
            } else {
                MPIDI_SIMPLE_REQUEST(rreq)->user_buf = NULL;
            }

            MPIDI_SIMPLE_REQUEST(rreq)->datatype = MPI_BYTE;
            MPIDI_SIMPLE_REQUEST(rreq)->next = NULL;
            MPIDI_SIMPLE_REQUEST(rreq)->pending = cell->pending;
            /* enqueue rreq */
            MPIDI_SIMPLE_REQUEST_ENQUEUE(rreq, MPIDI_SIMPLE_recvq_unexpected);
            MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                            (MPL_DBG_FDEST, "Unexpected from grank %d to %d in progress %d,%d,%d\n",
                             cell->my_rank, MPIDI_SIMPLE_mem_region.rank,
                             cell->rank, cell->tag, cell->context_id));
        } else {
            /* examine another message in unexpected queue */
            prev_sreq = sreq;
            sreq = MPIDI_SIMPLE_REQUEST(sreq)->next;
            goto unexpected_l;
        }
    }
release_cell_l:

    if(in_cell) {
        /* release cell */
        MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                        (MPL_DBG_FDEST, "Received from grank %d to %d in progress %d,%d,%d\n", cell->my_rank,
                         MPIDI_SIMPLE_mem_region.rank, cell->rank, cell->tag, cell->context_id));
        cell->pending = NULL;
        {
            MPIDI_SIMPLE_queue_enqueue(MPIDI_SIMPLE_mem_region.FreeQ[cell->my_rank], cell);
        }
    } else {
        /* destroy unexpected req */
        MPIDI_SIMPLE_REQUEST(sreq)->pending = NULL;
        MPL_free(MPIDI_SIMPLE_REQUEST(sreq)->user_buf);
        MPIDI_SIMPLE_REQUEST_DEQUEUE_AND_SET_ERROR(&sreq, prev_sreq, MPIDI_SIMPLE_recvq_unexpected, mpi_errno);
    }

    (*completion_count)++;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    return mpi_errno;
}

/* ----------------------------------------------------- */
/* MPIDI_SIMPLE_progress_send                     */
/* ----------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SIMPLE_progress_send)
static inline int MPIDI_SIMPLE_progress_send(int blocking, int *completion_count)
{
    int mpi_errno = MPI_SUCCESS;
    int dest;
    MPIDI_SIMPLE_cell_ptr_t cell = NULL;
    MPIR_Request *sreq = MPIDI_SIMPLE_sendq.head;
    MPIR_Request *prev_sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);

    if(sreq == NULL)
        goto fn_exit;

    /* try to send via freeq */
    if(!MPIDI_SIMPLE_queue_empty(MPIDI_SIMPLE_mem_region.my_freeQ)) {
        MPIDI_SIMPLE_queue_dequeue(MPIDI_SIMPLE_mem_region.my_freeQ, &cell);
        MPIDI_SIMPLE_ENVELOPE_GET(MPIDI_SIMPLE_REQUEST(sreq), cell->rank, cell->tag, cell->context_id);
        dest = MPIDI_SIMPLE_REQUEST(sreq)->dest;
        char *recv_buffer = (char *) cell->pkt.mpich.p.payload;
        size_t data_sz = MPIDI_SIMPLE_REQUEST(sreq)->data_sz;
        /*
         * TODO: make request field dest_lpid (or even recvQ[dest_lpid]) instead of dest - no need to do rank_to_lpid each time
         */
        int grank = MPIDI_CH4U_rank_to_lpid(dest, sreq->comm);
        cell->pending = NULL;

        if(MPIDI_SIMPLE_REQUEST(sreq)->type == MPIDI_SIMPLE_TYPESYNC) {
            /* increase req cc in order to release req only after ACK, do it once per SYNC request */
            /* non-NULL pending req signal receiver about sending ACK back */
            /* the pending req should be sent back for sender to decrease cc, for it is dequeued already */
            int c;
            cell->pending = sreq;
            MPIR_cc_incr(sreq->cc_ptr, &c);
            MPIDI_SIMPLE_REQUEST(sreq)->type = MPIDI_SIMPLE_TYPESTANDARD;
        }

        if(data_sz <= MPIDI_SIMPLE_EAGER_THRESHOLD) {
            cell->pkt.mpich.datalen = data_sz;

            if(MPIDI_SIMPLE_REQUEST(sreq)->type == MPIDI_SIMPLE_TYPEACK) {
                cell->pkt.mpich.type = MPIDI_SIMPLE_TYPEACK;
                cell->pending = MPIDI_SIMPLE_REQUEST(sreq)->pending;
            } else {
                /* eager message */
                if(MPIDI_SIMPLE_REQUEST(sreq)->segment_ptr) {
                    /* non-contig */
                    MPID_Segment_pack(MPIDI_SIMPLE_REQUEST(sreq)->segment_ptr, MPIDI_SIMPLE_REQUEST(sreq)->segment_first, (MPI_Aint *)&MPIDI_SIMPLE_REQUEST(sreq)->segment_size, recv_buffer);
                    MPID_Segment_free(MPIDI_SIMPLE_REQUEST(sreq)->segment_ptr);
                } else {
                    /* contig */
                    MPIU_Memcpy((void *) recv_buffer, MPIDI_SIMPLE_REQUEST(sreq)->user_buf, data_sz);
                }

                cell->pkt.mpich.type = MPIDI_SIMPLE_TYPEEAGER;
                /* set status */
                /*
                 * TODO: incorrect count for LMT - set to a last chunk of data
                 * is send status required?
                 */
                sreq->status.MPI_SOURCE = cell->rank;
                sreq->status.MPI_TAG = cell->tag;
                MPIR_STATUS_SET_COUNT(sreq->status, data_sz);
            }

            /* dequeue sreq */
            MPIDI_SIMPLE_REQUEST_DEQUEUE_AND_SET_ERROR(&sreq, prev_sreq, MPIDI_SIMPLE_sendq, mpi_errno);
        } else {
            /* long message */
            if(MPIDI_SIMPLE_REQUEST(sreq)->segment_ptr) {
                /* non-contig */
                size_t last = MPIDI_SIMPLE_REQUEST(sreq)->segment_first + MPIDI_SIMPLE_EAGER_THRESHOLD;
                MPID_Segment_pack(MPIDI_SIMPLE_REQUEST(sreq)->segment_ptr, MPIDI_SIMPLE_REQUEST(sreq)->segment_first, (MPI_Aint *)&last, recv_buffer);
                MPIDI_SIMPLE_REQUEST(sreq)->segment_first = last;
            } else {
                /* contig */
                MPIU_Memcpy((void *) recv_buffer, MPIDI_SIMPLE_REQUEST(sreq)->user_buf, MPIDI_SIMPLE_EAGER_THRESHOLD);
                MPIDI_SIMPLE_REQUEST(sreq)->user_buf += MPIDI_SIMPLE_EAGER_THRESHOLD;
            }

            cell->pkt.mpich.datalen = MPIDI_SIMPLE_EAGER_THRESHOLD;
            MPIDI_SIMPLE_REQUEST(sreq)->data_sz -= MPIDI_SIMPLE_EAGER_THRESHOLD;
            cell->pkt.mpich.type = MPIDI_SIMPLE_TYPELMT;
        }

        MPL_DBG_MSG_FMT(MPIR_DBG_HANDLE, TYPICAL,
                        (MPL_DBG_FDEST, "Sent to grank %d from %d in progress %d,%d,%d\n", grank, cell->my_rank, cell->rank, cell->tag,
                         cell->context_id));
        MPIDI_SIMPLE_queue_enqueue(MPIDI_SIMPLE_mem_region.RecvQ[grank], cell);
        (*completion_count)++;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_progress)
static inline int MPIDI_CH4_SHM_progress(int blocking)
{
    int complete = 0;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_PROGRESS);

    do {
        /* Receieve progress */
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
        MPIDI_SIMPLE_progress_recv(blocking, &complete);
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
        /* Send progress */
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_SIMPLE_SHM_MUTEX);
        MPIDI_SIMPLE_progress_send(blocking, &complete);
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_SIMPLE_SHM_MUTEX);

        MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
        MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

        if(complete > 0)
            break;
    } while(blocking);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_PROGRESS);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_test(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_poke(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_CH4_SHM_progress_start(MPID_Progress_state *state)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_CH4_SHM_progress_end(MPID_Progress_state *state)
{
    MPIU_Assert(0);
    return;
}

static inline int MPIDI_CH4_SHM_progress_wait(MPID_Progress_state *state)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_register(int (*progress_fn)(int *))
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_deregister(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_activate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_deactivate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_SIMPLE_PROGRESS_H_INCLUDED */