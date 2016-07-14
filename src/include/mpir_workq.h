/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef MPIR_WORKQ_H_INCLUDED
#define MPIR_WORKQ_H_INCLUDED

#if defined(MPIQ_QUEUE_MODEL)

/* First define the queue implementation type */
#if defined(MPIQ_USE_MSQUEUE)
#include <queue/zm_msqueue.h>
#define MPIQ_queue_t       zm_msqueue_t
#define MPIQ_queue_init    zm_msqueue_init
#define MPIQ_queue_enqueue zm_msqueue_enqueue
#define MPIQ_queue_dequeue zm_msqueue_dequeue
#else
#include <queue/zm_glqueue.h>
#define MPIQ_queue_t       zm_glqueue_t
#define MPIQ_queue_init    zm_glqueue_init
#define MPIQ_queue_enqueue zm_glqueue_enqueue
#define MPIQ_queue_dequeue zm_glqueue_dequeue
#endif

/* Point-to-Point */
typedef enum MPIQ_pt2pt_op MPIQ_pt2pt_op_t;
typedef struct MPIQ_pt2pt_elemt MPIQ_pt2pt_elemt_t;

enum MPIQ_pt2pt_op {MPIQ_ISEND, MPIQ_IRECV};

struct MPIQ_pt2pt_elemt {
    MPIQ_pt2pt_op_t op;
    const void *send_buf;
    void *recv_buf;
    MPI_Aint count;
    MPI_Datatype datatype;
    int rank;
    int tag;
    MPIR_Comm *comm_ptr;
    MPI_Request *request;
};

/* RMA */
typedef enum MPIQ_rma_op MPIQ_rma_op_t;
typedef struct MPIQ_rma_elemt MPIQ_rma_elemt_t;

enum MPIQ_rma_op {MPIQ_PUT};

struct MPIQ_rma_elemt {
    MPIQ_rma_op_t op;
    const void *origin_addr;
    int origin_count;
    MPI_Datatype origin_datatype;
    int target_rank;
    MPI_Aint target_disp;
    int target_count;
    MPI_Datatype target_datatype;
    MPIR_Win *win_ptr;
};

/* Global queues for saving pending operations to issue */
extern MPIQ_queue_t MPIQ_pt2pt_pend_ops;
extern MPIQ_queue_t MPIQ_rma_pend_ops;

static inline void MPIQ_workq_pt2pt_enqueue_body(MPIQ_pt2pt_op_t op,
                                            const void *send_buf,
                                            void *recv_buf,
                                            MPI_Aint count,
                                            MPI_Datatype datatype,
                                            int rank,
                                            int tag,
                                            MPIR_Comm *comm_ptr,
                                            MPI_Request *request) {
    *request = MPIQ_REQUEST_IDLE;
    MPIQ_pt2pt_elemt_t* pt2pt_elemt = NULL;
    pt2pt_elemt = MPID_Alloc_mem(sizeof *pt2pt_elemt, NULL);
    pt2pt_elemt->op       = op;
    pt2pt_elemt->send_buf = send_buf;
    pt2pt_elemt->recv_buf = recv_buf;
    pt2pt_elemt->count    = count;
    pt2pt_elemt->datatype = datatype;
    pt2pt_elemt->rank     = rank;
    pt2pt_elemt->tag      = tag;
    pt2pt_elemt->comm_ptr = comm_ptr;
    pt2pt_elemt->request  = request;
    MPIQ_queue_enqueue(&MPIQ_pt2pt_pend_ops, pt2pt_elemt);
}

static inline void MPIQ_workq_rma_enqueue_body(MPIQ_rma_op_t op,
                                          const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype,
                                          MPIR_Win *win_ptr) {
    MPIQ_rma_elemt_t* rma_elemt = NULL;
    rma_elemt = MPID_Alloc_mem(sizeof *rma_elemt, NULL);
    rma_elemt->op               = op;
    rma_elemt->origin_addr      = origin_addr;
    rma_elemt->origin_count     = origin_count;
    rma_elemt->origin_datatype  = origin_datatype;
    rma_elemt->target_rank      = target_rank;
    rma_elemt->target_disp      = target_disp;
    rma_elemt->target_count     = target_count;
    rma_elemt->target_datatype  = target_datatype;
    rma_elemt->win_ptr          = win_ptr;
    MPIQ_queue_enqueue(&MPIQ_rma_pend_ops, rma_elemt);
}

