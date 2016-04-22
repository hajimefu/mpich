/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

#undef FUNCNAME
#define FUNCNAME MPID_Barrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Barrier(MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Barrier_impl(comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Bcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Bcast(void * buffer, int count, MPI_Datatype datatype, int root,
               MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Bcast_impl(buffer, count, datatype, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Allreduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Allreduce(const void * sendbuf, void * recvbuf, int count,
                   MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                   MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Allreduce_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                   MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                    void * recvbuf, const int * recvcounts, const int * displs,
                    MPI_Datatype recvtype, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcounts, displs, recvtype, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Scatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                 void * recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Scatter_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
            recvtype, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Scatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Scatterv(const void * sendbuf, const int * sendcounts, const int * displs,
                  MPI_Datatype sendtype, void * recvbuf, int recvcount,
                  MPI_Datatype recvtype, int root, MPIR_Comm * comm,
                  MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Scatterv_impl(sendbuf, sendcounts, displs, sendtype, recvbuf,
            recvcount, recvtype, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Gather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Gather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                void * recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Gather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
            recvtype, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Gatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Gatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                 void * recvbuf, const int * recvcounts, const int * displs,
                 MPI_Datatype recvtype, int root, MPIR_Comm * comm,
                 MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Gatherv_impl(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
            displs, recvtype, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                  void * recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Alltoall_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
            recvtype, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls,
                   MPI_Datatype sendtype, void * recvbuf, const int * recvcounts,
                   const int * rdispls, MPI_Datatype recvtype, MPIR_Comm * comm,
                   MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype, recvbuf,
            recvcounts, rdispls, recvtype, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Alltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[],
                   const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[],
                   const int rdispls[], const MPI_Datatype recvtypes[],
                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Alltoallw_impl(sendbuf, sendcounts, sdispls,
                               sendtypes, recvbuf, recvcounts,
                               rdispls, recvtypes, comm_ptr, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Reduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Reduce(const void * sendbuf, void * recvbuf, int count,
                MPI_Datatype datatype, MPI_Op op, int root,
                MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Reduce_impl(sendbuf, recvbuf, count, datatype, op, root, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Reduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Reduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[],
                        MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                        MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Reduce_scatter_impl(sendbuf, recvbuf, recvcounts,
                                    datatype, op, comm_ptr, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Reduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Reduce_scatter_block(const void *sendbuf, void *recvbuf,
                              int recvcount, MPI_Datatype datatype,
                              MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Reduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
                                          datatype, op, comm_ptr, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Scan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Scan(const void * sendbuf, void * recvbuf, int count,
              MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
              MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Scan_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Exscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Exscan(const void * sendbuf, void * recvbuf, int count,
                MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Exscan_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Neighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Neighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                            void * recvbuf, int recvcount, MPI_Datatype recvtype,
                            MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Neighbor_allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Neighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Neighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                             void * recvbuf, const int recvcounts[], const int displs[],
                             MPI_Datatype recvtype, MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Neighbor_allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcounts, displs, recvtype, comm);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Neighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Neighbor_alltoallv(const void * sendbuf, const int sendcounts[],
                            const int sdispls[], MPI_Datatype sendtype,
                            void * recvbuf, const int recvcounts[],
                            const int rdispls[], MPI_Datatype recvtype,
                            MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Neighbor_alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
            recvbuf, recvcounts, rdispls, recvtype, comm);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Neighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Neighbor_alltoallw(const void * sendbuf, const int sendcounts[],
                            const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                            void * recvbuf, const int recvcounts[],
                            const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                            MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Neighbor_alltoallw_impl(sendbuf, sendcounts, sdispls,
            sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Neighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Neighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                           void * recvbuf, int recvcount, MPI_Datatype recvtype,
                           MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Neighbor_alltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ineighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ineighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                             void * recvbuf, int recvcount, MPI_Datatype recvtype,
                             MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ineighbor_allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ineighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ineighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                              void * recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype,
                              MPIR_Comm * comm, MPI_Request *request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ineighbor_allgatherv_impl(sendbuf, sendcount, sendtype,
            recvbuf, recvcounts, displs, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ineighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ineighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                            void * recvbuf, int recvcount, MPI_Datatype recvtype,
                            MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ineighbor_alltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ineighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ineighbor_alltoallv(const void * sendbuf, const int sendcounts[],
                             const int sdispls[], MPI_Datatype sendtype,
                             void * recvbuf, const int recvcounts[],
                             const int rdispls[], MPI_Datatype recvtype,
                             MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ineighbor_alltoallv_impl(sendbuf, sendcounts, sdispls,
            sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ineighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ineighbor_alltoallw(const void * sendbuf, const int sendcounts[],
                             const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                             void * recvbuf, const int recvcounts[],
                             const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                             MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ineighbor_alltoallw_impl(sendbuf, sendcounts, sdispls,
            sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ibarrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ibarrier(MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ibarrier_impl(comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ibcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ibcast(void * buffer, int count, MPI_Datatype datatype, int root,
                MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ibcast_impl(buffer, count, datatype, root, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iallgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iallgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                    void * recvbuf, int recvcount, MPI_Datatype recvtype,
                    MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iallgather_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iallgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iallgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                     void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype,
                     MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iallgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcounts, displs, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iallreduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iallreduce(const void * sendbuf, void * recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                    MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iallreduce_impl(sendbuf, recvbuf, count, datatype, op, comm,
            request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ialltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ialltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                   MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ialltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ialltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ialltoallv(const void * sendbuf, const int sendcounts[],
                    const int sdispls[], MPI_Datatype sendtype,
                    void * recvbuf, const int recvcounts[],
                    const int rdispls[], MPI_Datatype recvtype,
                    MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ialltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
            recvbuf, recvcounts, rdispls, recvtype, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ialltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ialltoallw(const void * sendbuf, const int sendcounts[],
                    const int sdispls[], const MPI_Datatype sendtypes[],
                    void * recvbuf, const int recvcounts[],
                    const int rdispls[], const MPI_Datatype recvtypes[],
                    MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ialltoallw_impl(sendbuf, sendcounts, sdispls, sendtypes,
            recvbuf, recvcounts, rdispls, recvtypes, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iexscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iexscan(const void * sendbuf, void * recvbuf, int count,
                 MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                 MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iexscan_impl(sendbuf, recvbuf, count, datatype, op, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Igather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Igather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                 void * recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Igather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
            recvtype, root, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Igatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Igatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                  void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype,
                  int root, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Igatherv_impl(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
            displs, recvtype, root, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ireduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ireduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount,
                               MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                               MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ireduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
            datatype, op, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ireduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ireduce_scatter(const void * sendbuf, void * recvbuf, const int recvcounts[],
                         MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                         MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ireduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype,
            op, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Ireduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Ireduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype,
                 MPI_Op op, int root, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Ireduce_impl(sendbuf, recvbuf, count, datatype, op, root, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iscan_impl(sendbuf, recvbuf, count, datatype, op, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iscatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iscatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                  void * recvbuf, int recvcount, MPI_Datatype recvtype,
                  int root, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iscatter_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
            recvtype, root, comm, request);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Iscatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_Iscatterv(const void * sendbuf, const int * sendcounts,
                   const int * displs, MPI_Datatype sendtype,
                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                   int root, MPIR_Comm * comm, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno = MPIR_Iscatterv_impl(sendbuf, sendcounts, displs, sendtype, recvbuf,
            recvcount, recvtype, root, comm, request);

    return mpi_errno;
}
