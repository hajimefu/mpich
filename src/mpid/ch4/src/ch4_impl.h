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
#ifndef MPIDCH4_IMPL_H_INCLUDED
#define MPIDCH4_IMPL_H_INCLUDED

#include "ch4_types.h"
#include <mpidch4.h>

/* Forward declarations used to resolve dependencies in inline builds */
/* Any MPIR_XXX functions used in CH4 need to have prototypes here    */
/* Todo:  Is this really the right place for the forward declarations */
int MPIR_Info_get_impl(MPID_Info *info_ptr, const char *key, int valuelen, char *value, int *flag);

/* Static inlines */
static inline int MPIDI_CH4I_get_source(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4U_SOURCE_MASK) >> MPIDI_CH4U_TAG_SHIFT));
}

static inline int MPIDI_CH4I_get_tag(uint64_t match_bits)
{
    return ((int) (match_bits & MPIDI_CH4U_TAG_MASK));
}

static inline int MPIDI_CH4I_get_context(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4U_CONTEXT_MASK) >>
                   (MPIDI_CH4U_TAG_SHIFT + MPIDI_CH4U_SOURCE_SHIFT)));
}

static inline int MPIDI_CH4I_get_context_index(uint64_t context_id)
{
    int raw_prefix, idx, bitpos, gen_id;
    raw_prefix = MPID_CONTEXT_READ_FIELD(PREFIX, context_id);
    idx = raw_prefix / MPIR_CONTEXT_INT_BITS;
    bitpos = raw_prefix % MPIR_CONTEXT_INT_BITS;
    gen_id = (idx * MPIR_CONTEXT_INT_BITS) + (31 - bitpos);
    return gen_id;
}

static inline MPID_Request *MPIDI_CH4I_create_req()
{
    //MPID_Request *req = MPIDI_CH4I_request_alloc_and_init(2);
    MPID_Request *req = MPIDI_netmod_request_create();
    MPIU_Object_set_ref(req, 2);
    MPIU_CH4U_REQUEST(req, status) = 0;
    return req;
}

static inline MPID_Request *MPIDI_CH4I_create_win_req()
{
    //MPID_Request *req = MPIDI_CH4I_request_alloc_and_init(1);
    MPID_Request *req = MPIDI_netmod_request_create();
    MPIU_Object_set_ref(req, 1);
    MPIU_CH4U_REQUEST(req, status) = 0;
    return req;
}

static inline void MPIDI_CH4I_complete_req(MPID_Request *req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    if (count == 0)
        MPIDI_Request_release(req);
}

#define dtype_add_ref_if_not_builtin(datatype_)                         \
    do {								\
	if ((datatype_) != MPI_DATATYPE_NULL &&				\
	    HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)	\
	{								\
	    MPID_Datatype *dtp_ = NULL;					\
	    MPID_Datatype_get_ptr((datatype_), dtp_);			\
	    MPID_Datatype_add_ref(dtp_);				\
	}								\
    } while (0)

#define dtype_release_if_not_builtin(datatype_)				\
    do {								\
	if ((datatype_) != MPI_DATATYPE_NULL &&				\
	    HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)	\
	{								\
	    MPID_Datatype *dtp_ = NULL;					\
	    MPID_Datatype_get_ptr((datatype_), dtp_);			\
	    MPID_Datatype_release(dtp_);				\
	}								\
    } while (0)

#define MPIDI_Datatype_get_info(_count, _datatype,                      \
                                _dt_contig_out, _data_sz_out,           \
                                _dt_ptr, _dt_true_lb)                   \
    ({									\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_ptr)        = NULL;					\
	    (_dt_contig_out) = TRUE;					\
	    (_dt_true_lb)    = 0;					\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	}								\
	else								\
	{								\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	    (_dt_true_lb)    = (_dt_ptr)->true_lb;			\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
	}								\
    })

#define MPIDI_Datatype_get_size_dt_ptr(_count, _datatype,               \
                                       _data_sz_out, _dt_ptr)           \
    ({									\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_ptr)        = NULL;					\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	}								\
	else								\
	{								\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
	}								\
    })

#define MPIDI_Datatype_check_contig(_datatype,_dt_contig_out)	\
    ({								\
      if (IS_BUILTIN(_datatype))				\
      {								\
       (_dt_contig_out) = TRUE;					\
       }							\
      else							\
      {								\
       MPID_Datatype *_dt_ptr;					\
       MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
       (_dt_contig_out) = (_dt_ptr)->is_contig;			\
       }							\
      })

