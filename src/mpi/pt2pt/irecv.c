/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/* -- Begin Profiling Symbol Block for routine MPI_Irecv */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Irecv = PMPI_Irecv
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Irecv  MPI_Irecv
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Irecv as PMPI_Irecv
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
              MPI_Comm comm, MPI_Request *request) __attribute__((weak,alias("PMPI_Irecv")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPI_Irecv
#define MPI_Irecv PMPI_Irecv

#endif

#undef FUNCNAME
#define FUNCNAME MPI_Irecv

/*@
    MPI_Irecv - Begins a nonblocking receive

Input Parameters:
+ buf - initial address of receive buffer (choice) 
. count - number of elements in receive buffer (integer) 
. datatype - datatype of each receive buffer element (handle) 
. source - rank of source (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

Output Parameters:
. request - communication request (handle) 

.N ThreadSafe

.N Fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
.N MPI_ERR_EXHAUSTED
@*/
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
	      int tag, MPI_Comm comm, MPI_Request *request)
{
    static const char FCNAME[] = "MPI_Irecv";
    int mpi_errno = MPI_SUCCESS;
    int cs_enter_success = 0;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Request *request_ptr = NULL;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPI_IRECV);

    MPIR_ERRTEST_INITIALIZED_ORDIE();
    
    MPIR_FUNC_TERSE_PT2PT_ENTER_BACK(MPID_STATE_MPI_IRECV);

    /* Validate handle parameters needing to be converted */
#   ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
	    MPIR_ERRTEST_COMM(comm, mpi_errno);
	}
        MPID_END_ERROR_CHECKS;
    }
#   endif /* HAVE_ERROR_CHECKING */
    
    /* Convert MPI object handles to object pointers */
    MPIR_Comm_get_ptr( comm, comm_ptr );

    /* Validate parameters if error checking is enabled */
#   ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            MPIR_Comm_valid_ptr( comm_ptr, mpi_errno, FALSE );
            if (mpi_errno) goto fn_fail;
	    
	    MPIR_ERRTEST_COUNT(count, mpi_errno);
	    MPIR_ERRTEST_RECV_RANK(comm_ptr, source, mpi_errno);
	    MPIR_ERRTEST_RECV_TAG(tag, mpi_errno);
	    MPIR_ERRTEST_ARGNULL(request,"request",mpi_errno);

	    /* Validate datatype handle */
	    MPIR_ERRTEST_DATATYPE(datatype, "datatype", mpi_errno);
	    
	    /* Validate datatype object */
	    if (HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN)
	    {
		MPIR_Datatype *datatype_ptr = NULL;

		MPID_Datatype_get_ptr(datatype, datatype_ptr);
		MPIR_Datatype_valid_ptr(datatype_ptr, mpi_errno);
		if (mpi_errno) goto fn_fail;
		MPID_Datatype_committed_ptr(datatype_ptr, mpi_errno);
		if (mpi_errno) goto fn_fail;
	    }
	    
	    /* Validate buffer */
	    MPIR_ERRTEST_USERBUFFER(buf,count,datatype,mpi_errno);
        }
        MPID_END_ERROR_CHECKS;
    }
#   endif /* HAVE_ERROR_CHECKING */

    /* ... body of routine ...  */

#if !defined(MPIQ_QUEUE_MODEL)
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

    mpi_errno = MPID_Irecv(buf, count, datatype, source, tag, comm_ptr, 
			   MPIR_CONTEXT_INTRA_PT2PT, &request_ptr);
    /* return the handle of the request to the user */
    /* MPIU_OBJ_HANDLE_PUBLISH is unnecessary for irecv, lower-level access is
     * responsible for its own consistency, while upper-level field access is
     * controlled by the completion counter */
    *request = request_ptr->handle;

    /* Put this part after setting the request so that if the request is
     * pending (which is still considered an error), it will still be set
     * correctly here. For real error cases, the user might get garbage as
     * their request value, but that's fine since the definition is
     * undefined anyway. */
    if (mpi_errno != MPI_SUCCESS) goto fn_fail;
