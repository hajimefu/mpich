/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2018 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2018 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include "mt_pt2pt_common.h"

/*
 * This function provides a framework to support different MT tests. It creates
 * multiple scenarios for the threads in terms of what communicators and what
 * tags they use and then launches the MT test routine 'run_test'. An MT test
 * should provide an implementation of the 'run_test' method.
 */
int main(int argc, char **argv)
{
    int i, pmode, nprocs, rank, errs = 0, err;
    struct thread_param params[NTHREADS];
    MPI_Comm dup_worlds[NTHREADS];

    MTest_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &pmode);
    if (pmode != MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI_THREAD_MULTIPLE not supported by the MPI implementation\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (nprocs != 2) {
        fprintf(stderr, "Need two processes\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    err = MTest_thread_barrier_init();
    if (err) {
        fprintf(stderr, "Could not create thread barrier\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Setup bufer for bsend */
#if defined(BSEND) || defined(IBSEND)
    {
        int n_experiment_grps = 3;
        int n_bsend_calls = ITER * NTHREADS * n_experiment_grps;
        int max_outstanding_bsend_calls = ITER * NTHREADS;
        int data_size = sizeof(int) * COUNT;
        int size = data_size * max_outstanding_bsend_calls      /* bsend buffer */
            + MPI_BSEND_OVERHEAD * n_bsend_calls;       /* bsend overhead */
        void *buff = malloc(size);
        if (!buff) {
            fprintf(stderr, "Could not allocate buffer for bsend\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        MPI_Buffer_attach(buff, size);
    }
#endif /* #if defined(BSEND) || defined(IBSEND) */

    /*
     * Test 1 - all threads perform send/recv on MPI_COMM_WORLD with the same tag
     */
    if (rank == 0)
        MTestPrintfMsg(1, "Testing MT Send-Recv with same comm, same tag\n");

    for (i = 0; i < NTHREADS; i++) {
        params[i].id = i;
        params[i].buff = malloc(sizeof(int) * BUFF_ELEMENT_COUNT);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (i = 0; i < NTHREADS; i++) {
        params[i].comm = MPI_COMM_WORLD;
        params[i].tag = 0;
        params[i].verify = 0;   /* Since all threads perform communications over the
                                 * same {comm, rank, tag}, message matching order becomes
                                 * non-deterministic, so don't verify received data */
        if (i == NTHREADS - 1)
            run_test(&params[i]);
        else
            MTest_Start_thread(run_test, &params[i]);
    }
    MTest_Join_threads();

    for (i = 0; i < NTHREADS; i++)
        errs += params[i].result;

    MPI_Barrier(MPI_COMM_WORLD);

    /*
     * Test 2 - all threads perform send/recv over MPI_COMM_WORLD,
     * but each thread uses different tag -- thread `i` uses tag `i`.
     * Thus thread `i` in rank 0 exclusively communicate with thread `i`
     * in rank 1.
     */
    if (rank == 0)
        MTestPrintfMsg(1, "Testing MT Send-Recv with same comm, per-thread tag\n");

    for (i = 0; i < NTHREADS; i++) {
        params[i].comm = MPI_COMM_WORLD;
        params[i].tag = i;
        params[i].verify = 1;
        if (i == NTHREADS - 1)
            run_test(&params[i]);
        else
            MTest_Start_thread(run_test, &params[i]);
    }
    MTest_Join_threads();

    for (i = 0; i < NTHREADS; i++)
        errs += params[i].result;

    MPI_Barrier(MPI_COMM_WORLD);

    /*
     * Test 3 - Every thread performs send/recv over different communicator.
     * Thread `i` in both ranks communicate with each other.
     * All threads use the same tag.
     */
    if (rank == 0)
        MTestPrintfMsg(1, "Testing MT Send-Recv with per-thread comm, same tag\n");

    for (i = 0; i < NTHREADS; i++) {
        MPI_Comm_dup(MPI_COMM_WORLD, &dup_worlds[i]);
        params[i].comm = dup_worlds[i];
        params[i].tag = 0;
        params[i].verify = 1;
        if (i == NTHREADS - 1)
            run_test(&params[i]);
        else
            MTest_Start_thread(run_test, &params[i]);
    }
    MTest_Join_threads();

    MPI_Barrier(MPI_COMM_WORLD);

    for (i = 0; i < NTHREADS; i++) {
        MPI_Comm_free(&dup_worlds[i]);
        errs += params[i].result;
        free(params[i].buff);
    }

    if (rank == 0)
        MTestPrintfMsg(1, "All tests done, cleaning up.\n");

    MTest_thread_barrier_free();

#if defined(BSEND) || defined(IBSEND)
    {
        void *buff;
        int size;
        MPI_Buffer_detach(&buff, &size);
        free(buff);
    }
#endif

    MTest_Finalize(errs);

    return errs;
}
