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
#ifndef NETMOD_OFI_PROGRESS_H_INCLUDED
#define NETMOD_OFI_PROGRESS_H_INCLUDED

#include "impl.h"
#include "events.h"
#include "am_events.h"

__attribute__((__always_inline__)) static inline
int MPIDI_CH4_NMI_OFI_Progress_generic(void *netmod_context,
                                       int   blocking,
                                       int   do_am,
                                       int   do_tagged)
{
    int                mpi_errno;
    struct fi_cq_tagged_entry  wc[MPIDI_CH4_NMI_OFI_NUM_CQ_ENTRIES];
    ssize_t            ret;

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);

    if(unlikely(MPIDI_CH4_NMI_OFI_Get_buffered(wc, 1)))
        mpi_errno = MPIDI_CH4_NMI_OFI_Handle_cq_entries(wc,1, 1);
    else if(likely(1)) {
        ret = fi_cq_read(MPIDI_Global.p2p_cq, (void *) wc, MPIDI_CH4_NMI_OFI_NUM_CQ_ENTRIES);

        if(likely(ret > 0))
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_cq_entries(wc,ret,0);
        else if(ret == -FI_EAGAIN)
            mpi_errno = MPI_SUCCESS;
        else
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_cq_error(ret);
    }

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);

    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_progress(void *netmod_context, int blocking)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PROGRESS);
    mpi_errno = MPIDI_CH4_NMI_OFI_Progress_generic(netmod_context,
                                                   blocking,
                                                   MPIDI_CH4_NMI_OFI_ENABLE_AM,
                                                   MPIDI_CH4_NMI_OFI_ENABLE_TAGGED);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_PROGRESS);
    return mpi_errno;
}

#endif /* NETMOD_OFI_PROGRESS_H_INCLUDED */