#define MPIDI_Datatype_check_contig_size(_datatype,_count,              \
                                         _dt_contig_out,                \
                                         _data_sz_out)                  \
    ({									\
      if (IS_BUILTIN(_datatype))					\
      {                                                                 \
	  (_dt_contig_out) = TRUE;					\
	  (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *			\
	      MPID_Datatype_get_basic_size(_datatype);			\
      }                                                                 \
      else								\
      {                                                                 \
	  MPID_Datatype *_dt_ptr;					\
	  MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	  (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	  (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
      }                                                                 \
    })

#define MPIDI_Datatype_check_size(_datatype,_count,_data_sz_out)        \
    ({                                                                  \
        if (IS_BUILTIN(_datatype))                                      \
        {                                                               \
            (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *               \
                MPID_Datatype_get_basic_size(_datatype);                \
        }                                                               \
        else                                                            \
        {                                                               \
            MPID_Datatype *_dt_ptr;                                     \
            MPID_Datatype_get_ptr((_datatype), (_dt_ptr));              \
            (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
        }                                                               \
    })

#define MPIDI_Datatype_check_contig_size_lb(_datatype,_count,           \
                                            _dt_contig_out,             \
                                            _data_sz_out,               \
                                            _dt_true_lb)                \
    ({									\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_contig_out) = TRUE;					\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	    (_dt_true_lb)    = 0;					\
	}								\
	else								\
	{								\
	    MPID_Datatype *_dt_ptr;					\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
	    (_dt_true_lb)    = (_dt_ptr)->true_lb;			\
	}								\
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

#define IS_BUILTIN(_datatype)				\
    (HANDLE_GET_KIND(_datatype) == HANDLE_KIND_BUILTIN)

#ifndef container_of
#define container_of(ptr, type, field)			\
    ((type *) ((char *)ptr - offsetof(type, field)))
#endif

static inline uint64_t MPIDI_CH4I_init_send_tag(MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_CH4U_SOURCE_SHIFT);
    match_bits |= source;
    match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
    match_bits |= (MPIDI_CH4U_TAG_MASK & tag);
    return match_bits;
}

static inline uint64_t MPIDI_CH4I_init_recvtag(uint64_t * mask_bits,
                                              MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits = 0;
    *mask_bits = MPIDI_CH4U_PROTOCOL_MASK;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_CH4U_SOURCE_SHIFT);

    if (MPI_ANY_SOURCE == source) {
        match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
        *mask_bits |= MPIDI_CH4U_SOURCE_MASK;
    }
    else {
        match_bits |= source;
        match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
    }

    if (MPI_ANY_TAG == tag)
        *mask_bits |= MPIDI_CH4U_TAG_MASK;
    else
        match_bits |= (MPIDI_CH4U_TAG_MASK & tag);

    return match_bits;
}

static inline int MPIDI_CH4I_valid_group_rank(int         lpid,
                                              MPID_Group *grp)
{
    int size = grp->size;
    int z;

    for(z = 0; z < size &&lpid != grp->lrank_to_lpid[z].lpid; ++z) {}
    return (z < size);
}

#define MPIDI_CH4I_PROGRESS()                                   \
    ({								\
	mpi_errno = MPIDI_Progress_test();			\
	if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
    })

#define MPIDI_CH4I_PROGRESS_WHILE(cond)         \
    ({						\
	while (cond)				\
	    MPIDI_CH4I_PROGRESS();              \
    })

#ifdef HAVE_ERROR_CHECKING
#define MPIDI_CH4I_EPOCH_CHECK1()                                       \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIU_CH4U_WIN(win, sync).target_epoch_type && \
           MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4I_EPOTYPE_REFENCE) \
        {                                                               \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_FENCE; \
            MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_FENCE; \
        }                                                               \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4I_EPOTYPE_NONE || \
           MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4I_EPOTYPE_POST) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                goto fn_fail, "**rmasync");             \
        MPID_END_ERROR_CHECKS;                                          \
    })

#define MPIDI_CH4I_EPOCH_CHECK2()                                  \
    ({                                                             \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_NONE && \
           MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_REFENCE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                goto fn_fail, "**rmasync");             \
    })

