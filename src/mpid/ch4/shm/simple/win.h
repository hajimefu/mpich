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
#ifndef SHM_SIMPLE_WIN_H_INCLUDED
#define SHM_SIMPLE_WIN_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_SHM_win_set_info(MPIR_Win *win, MPIR_Info *info)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_start(MPIR_Group *group, int assert, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_complete(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_post(MPIR_Group *group, int assert, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_wait(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_test(MPIR_Win *win, int *flag)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_lock(int lock_type, int rank, int assert, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_unlock(int rank, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_get_info(MPIR_Win *win, MPIR_Info **info_p_p)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_win_free(MPIR_Win **win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_fence(int assert, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_create(void *base,
                                           MPI_Aint length,
                                           int disp_unit,
                                           MPIR_Info *info, MPIR_Comm *comm_ptr, MPIR_Win **win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_attach(MPIR_Win *win, void *base, MPI_Aint size)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_allocate_shared(MPI_Aint size,
                                                    int disp_unit,
                                                    MPIR_Info *info_ptr,
                                                    MPIR_Comm *comm_ptr,
                                                    void **base_ptr, MPIR_Win **win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_detach(MPIR_Win *win, const void *base)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_shared_query(MPIR_Win *win,
                                                 int rank,
                                                 MPI_Aint *size, int *disp_unit, void *baseptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_allocate(MPI_Aint size,
                                             int disp_unit,
                                             MPIR_Info *info,
                                             MPIR_Comm *comm, void *baseptr, MPIR_Win **win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_flush(int rank, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_flush_local_all(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_unlock_all(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_create_dynamic(MPIR_Info *info, MPIR_Comm *comm, MPIR_Win **win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_flush_local(int rank, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_sync(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_flush_all(MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_win_lock_all(int assert, MPIR_Win *win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


#endif /* SHM_SIMPLE_WIN_H_INCLUDED */
