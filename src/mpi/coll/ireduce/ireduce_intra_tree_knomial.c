/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2017 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2018 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include "mpiimpl.h"

/* generate gentran algo prototypes */
#include "tsp_gentran.h"
#include "ireduce_tsp_tree_algos_prototypes.h"
#include "tsp_undef.h"

#undef FUNCNAME
#define FUNCNAME MPIR_Ireduce_intra_tree_knomial
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIR_Ireduce_intra_tree_knomial(const void *sendbuf, void *recvbuf, int count,
                                    MPI_Datatype datatype, MPI_Op op, int root,
                                    MPIR_Comm * comm_ptr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPII_Gentran_Ireduce_intra_tree(sendbuf, recvbuf, count, datatype, op, root,
                                                comm_ptr, request, TREE_TYPE_KNOMIAL,
                                                MPIR_CVAR_IREDUCE_TREE_KVAL,
                                                MPIR_CVAR_IREDUCE_TREE_PIPELINE_CHUNK_SIZE);

    return mpi_errno;
}
