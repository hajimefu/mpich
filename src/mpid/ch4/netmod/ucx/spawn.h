/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2015 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */
#ifndef NETMOD_UCX_SPAWN_H_INCLUDED
#define NETMOD_UCX_SPWAN_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_comm_open_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_open_port(MPID_Info * info_ptr, char *port_name)
{
    int mpi_errno = MPI_SUCCESS;
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_comm_close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_close_port(const char *port_name)
{
    int mpi_errno = MPI_SUCCESS;
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_connect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_connect(const char *port_name,
                                            MPID_Info * info,
                                            int root, MPID_Comm * comm_ptr, MPID_Comm ** newcomm)
{
    int mpi_errno = MPI_SUCCESS;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;



}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_disconnect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_disconnect(MPID_Comm * comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_COMM_DISCONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_COMM_DISCONNECT);



  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_COMM_DISCONNECT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_comm_close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_accept(const char *port_name,
                                           MPID_Info * info,
                                           int root, MPID_Comm * comm_ptr, MPID_Comm ** newcomm)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_COMM_ACCEPT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_COMM_ACCEPT);



  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_COMM_ACCEPT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif
