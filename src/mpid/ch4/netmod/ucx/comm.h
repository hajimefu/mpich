/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_create(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_COMM_CREATE);

    MPIDI_CH4U_init_comm(comm);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_COMM_CREATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_destroy(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_COMM_DESTROY);

    MPIDI_CH4U_destroy_comm(comm);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_COMM_DESTROY);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* COMM_H_INCLUDED */
