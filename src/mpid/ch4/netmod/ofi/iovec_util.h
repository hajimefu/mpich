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
#ifndef NETMOD_OFI_IOVEC_UTIL_H_INCLUDED
#define NETMOD_OFI_IOVEC_UTIL_H_INCLUDED

#include <stdint.h>
#include <sys/uio.h>
#include <stdio.h>
#include <assert.h>
#include "types.h"

#define MPIDI_CH4_NMI_OFI_IOV_DONE     0
#define MPIDI_CH4_NMI_OFI_IOV_SUCCESS  0
#define MPIDI_CH4_NMI_OFI_IOV_EAGAIN   1
#define MPIDI_CH4_NMI_OFI_IOV_ERROR   -1

#define MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(var)                                             \
  ({                                                                    \
    iov_state->var## _base_addr = var;                                  \
    iov_state->var## _count     = var## _count;                         \
    iov_state->var## _iov       = var## _iov;                           \
    iov_state->var## _idx       = 0;                                    \
    iov_state->var## _addr = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_base + iov_state->var## _base_addr; \
    iov_state->var## _size = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_len; \
    while(iov_state->var## _size == 0) {                                \
      iov_state->var## _idx++;                                          \
      if(iov_state->var## _idx < iov_state->var## _count) {             \
        iov_state->var## _addr = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_base + iov_state->var## _base_addr; \
        iov_state->var## _size = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_len; \
      } else {                                                          \
        break;                                                          \
      }                                                                 \
    }                                                                   \
  })

#define MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(var)                                             \
  ({                                                                    \
    *var## _addr_next       = iov_state->var## _addr;                   \
    iov_state->var## _addr += buf_size;                                 \
    iov_state->var## _size -= buf_size;                                 \
    while(iov_state->var## _size == 0) {                                \
      iov_state->var## _idx++;                                          \
      if(iov_state->var## _idx < iov_state->var## _count) {             \
        iov_state->var## _addr = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_base + iov_state->var## _base_addr; \
        iov_state->var## _size = (uintptr_t)iov_state->var## _iov[iov_state->var## _idx].iov_len; \
      } else {                                                          \
        break;                                                          \
      }                                                                 \
    }                                                                   \
  })

