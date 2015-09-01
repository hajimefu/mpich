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
#ifndef MPIDCH4_COMM_H_INCLUDED
#define MPIDCH4_COMM_H_INCLUDED

#include "ch4_impl.h"

__CH4_INLINE__ int MPIDI_Comm_AS_enabled(MPID_Comm * comm)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_reenable_anysource(MPID_Comm * comm, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_remote_group_failed(MPID_Comm * comm, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_group_failed(MPID_Comm * comm_ptr, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_failure_ack(MPID_Comm * comm_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_failure_get_acked(MPID_Comm * comm_ptr,
                                                MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_revoke(MPID_Comm * comm_ptr, int is_remote)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_get_all_failed_procs(MPID_Comm * comm_ptr, MPID_Group ** failed_group,
                                                   int tag)
{
    MPIU_Assert(0);
    return 0;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_create(MPID_Comm * comm)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_CREATE);
    mpi_errno = MPIDI_netmod_comm_create(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_CREATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_destroy(MPID_Comm * comm)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_DESTROY);
    mpi_errno = MPIDI_netmod_comm_destroy(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_DESTROY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* MPIDCH4_COMM_H_INCLUDED */