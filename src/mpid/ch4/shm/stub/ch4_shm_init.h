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
#ifndef CH4_SHM_STUB_INIT_H_INCLUDED
#define CH4_SHM_STUB_INIT_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_CH4_SHM_init(int rank, int size)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_finalize(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void *MPIDI_CH4_SHM_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    MPIU_Assert(0);
    return NULL;
}

static inline int MPIDI_CH4_SHM_free_mem(void *ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_comm_get_lpid(MPID_Comm * comm_ptr,
                                          int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    *id_p = (MPID_Node_id_t) 0;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    *max_id_p = (MPID_Node_id_t) 1;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_getallincomm(MPID_Comm * comm_ptr,
                                         int local_size, MPIR_Gpid local_gpids[], int *singlePG)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                        int size, const int lpids[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_INIT_H_INCLUDED */
