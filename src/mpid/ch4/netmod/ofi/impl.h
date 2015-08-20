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
#ifndef NETMOD_OFI_IMPL_H_INCLUDED
#define NETMOD_OFI_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "types.h"
#include "mpidch4u.h"
#include "../../src/ch4_impl.h"

/* The purpose of this hacky #ifdef is to tag                */
/* select MPI util functions with always inline.  A better   */
/* approach would be to declare them inline in a header file */
/* In a static library with the "always_inline" attribute    */
/* This allows an application with ipo/pgo enabled to inline */
/* the function directly into the application despite        */
/* mpi header files not using attribute inline               */
/* Trick doesn't work with gcc or clang                      */
#if defined(__INTEL_COMPILER)
#ifndef __cplusplus
struct MPIU_Object_alloc_t;
#define ILU(ret,fcname,...) inline __attribute__((always_inline)) ret MPIU_##fcname(__VA_ARGS__)
ILU(void *, Handle_obj_alloc_unsafe, struct MPIU_Object_alloc_t *);
ILU(void *, Handle_obj_alloc, struct MPIU_Object_alloc_t *);
ILU(void *, Handle_direct_init, void *, int, int, int);
ILU(void *, Handle_indirect_init, void *(**)[], int *, int, int, int, int);
ILU(void, Handle_obj_alloc_complete, struct MPIU_Object_alloc_t *, int);
ILU(void, Handle_obj_free, struct MPIU_Object_alloc_t *objmem, void *object);
ILU(void *, Handle_get_ptr_indirect, int, struct MPIU_Object_alloc_t *);
#undef ILU
#endif /* __cplusplus */
#endif /* __clang__ || __INTEL_COMPILER */

/*
 * Helper routines and macros for request completion
 */
#define MPIDI_Win_request_tls_alloc(req)                                \
  ({                                                                           \
  (req) = (MPIDI_Win_request*)MPIU_Handle_obj_alloc(&MPIDI_Request_mem); \
  if (req == NULL)                                                           \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Win Request");   \
  })

#define MPIDI_Win_request_tls_free(req) \
  MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Win_request_complete(req)                 \
  ({                                                    \
  int count;                                          \
  MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
              == MPID_REQUEST);                       \
  MPIU_Object_release_ref(req, &count);               \
  MPIU_Assert(count >= 0);                            \
  if (count == 0)                                     \
    {                                                 \
      MPIU_Free(req->noncontig);                      \
      MPIDI_Win_request_tls_free(req);                \
    }                                                 \
  })

#define MPIDI_Win_request_alloc_and_init(req,count)             \
  ({                                                            \
    MPIDI_Win_request_tls_alloc(req);                           \
    MPIU_Assert(req != NULL);                                   \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)                \
                == MPID_REQUEST);                               \
    MPIU_Object_set_ref(req, count);                            \
    memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,                \
           sizeof(MPIDI_Win_request)-                           \
           MPIDI_REQUEST_HDR_SIZE);                             \
    req->noncontig = (MPIDI_Win_noncontig*)MPIU_Calloc(1,sizeof(*(req->noncontig))); \
  })

#define MPIDI_Ssendack_request_tls_alloc(req)                           \
  ({                                                                    \
  (req) = (MPIDI_Ssendack_request*)                                     \
    MPIU_Handle_obj_alloc(&MPIDI_Request_mem);                          \
  if (req == NULL)                                                      \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1,                              \
               "Cannot allocate Ssendack Request");                     \
  })

#define MPIDI_Ssendack_request_tls_free(req) \
  MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Ssendack_request_alloc_and_init(req)      \
  ({                                                    \
    MPIDI_Ssendack_request_tls_alloc(req);              \
    MPIU_Assert(req != NULL);                           \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPID_SSENDACK_REQUEST);              \
  })

#define MPIDI_Request_create_null_rreq(rreq_, mpi_errno_, FAIL_)        \
  do {                                                                  \
    (rreq_) = MPIDI_Request_create();                                   \
    if ((rreq_) != NULL) {                                              \
      MPIU_Object_set_ref((rreq_), 1);                                  \
      MPID_cc_set(&(rreq_)->cc, 0);                                     \
      (rreq_)->kind = MPID_REQUEST_RECV;                                \
      MPIR_Status_set_procnull(&(rreq_)->status);                       \
    }                                                                   \
    else {                                                              \
      MPIR_ERR_SETANDJUMP(mpi_errno_,MPI_ERR_OTHER,"**nomemreq");       \
    }                                                                   \
  } while (0)


