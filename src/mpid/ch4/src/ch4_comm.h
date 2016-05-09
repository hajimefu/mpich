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
#ifndef CH4_COMM_H_INCLUDED
#define CH4_COMM_H_INCLUDED

#include "ch4_impl.h"
#include "ch4i_comm.h"

__CH4_INLINE__ int MPIDI_Comm_AS_enabled(MPIR_Comm * comm)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_reenable_anysource(MPIR_Comm * comm, MPIR_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_remote_group_failed(MPIR_Comm * comm, MPIR_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_group_failed(MPIR_Comm * comm_ptr, MPIR_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_failure_ack(MPIR_Comm * comm_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_failure_get_acked(MPIR_Comm * comm_ptr,
                                                MPIR_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_revoke(MPIR_Comm * comm_ptr, int is_remote)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_get_all_failed_procs(MPIR_Comm * comm_ptr, MPIR_Group ** failed_group,
                                                   int tag)
{
    MPIU_Assert(0);
    return 0;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_split_type
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_split_type(MPIR_Comm  *comm_ptr,
                                         int         split_type,
                                         int         key,
                                         MPIR_Info  *info_ptr,
                                         MPIR_Comm **newcomm_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    int            idx;
    MPID_Node_id_t node_id;

    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_SPLIT_TYPE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_SPLIT_TYPE);

    if(split_type == MPI_COMM_TYPE_SHARED) {
        MPIDI_Comm_get_lpid(comm_ptr,comm_ptr->rank,&idx,FALSE);
        MPIDI_Get_node_id(comm_ptr, comm_ptr->rank, &node_id);
        mpi_errno              = MPIR_Comm_split_impl(comm_ptr,node_id,key,newcomm_ptr);
    } else
        mpi_errno              = MPIR_Comm_split_impl(comm_ptr, MPI_UNDEFINED, key, newcomm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_SPLIT_TYPE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_create(MPIR_Comm * comm)
{
    int mpi_errno;
    int i, *uniq_avtids;
    int max_n_avts;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_CREATE);
    mpi_errno = MPIDI_NM_comm_create(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#if defined(MPIDI_BUILD_CH4_SHM)
    mpi_errno = MPIDI_SHM_comm_create(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

    /* comm_world and comm_self are already initialized */
    if (comm != MPIR_Process.comm_world && comm != MPIR_Process.comm_self) {
        MPIDII_comm_create_rank_map(comm);
        /* add ref to avts */
        switch (MPIDII_COMM(comm,map).mode) {
            case MPIDII_RANK_MAP_NONE:
                break;
            case MPIDII_RANK_MAP_MLUT:
                max_n_avts = MPIDIU_get_max_n_avts();
                uniq_avtids = (int *) MPL_malloc(max_n_avts * sizeof(int));
                memset(uniq_avtids, 0, max_n_avts);
                for (i = 0; i < MPIDII_COMM(comm,map).size; i++) {
                    if (uniq_avtids[MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid] == 0) {
                        uniq_avtids[MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid] = 1;
                        MPIDIU_avt_add_ref(MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid);
                    }
                }
                MPL_free(uniq_avtids);
                break;
            default:
                MPIDIU_avt_add_ref(MPIDII_COMM(comm,map).avtid);
        }

        switch (MPIDII_COMM(comm,local_map).mode) {
            case MPIDII_RANK_MAP_NONE:
                break;
            case MPIDII_RANK_MAP_MLUT:
                max_n_avts = MPIDIU_get_max_n_avts();
                uniq_avtids = (int *) MPL_malloc(max_n_avts * sizeof(int));
                memset(uniq_avtids, 0, max_n_avts);
                for (i = 0; i < MPIDII_COMM(comm,local_map).size; i++) {
                    if (uniq_avtids[MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid] == 0) {
                        uniq_avtids[MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid] = 1;
                        MPIDIU_avt_add_ref(MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid);
                    }
                }
                MPL_free(uniq_avtids);
                break;
            default:
                MPIDIU_avt_add_ref(MPIDII_COMM(comm,local_map).avtid);
        }
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
__CH4_INLINE__ int MPIDI_Comm_destroy(MPIR_Comm * comm)
{
    int mpi_errno;
    int i, *uniq_avtids;
    int max_n_avts;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_DESTROY);
    /* release ref to avts */
    switch (MPIDII_COMM(comm,map).mode) {
        case MPIDII_RANK_MAP_NONE:
            break;
        case MPIDII_RANK_MAP_MLUT:
            max_n_avts = MPIDIU_get_max_n_avts();
            uniq_avtids = (int *) MPL_malloc(max_n_avts * sizeof(int));
            memset(uniq_avtids, 0, max_n_avts);
            for (i = 0; i < MPIDII_COMM(comm,map).size; i++) {
                if (uniq_avtids[MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid] == 0) {
                    uniq_avtids[MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid] = 1;
                    MPIDIU_avt_release_ref(MPIDII_COMM(comm,map).irreg.mlut.gpid[i].avtid);
                }
            }
            MPL_free(uniq_avtids);
            break;
        default:
            MPIDIU_avt_release_ref(MPIDII_COMM(comm,map).avtid);
    }

    switch (MPIDII_COMM(comm,local_map).mode) {
        case MPIDII_RANK_MAP_NONE:
            break;
        case MPIDII_RANK_MAP_MLUT:
            max_n_avts = MPIDIU_get_max_n_avts();
            uniq_avtids = (int *) MPL_malloc(max_n_avts * sizeof(int));
            memset(uniq_avtids, 0, max_n_avts);
            for (i = 0; i < MPIDII_COMM(comm,local_map).size; i++) {
                if (uniq_avtids[MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid] == 0) {
                    uniq_avtids[MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid] = 1;
                    MPIDIU_avt_release_ref(MPIDII_COMM(comm,local_map).irreg.mlut.gpid[i].avtid);
                }
            }
            MPL_free(uniq_avtids);
            break;
        default:
            MPIDIU_avt_release_ref(MPIDII_COMM(comm,local_map).avtid);
    }

    mpi_errno = MPIDI_NM_comm_destroy(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#if defined(MPIDI_BUILD_CH4_SHM)
    mpi_errno = MPIDI_SHM_comm_destroy(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

    if (MPIDII_COMM(comm,map).mode == MPIDII_RANK_MAP_LUT) {
        MPIDIU_release_lut(MPIDII_COMM(comm,map).irreg.lut.t);
    }
    if (MPIDII_COMM(comm,local_map).mode == MPIDII_RANK_MAP_LUT) {
        MPIDIU_release_lut(MPIDII_COMM(comm,local_map).irreg.lut.t);
    }
    if (MPIDII_COMM(comm,map).mode == MPIDII_RANK_MAP_MLUT) {
        MPIDIU_release_mlut(MPIDII_COMM(comm,map).irreg.mlut.t);
    }
    if (MPIDII_COMM(comm,local_map).mode == MPIDII_RANK_MAP_MLUT) {
        MPIDIU_release_mlut(MPIDII_COMM(comm,local_map).irreg.mlut.t);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_DESTROY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* CH4_COMM_H_INCLUDED */
