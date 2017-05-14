/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef CH4I_WORKQ_H_INCLUDED
#define CH4I_WORKQ_H_INCLUDED

#include "ch4i_workq_types.h"

/* For profiling */
extern double MPIDI_pt2pt_enqueue_time;
extern double MPIDI_pt2pt_progress_time;
extern double MPIDI_pt2pt_issue_pend_time;
extern double MPIDI_rma_enqueue_time;
extern double MPIDI_rma_progress_time;
extern double MPIDI_rma_issue_pend_time;

#if defined(MPIDI_WORKQ_PROFILE)
#define MPIDI_WORKQ_PT2PT_ENQUEUE_START  double enqueue_t1 = MPI_Wtime();
#define MPIDI_WORKQ_PT2PT_ENQUEUE_STOP                          \
    do {                                                        \
        double enqueue_t2 = MPI_Wtime();                        \
        MPIDI_pt2pt_enqueue_time += (enqueue_t2 - enqueue_t1);  \
    }while(0)
#define MPIDI_WORKQ_PROGRESS_START double progress_t1 = MPI_Wtime();
#define MPIDI_WORKQ_PROGRESS_STOP                                 \
    do {                                                                \
        double progress_t2 = MPI_Wtime();                               \
        MPIDI_pt2pt_progress_time += (progress_t2 - progress_t1);       \
    }while(0)
#define MPIDI_WORKQ_ISSUE_START    double issue_t1 = MPI_Wtime();
#define MPIDI_WORKQ_ISSUE_STOP                            \
    do {                                                        \
        double issue_t2 = MPI_Wtime();                          \
        MPIDI_pt2pt_issue_pend_time += (issue_t2 - issue_t1);   \
    }while(0)

#define MPIDI_WORKQ_RMA_ENQUEUE_START    double enqueue_t1 = MPI_Wtime();
#define MPIDI_WORKQ_RMA_ENQUEUE_STOP                            \
    do {                                                        \
        double enqueue_t2 = MPI_Wtime();                        \
        MPIDI_rma_enqueue_time += (enqueue_t2 - enqueue_t1);    \
    }while(0)
#else /*!defined(MPIDI_WORKQ_PROFILE)*/
#define MPIDI_WORKQ_PT2PT_ENQUEUE_START
#define MPIDI_WORKQ_PT2PT_ENQUEUE_STOP
#define MPIDI_WORKQ_PROGRESS_START
#define MPIDI_WORKQ_PROGRESS_STOP
#define MPIDI_WORKQ_ISSUE_START
#define MPIDI_WORKQ_ISSUE_STOP

#define MPIDI_WORKQ_RMA_ENQUEUE_START
#define MPIDI_WORKQ_RMA_ENQUEUE_STOP
#endif

static inline void MPIDI_workq_pt2pt_enqueue_body(MPIDI_workq_op_t op,
                                                  const void *send_buf,
                                                  void *recv_buf,
                                                  MPI_Aint count,
                                                  MPI_Datatype datatype,
                                                  int rank,
                                                  int tag,
                                                  MPIR_Comm *comm_ptr,
                                                  int context_offset,
                                                  MPIDI_av_entry_t *addr,
                                                  int vni_idx,
                                                  MPI_Status *status,
                                                  MPIR_Request *request)
{
    MPIDI_workq_elemt_t* pt2pt_elemt = NULL;
    pt2pt_elemt = MPL_malloc(sizeof (*pt2pt_elemt));
    pt2pt_elemt->op       = op;
    pt2pt_elemt->send_buf = send_buf;
    pt2pt_elemt->recv_buf = recv_buf;
    pt2pt_elemt->count    = count;
    pt2pt_elemt->datatype = datatype;
    pt2pt_elemt->rank     = rank;
    pt2pt_elemt->tag      = tag;
    pt2pt_elemt->comm_ptr = comm_ptr;
    pt2pt_elemt->context_offset = context_offset;
    pt2pt_elemt->pt2pt_addr = addr;
    pt2pt_elemt->status   = status;
    pt2pt_elemt->request  = request;
    MPIDI_workq_enqueue(&comm_ptr->dev.work_queues[vni_idx].pend_ops, pt2pt_elemt);
}

static inline void MPIDI_workq_rma_enqueue_body(MPIDI_workq_op_t op,
                                                const void *origin_addr,
                                                int origin_count,
                                                MPI_Datatype origin_datatype,
                                                int target_rank,
                                                MPI_Aint target_disp,
                                                int target_count,
                                                MPI_Datatype target_datatype,
                                                MPIR_Win *win_ptr,
                                                MPIDI_av_entry_t *addr,
                                                int vni_idx)
{
    MPIDI_workq_elemt_t* rma_elemt = NULL;
    rma_elemt = MPL_malloc(sizeof (*rma_elemt));
    rma_elemt->op               = op;
    rma_elemt->origin_addr      = origin_addr;
    rma_elemt->origin_count     = origin_count;
    rma_elemt->origin_datatype  = origin_datatype;
    rma_elemt->target_rank      = target_rank;
    rma_elemt->target_disp      = target_disp;
    rma_elemt->target_count     = target_count;
    rma_elemt->target_datatype  = target_datatype;
    rma_elemt->win_ptr          = win_ptr;
    rma_elemt->rma_addr         = addr;
    MPIDI_workq_enqueue(&win_ptr->dev.work_queues[vni_idx].pend_ops, rma_elemt);
}

static inline int MPIDI_workq_vni_progress_body(int vni_idx)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_workq_elemt_t* workq_elemt = NULL;
    MPIDI_workq_list_t *cur_workq;
    MPIDI_WORKQ_PROGRESS_START;
    MPL_DL_FOREACH(MPIDI_CH4_Global.vni_queues[vni_idx], cur_workq) {
        MPIDI_workq_dequeue(&cur_workq->pend_ops, (void**)&workq_elemt);
        while(workq_elemt != NULL) {
            MPIDI_WORKQ_ISSUE_START;
            switch(workq_elemt->op) {
            case SEND:
                mpi_errno = MPIDI_NM_mpi_send(workq_elemt->send_buf,
                                              workq_elemt->count,
                                              workq_elemt->datatype,
                                              workq_elemt->rank,
                                              workq_elemt->tag,
                                              workq_elemt->comm_ptr,
                                              workq_elemt->context_offset,
                                              workq_elemt->pt2pt_addr,
                                              &workq_elemt->request);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                break;
            case ISEND:
                mpi_errno = MPIDI_NM_mpi_isend(workq_elemt->send_buf,
                                               workq_elemt->count,
                                               workq_elemt->datatype,
                                               workq_elemt->rank,
                                               workq_elemt->tag,
                                               workq_elemt->comm_ptr,
                                               workq_elemt->context_offset,
                                               workq_elemt->pt2pt_addr,
                                               &workq_elemt->request);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                break;
            case RECV:
                mpi_errno = MPIDI_NM_mpi_recv(workq_elemt->recv_buf,
                                               workq_elemt->count,
                                               workq_elemt->datatype,
                                               workq_elemt->rank,
                                               workq_elemt->tag,
                                               workq_elemt->comm_ptr,
                                               workq_elemt->context_offset,
                                               workq_elemt->pt2pt_addr,
                                               workq_elemt->status,
                                               &workq_elemt->request);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                break;
            case IRECV:
                mpi_errno = MPIDI_NM_mpi_irecv(workq_elemt->recv_buf,
                                               workq_elemt->count,
                                               workq_elemt->datatype,
                                               workq_elemt->rank,
                                               workq_elemt->tag,
                                               workq_elemt->comm_ptr,
                                               workq_elemt->context_offset,
                                               workq_elemt->pt2pt_addr,
                                               &workq_elemt->request);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                break;
            case PUT:
                mpi_errno = MPIDI_NM_mpi_put(workq_elemt->origin_addr,
                                             workq_elemt->origin_count,
                                             workq_elemt->origin_datatype,
                                             workq_elemt->target_rank,
                                             workq_elemt->target_disp,
                                             workq_elemt->target_count,
                                             workq_elemt->target_datatype,
                                             workq_elemt->win_ptr,
                                             workq_elemt->rma_addr);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                break;
            }
            MPIDI_WORKQ_ISSUE_STOP;
            MPL_free(workq_elemt);
            MPIDI_workq_dequeue(&cur_workq->pend_ops, (void**)&workq_elemt);
        }
    }
    MPIDI_WORKQ_PROGRESS_STOP;
fn_fail:
    return mpi_errno;
}

static inline void MPIDI_workq_pt2pt_enqueue(MPIDI_workq_op_t op,
                                             const void *send_buf,
                                             void *recv_buf,
                                             MPI_Aint count,
                                             MPI_Datatype datatype,
                                             int rank,
                                             int tag,
                                             MPIR_Comm *comm_ptr,
                                             int context_offset,
                                             MPIDI_av_entry_t *addr,
                                             int vni_idx,
                                             MPI_Status *status,
                                             MPIR_Request *request)
{
    MPIDI_WORKQ_PT2PT_ENQUEUE_START;
    MPIDI_workq_pt2pt_enqueue_body(op, send_buf, recv_buf, count, datatype,
                                   rank, tag, comm_ptr, context_offset, addr, vni_idx, status, request);
    MPIDI_WORKQ_PT2PT_ENQUEUE_STOP;
}

static inline void MPIDI_workq_rma_enqueue(MPIDI_workq_op_t op,
                                           const void *origin_addr,
                                           int origin_count,
                                           MPI_Datatype origin_datatype,
                                           int target_rank,
                                           MPI_Aint target_disp,
                                           int target_count,
                                           MPI_Datatype target_datatype,
                                           MPIR_Win *win_ptr,
                                           MPIDI_av_entry_t *addr,
                                           int vni_idx)
{
    MPIDI_WORKQ_RMA_ENQUEUE_START;
    MPIDI_workq_rma_enqueue_body(op, origin_addr, origin_count, origin_datatype,
                                 target_rank, target_disp, target_count, target_datatype,
                                 win_ptr, addr, vni_idx);
    MPIDI_WORKQ_RMA_ENQUEUE_STOP;
}

static inline int MPIDI_workq_vni_progress(int vni_idx)
{
    int mpi_errno;
    mpi_errno = MPIDI_workq_vni_progress_body(vni_idx);
    return mpi_errno;
}

static inline int MPIDI_workq_global_progress(int* made_progress)
{
    int mpi_errno, vni_idx;
    *made_progress = 1;
    for (vni_idx = 0; vni_idx < MPIDI_CH4_Global.n_netmod_vnis; vni_idx++) {
        MPID_THREAD_CS_ENTER(VNI, MPIDI_CH4_Global.vni_locks[vni_idx]);
        mpi_errno = MPIDI_workq_vni_progress(vni_idx);
        if(unlikely(mpi_errno != MPI_SUCCESS))
            break;
        MPID_THREAD_CS_EXIT(VNI, MPIDI_CH4_Global.vni_locks[vni_idx]);
    }
    return mpi_errno;
}

#define MPIDI_DISPATCH_PT2PT_RECV(func, send_buf, recv_buf, count, datatype, rank, tag, comm, context_offset, addr, status, request, err) \
    err = func(recv_buf, count, datatype, rank, tag, comm, context_offset, addr, status, request);
#define MPIDI_DISPATCH_PT2PT_IRECV(func, send_buf, recv_buf, count, datatype, rank, tag, comm, context_offset, addr, status, request, err) \
    err = func(recv_buf, count, datatype, rank, tag, comm, context_offset, addr, request);
#define MPIDI_DISPATCH_PT2PT_SEND(func, send_buf, recv_buf, count, datatype, rank, tag, comm, context_offset, addr, status, request, err) \
    err = func(send_buf, count, datatype, rank, tag, comm, context_offset, addr, request);
#define MPIDI_DISPATCH_PT2PT_ISEND(func, send_buf, recv_buf, count, datatype, rank, tag, comm, context_offset, addr, status, request, err) \
    err = func(send_buf, count, datatype, rank, tag, comm, context_offset, addr, request);

#endif /* CH4I_WORKQ_H_INCLUDED */