#define PROGRESS()                                       \
  ({                                                     \
    mpi_errno = MPIDI_Progress_test();                   \
    if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno); \
  })

#define PROGRESS_WHILE(cond)                          \
  ({                                                  \
  while (cond)                                         \
       PROGRESS();                                    \
  })

#define MPIU_CH4_OFI_ERR  MPIR_ERR_CHKANDJUMP4
#define FI_RC(FUNC,STR)                                     \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);   \
        ssize_t _ret = FUNC;                                \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);    \
        MPIU_CH4_OFI_ERR(_ret<0,                            \
                         mpi_errno,                         \
                         MPI_ERR_OTHER,                     \
                         "**ofid_"#STR,                     \
                         "**ofid_"#STR" %s %d %s %s",       \
                         __SHORT_FILE__,                    \
                         __LINE__,                          \
                         FCNAME,                            \
                         fi_strerror(-_ret));               \
    } while (0)

#define FI_RC_RETRY(FUNC,STR)                               \
    do {                                                    \
    ssize_t _ret;                                           \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);   \
        _ret = FUNC;                                        \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);    \
        if(likely(_ret==0)) break;                          \
        MPIU_CH4_OFI_ERR(_ret!=-FI_EAGAIN,                  \
                         mpi_errno,                         \
                         MPI_ERR_OTHER,                     \
                         "**ofid_"#STR,                     \
                         "**ofid_"#STR" %s %d %s %s",       \
                         __SHORT_FILE__,                    \
                         __LINE__,                          \
                         FCNAME,                            \
                         fi_strerror(-_ret));               \
        PROGRESS();                                         \
    } while (_ret == -FI_EAGAIN);                           \
    } while (0)

#define FI_RC_RETRY_NOLOCK(FUNC,STR)                          \
  do                                                          \
    {                                                         \
     ssize_t _ret;                                            \
     do {                                                     \
         _ret = FUNC;                                         \
         if (likely(_ret==0)) break;                          \
         MPIU_CH4_OFI_ERR(_ret!=FI_EAGAIN,                    \
                          mpi_errno,                          \
                          MPI_ERR_OTHER,                      \
                          "**ofid_"#STR,                      \
                          "**ofid_"#STR" %s %d %s %s",        \
                          __SHORT_FILE__,                     \
                          __LINE__,                           \
                          FCNAME,                             \
                          fi_strerror(-_ret));                \
         PROGRESS();                                          \
         if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno); \
     } while (_ret == -FI_EAGAIN);                            \
    } while (0)

#define PMI_RC(FUNC,STR)                            \
  do                                                \
    {                                               \
      pmi_errno  = FUNC;                            \
      MPIU_CH4_OFI_ERR(pmi_errno!=PMI_SUCCESS,      \
                       mpi_errno,                   \
                       MPI_ERR_OTHER,               \
                       "**ofid_"#STR,               \
                       "**ofid_"#STR" %s %d %s %s", \
                       __SHORT_FILE__,              \
                       __LINE__,                    \
                       FCNAME,                      \
                       #STR);                       \
    } while (0)

#define MPI_RC_POP(FUNC)                                        \
  do                                                            \
    {                                                           \
      mpi_errno = FUNC;                                         \
      if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);      \
    } while (0)

#define MPIU_STR_RC(FUNC,STR)                                   \
  do                                                            \
    {                                                           \
      str_errno = FUNC;                                         \
      MPIU_CH4_OFI_ERR(str_errno!=MPIU_STR_SUCCESS,             \
                       mpi_errno,                               \
                       MPI_ERR_OTHER,                           \
                       "**"#STR,                                \
                       "**"#STR" %s %d %s %s",                  \
                       __SHORT_FILE__,                          \
                       __LINE__,                                \
                       FCNAME,                                  \
                       #STR);                                   \
    } while (0)

#define REQ_CREATE(req)                           \
  ({                                              \
    req = MPIDI_Request_alloc_and_init(2);        \
  })


#define WINREQ_CREATE(req)                     \
  ({                                           \
    MPIDI_Win_request_alloc_and_init(req,1);   \
  })

#define SSENDACKREQ_CREATE(req)            \
  ({                                       \
    MPIDI_Ssendack_request_tls_alloc(req); \
  })

/* Common Utility functions used by the
 * C and C++ components
 */
static inline MPID_Request *MPIDI_Request_alloc_and_init(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);

    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");

    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPID_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    return req;
}



