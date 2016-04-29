/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef RMA_H_INCLUDED
#define RMA_H_INCLUDED

#include "impl.h"

static inline int MPIDI_UCX_contig_put(const void *origin_addr,
                                   size_t size,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   MPIR_Win * win)
{

    MPIDI_UCX_win_info_t* win_info = &(MPIDI_UCX_WIN_INFO(win, target_rank));
    size_t offset;
    uint64_t base;
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t status;
    MPIR_Comm *comm = win->comm_ptr;
    ucp_ep_h ep = MPIDI_UCX_COMM_TO_EP(comm, target_rank);

    MPIDI_CH4U_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    base = win_info->addr;
    offset = target_disp * win_info->disp;

    status = ucp_put_nbi(ep, origin_addr, size, base+offset, win_info->rkey);
 /*   if(status == UCS_INPROGRESS)
        MPIDI_UCX_WIN(win).need_local_flush = 1;
    else*/
      MPIDI_UCX_CHK_STATUS(status, ucp_mem_map);

 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;



}

static inline int MPIDI_UCX_contig_get(void *origin_addr,
                                   size_t size,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   MPIR_Win * win)
{

    MPIDI_UCX_win_info_t* win_info = &(MPIDI_UCX_WIN_INFO(win, target_rank));
    size_t offset;
    uint64_t base;
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t status;
    MPIR_Comm *comm = win->comm_ptr;
    ucp_ep_h ep = MPIDI_UCX_COMM_TO_EP(comm, target_rank);


    MPIDI_CH4U_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    base = win_info->addr;
    offset = target_disp * win_info->disp;

    status = ucp_get_nbi(ep, origin_addr, size, base+offset, win_info->rkey);
/*    if(status == UCS_INPROGRESS)
        MPIDI_UCX_WIN(win).need_local_flush = 1;
    else*/
     MPIDI_UCX_CHK_STATUS(status, ucp_mem_map);

 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;



}
static inline int MPIDI_CH4_NM_put(const void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count, MPI_Datatype target_datatype, MPIR_Win * win)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_PUT);
    int            target_contig,origin_contig,mpi_errno = MPI_SUCCESS;
    size_t target_bytes,origin_bytes;
    MPI_Aint       origin_true_lb,target_true_lb;
    size_t         offset;

    MPIDI_Datatype_check_contig_size_lb(target_datatype,target_count,
                                        target_contig,target_bytes,
                                        target_true_lb);
    MPIDI_Datatype_check_contig_size_lb(origin_datatype,origin_count,
                                        origin_contig,origin_bytes,
                                        origin_true_lb);

    MPIR_ERR_CHKANDJUMP((origin_bytes != target_bytes),mpi_errno,MPI_ERR_SIZE,"**rmasize");

    if(unlikely((origin_bytes == 0) ||(target_rank == MPI_PROC_NULL)))
        goto fn_exit;

#ifdef MPIDI_UCX_SHM
    if(!target_contig || !origin_contig || MPIDI_UCX_WIN_INFO(win, target_rank).rkey == NULL)
#else
    if(!target_contig ||  !origin_contig)
#endif
       return  MPIDI_CH4U_put(origin_addr, origin_count, origin_datatype,
                          target_rank, target_disp, target_count, target_datatype, win);

    MPIDI_CH4U_EPOCH_CHECK_SYNC(win, mpi_errno, goto fn_fail);

    if (target_rank == win->comm_ptr->rank) {
        offset = win->disp_unit * target_disp;
        return MPIR_Localcopy(origin_addr,
                              origin_count,
                              origin_datatype,
                              (char *)win->base + offset,
                              target_count,
                              target_datatype);
    }




       mpi_errno = MPIDI_UCX_contig_put(origin_addr, origin_bytes,
                                      target_rank, target_disp, win);
 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_get(void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count, MPI_Datatype target_datatype, MPIR_Win * win)
{


    int            origin_contig,target_contig, mpi_errno = MPI_SUCCESS;
    size_t origin_bytes;
    size_t         offset;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_GET);

    MPIDI_Datatype_check_contig_size(origin_datatype,origin_count,
                                     origin_contig,origin_bytes);

    if(unlikely((origin_bytes == 0) || (target_rank == MPI_PROC_NULL)))
        goto fn_exit;

    MPIDI_Datatype_check_contig(target_datatype,target_contig);