#else
    MPID_THREAD_CS_TRYENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX, cs_enter_success);
    if (cs_enter_success) {
        /* Got the lock: progress all pending ops */
        /* First flush what's in the queue in FIFO order */
        MPIQ_pt2pt_elemt_t* pt2pt_elemt = NULL;
        zm_glqueue_dequeue(&comm_ptr->pend_ops_q, (void**)&pt2pt_elemt);
        while(pt2pt_elemt != NULL) {
            switch(pt2pt_elemt->op) {
                case MPIQ_ISEND:
                    mpi_errno = MPID_Isend( pt2pt_elemt->send_buf,
                                            pt2pt_elemt->count,
                                            pt2pt_elemt->datatype,
                                            pt2pt_elemt->rank,
                                            pt2pt_elemt->tag,
                                            comm_ptr,
                                            MPIR_CONTEXT_INTRA_PT2PT,
                                            &request_ptr);
                    if (mpi_errno != MPI_SUCCESS) goto fn_fail;
                    MPII_SENDQ_REMEMBER(request_ptr,pt2pt_elemt->rank,pt2pt_elemt->tag,comm_ptr->context_id);
                    *pt2pt_elemt->request = request_ptr->handle;
                case MPIQ_IRECV:
                    mpi_errno = MPID_Irecv( pt2pt_elemt->recv_buf,
                                            pt2pt_elemt->count,
                                            pt2pt_elemt->datatype,
                                            pt2pt_elemt->rank,
                                            pt2pt_elemt->tag,
                                            comm_ptr,
                                            MPIR_CONTEXT_INTRA_PT2PT,
                                            &request_ptr);
                    *pt2pt_elemt->request = request_ptr->handle;
                    if (mpi_errno != MPI_SUCCESS) goto fn_fail;
            }
            MPID_Free_mem(pt2pt_elemt);
            zm_glqueue_dequeue(&comm_ptr->pend_ops_q, (void**)&pt2pt_elemt);
        }
        /* Issue my own operation */
        mpi_errno = MPID_Irecv(buf, count, datatype, source, tag, comm_ptr, 
			   MPIR_CONTEXT_INTRA_PT2PT, &request_ptr);
        *request = request_ptr->handle;

        if (mpi_errno != MPI_SUCCESS) goto fn_fail;
    } else {
        /* Failed to acquire the lock: enqueue my work */
        /* First return to the user a handle different from MPI_REQUEST_NULL */
        *request = MPIQ_REQUEST_IDLE;
        MPIQ_pt2pt_elemt_t* pt2pt_elemt = NULL;
        pt2pt_elemt = MPID_Alloc_mem(sizeof *pt2pt_elemt, NULL);
        pt2pt_elemt->op       = MPIQ_IRECV;
        pt2pt_elemt->send_buf = NULL;
        pt2pt_elemt->recv_buf = buf;
        pt2pt_elemt->count    = count;
        pt2pt_elemt->datatype = datatype;
        pt2pt_elemt->rank     = source;
        pt2pt_elemt->tag      = tag;
        pt2pt_elemt->request  = request;
        zm_glqueue_enqueue(&comm_ptr->pend_ops_q, pt2pt_elemt);
    }
#endif

    /* ... end of body of routine ... */
    
  fn_exit:
    MPIR_FUNC_TERSE_PT2PT_EXIT_BACK(MPID_STATE_MPI_IRECV);
#if defined(MPIQ_QUEUE_MODEL)
    if(cs_enter_success)
#endif
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    return mpi_errno;

  fn_fail:
    /* --BEGIN ERROR HANDLING-- */
#   ifdef HAVE_ERROR_CHECKING
    {
	mpi_errno = MPIR_Err_create_code(
	    mpi_errno, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__, MPI_ERR_OTHER, "**mpi_irecv",
	    "**mpi_irecv %p %d %D %i %t %C %p", buf, count, datatype, source, tag, comm, request);
    }
#   endif
    mpi_errno = MPIR_Err_return_comm( comm_ptr, FCNAME, mpi_errno );
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}