#define MPIDI_CH4_NMI_OFI_INIT_IOV(var)                                                   \
  ({                                                                    \
    ((struct iovec*)(&var## _iov[0]))->iov_len  = last_len;                  \
    ((struct iovec*)(&var## _iov[0]))->iov_base = (void*)var## _last_addr;   \
    *var## _iovs_nout = 1;                                              \
  })

#define MPIDI_CH4_NMI_OFI_UPDATE_IOV(var)                                                 \
  ({                                                                    \
  var## _idx++;                                                         \
  (*var## _iovs_nout)++;                                                \
  ((struct iovec*)(&var## _iov[var## _idx]))->iov_base = (void *)var## _addr; \
  ((struct iovec*)(&var## _iov[var## _idx]))->iov_len  = len;                \
  })

#define MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE1(var1,var2)                                    \
  ({                                                                    \
    if(*var2## _iovs_nout>=var2## _max_iovs) return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;   \
    ((struct iovec*)(&var1## _iov[var1## _idx]))->iov_len += len;            \
    var2## _idx++;                                                      \
    (*var2## _iovs_nout)++;                                             \
    ((struct iovec*)(&var2## _iov[var2## _idx]))->iov_base = (void *)var2## _addr; \
    ((struct iovec*)(&var2## _iov[var2## _idx]))->iov_len  = len;            \
    MPIDI_CH4_NMI_OFI_Next_iovec_state(iov_state,&origin_addr, &target_addr, &len); \
})

#define MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE2(var1,var2,var3)                               \
  ({                                                                    \
    if(*var2## _iovs_nout>=var2## _max_iovs) return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;   \
    if(*var3## _iovs_nout>=var3## _max_iovs) return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;   \
    ((struct iovec*)(&var1## _iov[var1## _idx]))->iov_len += len;            \
    var2## _idx++;                                                      \
    (*var2## _iovs_nout)++;                                             \
    ((struct iovec*)(&var2## _iov[var2## _idx]))->iov_base = (void *)var2## _addr; \
    ((struct iovec*)(&var2## _iov[var2## _idx]))->iov_len  = len;            \
    var3## _idx++;                                                      \
    (*var3## _iovs_nout)++;                                             \
    ((struct iovec*)(&var3## _iov[var3## _idx]))->iov_base = (void *)var3## _addr; \
    ((struct iovec*)(&var3## _iov[var3## _idx]))->iov_len  = len;            \
    MPIDI_CH4_NMI_OFI_Next_iovec_state2(iov_state,&origin_addr, &result_addr, &target_addr,&len); \
})

static inline
int MPIDI_CH4_NMI_OFI_Init_iovec_state(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                       uintptr_t                        origin,
                                       uintptr_t                        target,
                                       size_t                           origin_count,
                                       size_t                           target_count,
                                       size_t                           buf_limit,
                                       struct iovec                    *origin_iov,
                                       struct iovec                    *target_iov)
{
    iov_state->buf_limit        = buf_limit;

    if((origin_count > 0) && (target_count > 0)) {
        MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(target);
        MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(origin);
    } else
        return MPIDI_CH4_NMI_OFI_IOV_ERROR;

    return MPIDI_CH4_NMI_OFI_IOV_SUCCESS;
}

static inline
int MPIDI_CH4_NMI_OFI_Init_iovec_state2(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                        uintptr_t                        origin,
                                        uintptr_t                        result,
                                        uintptr_t                        target,
                                        size_t                           origin_count,
                                        size_t                           result_count,
                                        size_t                           target_count,
                                        size_t                           buf_limit,
                                        struct iovec                    *origin_iov,
                                        struct iovec                    *result_iov,
                                        struct iovec                    *target_iov)

{
    iov_state->buf_limit        = buf_limit;

    if((origin_count > 0) && (target_count > 0) && (result_count > 0)) {
        MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(target);
        MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(origin);
        MPIDI_CH4_NMI_OFI_INIT_IOV_STATE(result);
    } else
        return MPIDI_CH4_NMI_OFI_IOV_ERROR;

    return MPIDI_CH4_NMI_OFI_IOV_SUCCESS;
}


static inline
int MPIDI_CH4_NMI_OFI_Peek_iovec_state(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                       uintptr_t                       *next_origin_addr,
                                       uintptr_t                       *next_target_addr,
                                       size_t                          *buf_len)
{
    if((iov_state->origin_size != 0) && (iov_state->target_size != 0)) {
        *next_origin_addr  = iov_state->origin_addr;
        *next_target_addr  = iov_state->target_addr;
        *buf_len           = MPL_MIN(iov_state->target_size,iov_state->origin_size);
        return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
    } else {
        if(((iov_state->origin_size != 0) || (iov_state->target_size != 0)))
            return MPIDI_CH4_NMI_OFI_IOV_ERROR;

        return MPIDI_CH4_NMI_OFI_IOV_DONE;
    }
}

static inline
int MPIDI_CH4_NMI_OFI_Peek_iovec_state2(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                        uintptr_t                       *next_origin_addr,
                                        uintptr_t                       *next_result_addr,
                                        uintptr_t                       *next_target_addr,
                                        size_t                          *buf_len)
{
    if((iov_state->origin_size != 0) && (iov_state->target_size != 0) && (iov_state->result_size != 0)) {
        *next_origin_addr  = iov_state->origin_addr;
        *next_result_addr  = iov_state->result_addr;
        *next_target_addr  = iov_state->target_addr;
        *buf_len           = MPL_MIN(MIN(iov_state->target_size,iov_state->origin_size),iov_state->result_size);
        return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
    } else {
        if(((iov_state->origin_size != 0) || (iov_state->target_size != 0) ||(iov_state->result_size != 0)))
            return MPIDI_CH4_NMI_OFI_IOV_ERROR;

        return MPIDI_CH4_NMI_OFI_IOV_DONE;
    }
}


static inline
int MPIDI_CH4_NMI_OFI_Next_iovec_state(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                       uintptr_t                       *origin_addr_next,
                                       uintptr_t                       *target_addr_next,
                                       size_t                          *buf_len)
{
    if((iov_state->origin_size != 0) && (iov_state->target_size != 0)) {
        uintptr_t buf_size = MIN(MIN(iov_state->target_size,iov_state->origin_size),iov_state->buf_limit);
        *buf_len           = buf_size;
        MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(target);
        MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(origin);
        return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
    } else {
        if(((iov_state->origin_size != 0) || (iov_state->target_size != 0)))
            return MPIDI_CH4_NMI_OFI_IOV_ERROR;

        return MPIDI_CH4_NMI_OFI_IOV_DONE;
    }
}

static inline
int MPIDI_CH4_NMI_OFI_Next_iovec_state2(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                        uintptr_t                       *origin_addr_next,
                                        uintptr_t                       *result_addr_next,
                                        uintptr_t                       *target_addr_next,
                                        size_t                          *buf_len)
{
    if((iov_state->origin_size != 0) && (iov_state->target_size != 0) && (iov_state->result_size != 0)) {
        uintptr_t buf_size = MIN(MIN(MIN(iov_state->target_size,iov_state->origin_size),iov_state->result_size),iov_state->buf_limit);
        *buf_len           = buf_size;
        MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(target);
        MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(origin);
        MPIDI_CH4_NMI_OFI_NEXT_IOV_STATE(result);
        return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
    } else {
        if(((iov_state->origin_size != 0) || (iov_state->target_size != 0) || (iov_state->result_size != 0)))
            return MPIDI_CH4_NMI_OFI_IOV_ERROR;

        return MPIDI_CH4_NMI_OFI_IOV_DONE;
    }
}

static inline
int MPIDI_CH4_NMI_OFI_Merge_iov_list(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                     struct iovec                    *origin_iov,
                                     size_t                           origin_max_iovs,
                                     struct fi_rma_iov               *target_iov,
                                     size_t                           target_max_iovs,
                                     size_t                          *origin_iovs_nout,
                                     size_t                          *target_iovs_nout)
{
    int rc;
    uintptr_t origin_addr,target_addr;
    uintptr_t origin_last_addr=0,target_last_addr=0;
    int       origin_idx=0, target_idx=0;
    size_t    len,last_len=0;
    CH4_COMPILE_TIME_ASSERT(offsetof(struct iovec,iov_base)==offsetof(struct fi_rma_iov,addr));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct iovec,iov_len)==offsetof(struct fi_rma_iov,len));

    rc = MPIDI_CH4_NMI_OFI_Next_iovec_state(iov_state,&origin_last_addr, &target_last_addr, &last_len);
    assert(rc!=MPIDI_CH4_NMI_OFI_IOV_ERROR);
    MPIDI_CH4_NMI_OFI_INIT_IOV(target);
    MPIDI_CH4_NMI_OFI_INIT_IOV(origin);

    while(rc > 0) {
        rc = MPIDI_CH4_NMI_OFI_Peek_iovec_state(iov_state,&origin_addr, &target_addr, &len);
        assert(rc!=MPIDI_CH4_NMI_OFI_IOV_ERROR);

        if(rc==MPIDI_CH4_NMI_OFI_IOV_DONE) return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;

        if(target_last_addr+last_len == target_addr) {
            MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE1(target,origin);
        } else if(origin_last_addr+last_len == origin_addr) {
            MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE1(origin,target);
        } else {
            if((*origin_iovs_nout>=origin_max_iovs)||(*target_iovs_nout>=target_max_iovs))
                return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;

            MPIDI_CH4_NMI_OFI_UPDATE_IOV(target);
            MPIDI_CH4_NMI_OFI_UPDATE_IOV(origin);
            MPIDI_CH4_NMI_OFI_Next_iovec_state(iov_state,&origin_addr, &target_addr, &len);
        }

        origin_last_addr=origin_addr;
        target_last_addr=target_addr;
        last_len  = len;
    }

    if(rc == MPIDI_CH4_NMI_OFI_IOV_DONE) return MPIDI_CH4_NMI_OFI_IOV_DONE;
    else return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
}

static inline
int MPIDI_CH4_NMI_OFI_Merge_iov_list2(MPIDI_CH4_NMI_OFI_Iovec_state_t *iov_state,
                                      struct iovec                    *origin_iov,
                                      size_t                           origin_max_iovs,
                                      struct iovec                    *result_iov,
                                      size_t                           result_max_iovs,
                                      struct fi_rma_iov               *target_iov,
                                      size_t                           target_max_iovs,
                                      size_t                          *origin_iovs_nout,
                                      size_t                          *result_iovs_nout,
                                      size_t                          *target_iovs_nout)
{
    int rc;
    uintptr_t origin_addr,result_addr,target_addr;
    uintptr_t origin_last_addr=0,result_last_addr=0,target_last_addr=0;
    int       origin_idx=0, result_idx=0, target_idx=0;
    size_t    len,last_len=0;

    rc = MPIDI_CH4_NMI_OFI_Next_iovec_state2(iov_state,&origin_last_addr,&result_last_addr,&target_last_addr, &last_len);
    assert(rc!=MPIDI_CH4_NMI_OFI_IOV_ERROR);
    MPIDI_CH4_NMI_OFI_INIT_IOV(target);
    MPIDI_CH4_NMI_OFI_INIT_IOV(origin);
    MPIDI_CH4_NMI_OFI_INIT_IOV(result);

    while(rc > 0) {
        rc = MPIDI_CH4_NMI_OFI_Peek_iovec_state2(iov_state,&origin_addr, &result_addr, &target_addr, &len);
        assert(rc!=MPIDI_CH4_NMI_OFI_IOV_ERROR);

        if(rc==MPIDI_CH4_NMI_OFI_IOV_DONE) return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;

        if(target_last_addr+last_len == target_addr) {
            MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE2(target,origin,result);
        } else if(origin_last_addr+last_len == origin_addr) {
            MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE2(origin,target,result);
        } else if(result_last_addr+last_len == result_addr) {
            MPIDI_CH4_NMI_OFI_UPDATE_IOV_STATE2(result,target,origin);
        } else {
            if((*origin_iovs_nout>=origin_max_iovs)||(*target_iovs_nout>=target_max_iovs)||(*result_iovs_nout>=result_max_iovs))
                return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;

            MPIDI_CH4_NMI_OFI_UPDATE_IOV(target);
            MPIDI_CH4_NMI_OFI_UPDATE_IOV(origin);
            MPIDI_CH4_NMI_OFI_UPDATE_IOV(result);
            MPIDI_CH4_NMI_OFI_Next_iovec_state2(iov_state,&origin_addr, &result_addr, &target_addr, &len);
        }

        origin_last_addr=origin_addr;
        result_last_addr=result_addr;
        target_last_addr=target_addr;
        last_len  = len;
    }

    if(rc == MPIDI_CH4_NMI_OFI_IOV_DONE) return MPIDI_CH4_NMI_OFI_IOV_DONE;
    else return MPIDI_CH4_NMI_OFI_IOV_EAGAIN;
}
#endif /* __mpid_iovec_util__h__ */