static inline fi_addr_t _comm_to_phys(MPID_Comm * comm, int rank, int ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    int ep_num = COMM_TO_EP(comm, rank);
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(COMM_TO_PHYS(comm, rank), rx_idx, MPIDI_MAX_ENDPOINTS_BITS);
#else
    return COMM_TO_PHYS(comm, rank);
#endif
}

static inline fi_addr_t _to_phys(int rank, int ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    int ep_num = 0;
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(TO_PHYS(rank), rx_idx, MPIDI_MAX_ENDPOINTS_BITS);
#else
    return TO_PHYS(rank);
#endif
}

static inline bool is_tag_sync(uint64_t match_bits)
{
    return (0 != (MPID_SYNC_SEND & match_bits));
}

static inline uint64_t init_sendtag(MPIU_Context_id_t contextid, int source, int tag, uint64_t type)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPID_SOURCE_SHIFT);
    match_bits |= source;
    match_bits = (match_bits << MPID_TAG_SHIFT);
    match_bits |= (MPID_TAG_MASK & tag) | type;
    return match_bits;
}

/* receive posting */
static inline uint64_t init_recvtag(uint64_t * mask_bits,
                                    MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits = 0;
    *mask_bits = MPID_PROTOCOL_MASK;
    match_bits = contextid;
    match_bits = (match_bits << MPID_SOURCE_SHIFT);

    if (MPI_ANY_SOURCE == source) {
        match_bits = (match_bits << MPID_TAG_SHIFT);
        *mask_bits |= MPID_SOURCE_MASK;
    }
    else {
        match_bits |= source;
        match_bits = (match_bits << MPID_TAG_SHIFT);
    }

    if (MPI_ANY_TAG == tag)
        *mask_bits |= MPID_TAG_MASK;
    else
        match_bits |= (MPID_TAG_MASK & tag);

    return match_bits;
}

static inline int get_tag(uint64_t match_bits)
{
    return ((int) (match_bits & MPID_TAG_MASK));
}

static inline int get_source(uint64_t match_bits)
{
    return ((int) ((match_bits & MPID_SOURCE_MASK) >> MPID_TAG_SHIFT));
}

#undef FUNCNAME
#define FUNCNAME do_control_win
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_control_win(MPIDI_Win_control_t * control,
                                 int rank, MPID_Win * win, int use_comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);

    CH4_COMPILE_TIME_ASSERT(MPID_MIN_CTRL_MSG_SZ == sizeof(MPIDI_Send_control_t));

    control->win_id = WIN_OFI(win)->win_id;
    control->origin_rank = win->comm_ptr->rank;

    MPIU_Assert(sizeof(*control) <= MPIDI_Global.max_buffered_send);
    /* Should already be holding a lock, so call the non-locking version */
    FI_RC_RETRY_NOLOCK(fi_inject(G_TXC_MSG(0),
                                 control, sizeof(*control),
                                 use_comm ? _comm_to_phys(win->comm_ptr, rank, MPIDI_API_MSG) :
                                 _to_phys(rank, MPIDI_API_MSG)), inject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME do_control_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_control_send(MPIDI_Send_control_t * control,
                                  char *send_buf,
                                  size_t msgsize,
                                  int rank, MPID_Comm * comm_ptr, MPID_Request * ackreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);

    control->origin_rank = comm_ptr->rank;
    control->send_buf = send_buf;
    control->msgsize = msgsize;
    control->comm_id = comm_ptr->context_id;
    control->endpoint_id = COMM_TO_EP(comm_ptr, comm_ptr->rank);
    control->ackreq = ackreq;
    MPIU_Assert(sizeof(*control) <= MPIDI_Global.max_buffered_send);
    FI_RC_RETRY(fi_inject(G_TXC_MSG(0),
                          control, sizeof(*control),
                          _comm_to_phys(comm_ptr, rank, MPIDI_API_MSG)), inject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif
