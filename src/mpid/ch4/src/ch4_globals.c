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

/* All global ADI data structures need to go in this file */
/* reference them with externs from other files           */

#include <mpidimpl.h>
#include "ch4_impl.h"

MPIDI_CH4_Global_t MPIDI_CH4_Global;

#ifndef MPID_REQUEST_PREALLOC
#define MPID_REQUEST_PREALLOC 16
#endif

MPID_Request MPIDI_Request_direct[MPID_REQUEST_PREALLOC] = { {0} };
MPIU_Object_alloc_t MPIDI_Request_mem = {
    0, 0, 0, 0,
    MPID_REQUEST,
    sizeof(MPID_Request),
    MPIDI_Request_direct,
    MPID_REQUEST_PREALLOC
};

MPIDI_netmod_funcs_t *MPIDI_netmod_func;
MPIDI_netmod_native_funcs_t *MPIDI_netmod_native_func;

#ifdef MPIDI_BUILD_CH4_SHM
MPIDI_shm_funcs_t        *MPIDI_shm_func;
MPIDI_shm_native_funcs_t *MPIDI_shm_native_func;
#endif

#ifdef MPID_DEVICE_DEFINES_THREAD_CS
pthread_mutex_t MPIDI_Mutex_lock[MPIDI_NUM_LOCKS];
#endif

/* The MPID_Abort ADI is strangely defined by the upper layers */
/* We should fix the upper layer to define MPID_Abort like any */
/* Other ADI */
#ifdef MPID_Abort
#define MPID_TMP MPID_Abort
#undef MPID_Abort
int MPID_Abort(MPID_Comm * comm, int mpi_errno, int exit_code, const char *error_msg)
{
    return MPIDI_Abort(comm, mpi_errno, exit_code, error_msg);
}

#define MPID_Abort MPID_TMP
#endif

/* Another weird ADI that doesn't follow convention */
static void init_comm()__attribute__((constructor));
static void init_comm()
{
  MPID_Comm_fns             = &MPIDI_CH4_Global.MPID_Comm_fns_store;
  MPID_Comm_fns->split_type =  MPIDI_Comm_split_type;
}



/* These aliases are used because we call MPID_Request_get_ptr   */
/* in some internal routines MPID_Request_get_ptr is a macro     */
/* that #defines to MPID_ instead of MPIDI_.  We have the option */
/* to copy the macros for a different namespace, but aliasing    */
/* is probably the safer option to avoid missing symbols         */
#ifdef MPID_Request_mem
#undef MPID_Request_mem
#endif

#ifdef MPID_Request_direct
#undef MPID_Request_direct
#endif
extern MPIU_Object_alloc_t MPID_Request_mem __attribute__((alias("MPIDI_Request_mem")));
extern MPID_Request        MPID_Request_direct[MPID_REQUEST_PREALLOC] __attribute__((alias("MPIDI_Request_direct")));
