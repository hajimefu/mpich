/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef CH4_PROGRESS_H_INCLUDED
#define CH4_PROGRESS_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Progress_test(void)
{
    int mpi_errno, made_progress, i;
    MPIDI_STATE_DECL(MPID_STATE_CH4_PROGRESS_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_PROGRESS_TEST);


    if(OPA_load_int(&MPIDI_CH4_Global.active_progress_hooks)) {
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4I_THREAD_PROGRESS_MUTEX);
        for (i = 0; i < MAX_PROGRESS_HOOKS; i++) {
            if (MPIDI_CH4_Global.progress_hooks[i].active == TRUE) {
                MPIU_Assert(MPIDI_CH4_Global.progress_hooks[i].func_ptr != NULL);
                mpi_errno = MPIDI_CH4_Global.progress_hooks[i].func_ptr(&made_progress);
                if (mpi_errno) {
                    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_MUTEX);
                    MPIR_ERR_POP(mpi_errno);
                }
            }
        }
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_MUTEX);
    }
    /* todo: progress unexp_list */
    mpi_errno = MPIDI_NM_progress(MPIDI_CH4_Global.netmod_context[0], 0);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#ifdef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_SHM_progress(0);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_PROGRESS_TEST);
    return mpi_errno;
  fn_fail:
    goto fn_exit;;
}

__CH4_INLINE__ int MPIDI_Progress_poke(void)
{
    return MPIDI_Progress_test();
}

__CH4_INLINE__ void MPIDI_Progress_start(MPID_Progress_state * state)
{
    return;
}

__CH4_INLINE__ void MPIDI_Progress_end(MPID_Progress_state * state)
{
    return;
}

__CH4_INLINE__ int MPIDI_Progress_wait(MPID_Progress_state * state)
{
    return MPIDI_Progress_test();
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_register
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Progress_register(int (*progress_fn) (int *), int *id)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_PROGRESS_REGISTER);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_PROGRESS_REGISTER);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    for (i = 0; i < MAX_PROGRESS_HOOKS; i++) {
        if (MPIDI_CH4_Global.progress_hooks[i].func_ptr == NULL) {
            MPIDI_CH4_Global.progress_hooks[i].func_ptr = progress_fn;
            MPIDI_CH4_Global.progress_hooks[i].active = FALSE;
            break;
        }
    }

    if (i >= MAX_PROGRESS_HOOKS)
        goto fn_fail;

    OPA_incr_int(&MPIDI_CH4_Global.active_progress_hooks);

    (*id) = i;

fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_PROGRESS_REGISTER);
    return mpi_errno;
fn_fail:
    mpi_errno = MPIR_Err_create_code( MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                      "MPIDI_Progress_register", __LINE__,
                                      MPI_ERR_INTERN, "**progresshookstoomany", 0 );
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_deregister
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Progress_deregister(int id)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_PROGRESS_DEREGISTER);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_PROGRESS_DEREGISTER);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIU_Assert(id >= 0);
    MPIU_Assert(id < MAX_PROGRESS_HOOKS);
    MPIU_Assert(MPIDI_CH4_Global.progress_hooks[id].func_ptr != NULL);
    MPIDI_CH4_Global.progress_hooks[id].func_ptr = NULL;
    MPIDI_CH4_Global.progress_hooks[id].active = FALSE;

    OPA_decr_int(&MPIDI_CH4_Global.active_progress_hooks);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_PROGRESS_DEREGISTER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_activate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Progress_activate(int id)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_PROGRESS_ACTIVATE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_PROGRESS_ACTIVATE);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIU_Assert(id >= 0);
    MPIU_Assert(id < MAX_PROGRESS_HOOKS);
    MPIU_Assert(MPIDI_CH4_Global.progress_hooks[id].active == FALSE);
    MPIU_Assert(MPIDI_CH4_Global.progress_hooks[id].func_ptr != NULL);
    MPIDI_CH4_Global.progress_hooks[id].active = TRUE;

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_PROGRESS_ACTIVATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_deactivate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Progress_deactivate(int id)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_PROGRESS_DEACTIVATE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_PROGRESS_DEACTIVATE);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIU_Assert(id >= 0);
    MPIU_Assert(id < MAX_PROGRESS_HOOKS);
    MPIU_Assert(MPIDI_CH4_Global.progress_hooks[id].active == TRUE);
    MPIU_Assert(MPIDI_CH4_Global.progress_hooks[id].func_ptr != NULL);
    MPIDI_CH4_Global.progress_hooks[id].active = FALSE;

    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_PROGRESS_DEACTIVATE);
    return mpi_errno;
}

#endif /* CH4_PROGRESS_H_INCLUDED */
