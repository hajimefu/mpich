/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/* -- Begin Profiling Symbol Block for routine MPI_Isend */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Isend = PMPI_Isend
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Isend  MPI_Isend
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Isend as PMPI_Isend
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request) __attribute__((weak,alias("PMPI_Isend")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPI_Isend
#define MPI_Isend PMPI_Isend

#endif

#undef FUNCNAME
#define FUNCNAME MPI_Isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
/*@
    MPI_Isend - Begins a nonblocking send

Input Parameters:
+ buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

Output Parameters:
. request - communication request (handle) 

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
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	      MPI_Comm comm, MPI_Request *request)
{
    int mpi_errno = MPI_SUCCESS;
    int cs_enter_success = 0;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Request *request_ptr = NULL;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPI_ISEND);

    MPIR_ERRTEST_INITIALIZED_ORDIE();
    
    MPIR_FUNC_TERSE_PT2PT_ENTER_FRONT(MPID_STATE_MPI_ISEND);

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
    /* TODO: Issue an acquire memory fence? */
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
	    MPIR_ERRTEST_SEND_RANK(comm_ptr, dest, mpi_errno);
	    MPIR_ERRTEST_SEND_TAG(tag, mpi_errno);
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
    while (!cs_enter_success)
        MPID_THREAD_CS_TRYENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX, cs_enter_success);

    mpi_errno = MPID_Isend(buf, count, datatype, dest, tag, comm_ptr,
			   MPIR_CONTEXT_INTRA_PT2PT, &request_ptr);
    if (mpi_errno != MPI_SUCCESS) goto fn_fail;
    MPII_SENDQ_REMEMBER(request_ptr,dest,tag,comm_ptr->context_id);
    /* return the handle of the request to the user */
    /* MPIU_OBJ_HANDLE_PUBLISH is unnecessary for isend, lower-level access is
     * responsible for its own consistency, while upper-level field access is
     * controlled by the completion counter */
    *request = request_ptr->handle;
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
                    mpi_errno = MPID_Isend( pt2pt_elemt->buf,
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
            }
            MPID_Free_mem(pt2pt_elemt);
            zm_glqueue_dequeue(&comm_ptr->pend_ops_q, (void**)&pt2pt_elemt);
        }
        /* Issue my own operation */
        mpi_errno = MPID_Isend(buf, count, datatype, dest, tag, comm_ptr,
                               MPIR_CONTEXT_INTRA_PT2PT, &request_ptr);
        if (mpi_errno != MPI_SUCCESS) goto fn_fail;
        MPII_SENDQ_REMEMBER(request_ptr,dest,tag,comm_ptr->context_id);
       /* return the handle of the request to the user */
       /* MPIU_OBJ_HANDLE_PUBLISH is unnecessary for isend, lower-level access is
        * responsible for its own consistency, while upper-level field access is
        * controlled by the completion counter */
        *request = request_ptr->handle;
    } else {
        /* Failed to acquire the lock: enqueue my work */
        /* First return to the user a handle different from MPI_REQUEST_NULL */
        *request = MPIQ_REQUEST_IDLE;
        MPIQ_pt2pt_elemt_t* pt2pt_elemt = NULL;
        pt2pt_elemt = MPID_Alloc_mem(sizeof *pt2pt_elemt, NULL);
        pt2pt_elemt->op       = MPIQ_ISEND;
        pt2pt_elemt->buf      = buf;
        pt2pt_elemt->count    = count;
        pt2pt_elemt->datatype = datatype;
        pt2pt_elemt->rank     = dest;
        pt2pt_elemt->tag      = tag;
        pt2pt_elemt->request  = request;
        zm_glqueue_enqueue(&comm_ptr->pend_ops_q, pt2pt_elemt);
    }
#endif

    /* ... end of body of routine ... */
    
  fn_exit:
    MPIR_FUNC_TERSE_PT2PT_EXIT(MPID_STATE_MPI_ISEND);
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
	    mpi_errno, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__, MPI_ERR_OTHER, "**mpi_isend",
	    "**mpi_isend %p %d %D %i %t %C %p", buf, count, datatype, dest, tag, comm, request);
    }
#   endif
    mpi_errno = MPIR_Err_return_comm( comm_ptr, FCNAME, mpi_errno );
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}