#define MPIDI_CH4I_EPOCH_START_CHECK()                                  \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if (MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4I_EPOTYPE_START && \
            !MPIDI_CH4I_valid_group_rank(target_rank,                   \
                                         MPIU_CH4U_WIN(win, sync).sc.group)) \
            MPIR_ERR_SETANDSTMT(mpi_errno,                              \
                                MPI_ERR_RMA_SYNC,                       \
                                goto fn_fail,                           \
                                "**rmasync");                           \
        MPID_END_ERROR_CHECKS;                                          \
    })

#define MPIDI_CH4I_EPOCH_FENCE_CHECK()                                  \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIU_CH4U_WIN(win, sync).target_epoch_type) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                goto fn_fail, "**rmasync");             \
        if (!(massert & MPI_MODE_NOPRECEDE) &&                          \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_FENCE && \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_REFENCE && \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_NONE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                goto fn_fail, "**rmasync");             \
        MPID_END_ERROR_CHECKS;                                          \
    })

#define MPIDI_CH4I_EPOCH_POST_CHECK()                              \
    ({                                                             \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).target_epoch_type != MPIDI_CH4I_EPOTYPE_NONE && \
           MPIU_CH4U_WIN(win, sync).target_epoch_type != MPIDI_CH4I_EPOTYPE_REFENCE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                goto fn_fail, "**rmasync");             \
        MPID_END_ERROR_CHECKS;                                          \
    })

#define MPIDI_CH4I_EPOCH_LOCK_CHECK()                                            \
({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                      \
    if((MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_LOCK) && \
       (MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_LOCK_ALL)) \
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                \
                            goto fn_fail, "**rmasync");                 \
    MPID_END_ERROR_CHECKS;                                              \
})

#define MPIDI_CH4I_EPOCH_FREE_CHECK()                                   \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIU_CH4U_WIN(win, sync).target_epoch_type || \
           (MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_NONE && \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4I_EPOTYPE_REFENCE)) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC, return mpi_errno, "**rmasync"); \
        MPID_END_ERROR_CHECKS;                                          \
})

#define MPIDI_CH4I_EPOCH_ORIGIN_CHECK(epoch_type)                       \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type != epoch_type)    \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                return mpi_errno, "**rmasync");         \
        MPID_END_ERROR_CHECKS;                                          \
    })

#define MPIDI_CH4I_EPOCH_TARGET_CHECK(epoch_type)                       \
    ({                                                                  \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIU_CH4U_WIN(win, sync).target_epoch_type != epoch_type)    \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                return mpi_errno, "**rmasync");         \
        MPID_END_ERROR_CHECKS;                                          \
    })

#else /* HAVE_ERROR_CHECKING */
#define MPIDI_CH4I_EPOCH_CHECK1()       if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_CHECK2()       if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_START_CHECK()  if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_FENCE_CHECK()  if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_POST_CHECK()   if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_LOCK_CHECK()   if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_FREE_CHECK()   if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_ORIGIN_CHECK(epoch_type) if(0) goto fn_fail;
#define MPIDI_CH4I_EPOCH_TARGET_CHECK(epoch_type) if(0) goto fn_fail;
#endif /* HAVE_ERROR_CHECKING */

#define MPIDI_CH4I_EPOCH_FENCE_EVENT()                             \
    ({                                                             \
        if(massert & MPI_MODE_NOSUCCEED)                           \
        {                                                               \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_NONE; \
            MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_NONE; \
        }                                                               \
        else                                                            \
        {                                                               \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_REFENCE; \
            MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_REFENCE; \
        }                                                               \
    })

#define MPIDI_CH4I_EPOCH_TARGET_EVENT()                                 \
    ({                                                                  \
        if(MPIU_CH4U_WIN(win, sync).target_epoch_type == MPIDI_CH4I_EPOTYPE_REFENCE) \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_REFENCE; \
        else                                                            \
            MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_NONE; \
    })

#define MPIDI_CH4I_EPOCH_ORIGIN_EVENT()                                 \
    ({                                                                  \
        if(MPIU_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4I_EPOTYPE_REFENCE) \
            MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_REFENCE; \
        else                                                            \
            MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_NONE; \
})

#endif /* MPIDCH4_IMPL_H_INCLUDED */