static inline int MPIQ_workq_pt2pt_progress() {
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *request_ptr = NULL;
    MPIQ_pt2pt_elemt_t* pt2pt_elemt = NULL;
    MPIQ_queue_dequeue(&MPIQ_pt2pt_pend_ops, (void**)&pt2pt_elemt);
    while(pt2pt_elemt != NULL) {
        switch(pt2pt_elemt->op) {
            case MPIQ_ISEND:
                mpi_errno = MPID_Isend( pt2pt_elemt->send_buf,
                                        pt2pt_elemt->count,
                                        pt2pt_elemt->datatype,
                                        pt2pt_elemt->rank,
                                        pt2pt_elemt->tag,
                                        pt2pt_elemt->comm_ptr,
                                        MPIR_CONTEXT_INTRA_PT2PT,
                                        &request_ptr);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                MPII_SENDQ_REMEMBER(request_ptr,pt2pt_elemt->rank,pt2pt_elemt->tag,comm_ptr->context_id);
                *pt2pt_elemt->request = request_ptr->handle;
            case MPIQ_IRECV:
                mpi_errno = MPID_Irecv( pt2pt_elemt->recv_buf,
                                        pt2pt_elemt->count,
                                        pt2pt_elemt->datatype,
                                        pt2pt_elemt->rank,
                                        pt2pt_elemt->tag,
                                        pt2pt_elemt->comm_ptr,
                                        MPIR_CONTEXT_INTRA_PT2PT,
                                        &request_ptr);
                *pt2pt_elemt->request = request_ptr->handle;
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
        }
        MPID_Free_mem(pt2pt_elemt);
        MPIQ_queue_dequeue(&MPIQ_pt2pt_pend_ops, (void**)&pt2pt_elemt);
    }
  fn_fail:
    return mpi_errno;
}

static inline int MPIQ_workq_rma_progress() {
    int mpi_errno = MPI_SUCCESS;
    MPIQ_rma_elemt_t* rma_elemt = NULL;
    MPIQ_queue_dequeue(&MPIQ_rma_pend_ops, (void**)&rma_elemt);
    while(rma_elemt != NULL) {
        switch(rma_elemt->op) {
            case MPIQ_PUT:
                mpi_errno = MPID_Put(rma_elemt->origin_addr,
                                     rma_elemt->origin_count,
                                     rma_elemt->origin_datatype,
                                     rma_elemt->target_rank,
                                     rma_elemt->target_disp,
                                     rma_elemt->target_count,
                                     rma_elemt->target_datatype,
                                     rma_elemt->win_ptr);
                if (mpi_errno != MPI_SUCCESS) goto fn_fail;
        }
        MPID_Free_mem(rma_elemt);
        MPIQ_queue_dequeue(&MPIQ_rma_pend_ops, (void**)&rma_elemt);
    }
  fn_fail:
    return mpi_errno;
}

static inline int MPIQ_workq_global_progress_body() {
    int mpi_errno = MPI_SUCCESS;
    mpi_errno =  MPIQ_workq_pt2pt_progress();
    if (mpi_errno != MPI_SUCCESS) goto fn_fail;
    mpi_errno =  MPIQ_workq_rma_progress();
  fn_fail:
    return mpi_errno;
}

static inline void MPIQ_workq_pt2pt_enqueue(MPIQ_pt2pt_op_t op,
                                            const void *send_buf,
                                            void *recv_buf,
                                            MPI_Aint count,
                                            MPI_Datatype datatype,
                                            int rank,
                                            int tag,
                                            MPIR_Comm *comm_ptr,
                                            MPI_Request *request) {
    MPIQ_workq_pt2pt_enqueue_body(op, send_buf, recv_buf, count, datatype,
                                  rank, tag, comm_ptr, request);

}

static inline void MPIQ_workq_rma_enqueue(MPIQ_rma_op_t op,
                                          const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype,
                                          MPIR_Win *win_ptr) {
    MPIQ_workq_rma_enqueue_body(op, origin_addr, origin_count, origin_datatype, target_rank,
                                target_disp, target_count, target_datatype, win_ptr);
}

static inline int MPIQ_workq_global_progress() {
    int mpi_errno;
    mpi_errno = MPIQ_workq_global_progress_body();
    return mpi_errno;
}

#endif /* MPIQ_QUEUE_MODEL */

#endif /* MPIR_WORKQ_H_INCLUDED */
