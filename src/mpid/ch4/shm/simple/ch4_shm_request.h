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
#ifndef SHM_REQUEST_H_INCLUDED
#define SHM_REQUEST_H_INCLUDED

#include "ch4_shm_impl.h"

static inline void MPIDI_CH4_SHM_am_request_init(MPID_Request *req)
{
    MPIU_Assert(0);
}

static inline void MPIDI_CH4_SHM_am_request_finalize(MPID_Request *req)
{
    MPIU_Assert(0);
}

#endif /* SHM_REQUEST_H_INCLUDED */