#ifdef MPIDI_UCX_SHM
    if(!origin_contig || !target_contig ||  MPIDI_UCX_WIN_INFO(win, target_rank).rkey == NULL )
#else
    if(!target_contig || !origin_contig)
#endif
     return MPIDI_CH4U_get(origin_addr, origin_count, origin_datatype,
                          target_rank, target_disp, target_count, target_datatype, win);

    MPIDI_CH4U_EPOCH_CHECK_SYNC(win, mpi_errno, goto fn_fail);

    if(target_rank == win->comm_ptr->rank) {
        offset    = target_disp * win->disp_unit;
        return  MPIR_Localcopy((char *)win->base + offset,
                                   target_count,
                                   target_datatype,
                                   origin_addr,
                                   origin_count,
                                   origin_datatype);
    }


    return  MPIDI_UCX_contig_get(origin_addr, origin_bytes,
                                      target_rank, target_disp, win);
 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_rput(const void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPIR_Win * win, MPIR_Request ** request)
{
    return MPIDI_CH4U_rput(origin_addr, origin_count, origin_datatype,
                           target_rank, target_disp, target_count, target_datatype, win, request);
}


static inline int MPIDI_CH4_NM_compare_and_swap(const void *origin_addr,
                                                const void *compare_addr,
                                                void *result_addr,
                                                MPI_Datatype datatype,
                                                int target_rank,
                                                MPI_Aint target_disp, MPIR_Win * win)
{
    return MPIDI_CH4U_compare_and_swap(origin_addr, compare_addr, result_addr,
                                       datatype, target_rank, target_disp, win);
}

static inline int MPIDI_CH4_NM_raccumulate(const void *origin_addr,
                                           int origin_count,
                                           MPI_Datatype origin_datatype,
                                           int target_rank,
                                           MPI_Aint target_disp,
                                           int target_count,
                                           MPI_Datatype target_datatype,
                                           MPI_Op op, MPIR_Win * win, MPIR_Request ** request)
{
    return MPIDI_CH4U_raccumulate(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, op, win, request);
}

static inline int MPIDI_CH4_NM_rget_accumulate(const void *origin_addr,
                                               int origin_count,
                                               MPI_Datatype origin_datatype,
                                               void *result_addr,
                                               int result_count,
                                               MPI_Datatype result_datatype,
                                               int target_rank,
                                               MPI_Aint target_disp,
                                               int target_count,
                                               MPI_Datatype target_datatype,
                                               MPI_Op op, MPIR_Win * win, MPIR_Request ** request)
{
    return MPIDI_CH4U_rget_accumulate(origin_addr, origin_count, origin_datatype,
                                      result_addr, result_count, result_datatype,
                                      target_rank, target_disp, target_count,
                                      target_datatype, op, win, request);
}

static inline int MPIDI_CH4_NM_fetch_and_op(const void *origin_addr,
                                            void *result_addr,
                                            MPI_Datatype datatype,
                                            int target_rank,
                                            MPI_Aint target_disp, MPI_Op op, MPIR_Win * win)
{
    return MPIDI_CH4U_fetch_and_op(origin_addr, result_addr, datatype,
                                   target_rank, target_disp, op, win);
}


static inline int MPIDI_CH4_NM_rget(void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPIR_Win * win, MPIR_Request ** request)
{
    return MPIDI_CH4U_rget(origin_addr, origin_count, origin_datatype,
                           target_rank, target_disp, target_count, target_datatype, win, request);
}


static inline int MPIDI_CH4_NM_get_accumulate(const void *origin_addr,
                                              int origin_count,
                                              MPI_Datatype origin_datatype,
                                              void *result_addr,
                                              int result_count,
                                              MPI_Datatype result_datatype,
                                              int target_rank,
                                              MPI_Aint target_disp,
                                              int target_count,
                                              MPI_Datatype target_datatype,
                                              MPI_Op op, MPIR_Win * win)
{
    return MPIDI_CH4U_get_accumulate(origin_addr, origin_count, origin_datatype,
                                     result_addr, result_count, result_datatype,
                                     target_rank, target_disp, target_count,
                                     target_datatype, op, win);
}

static inline int MPIDI_CH4_NM_accumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype, MPI_Op op, MPIR_Win * win)
{
    return MPIDI_CH4U_accumulate(origin_addr, origin_count, origin_datatype,
                                 target_rank, target_disp, target_count, target_datatype, op, win);
}

#endif /* RMA_H_INCLUDED */
