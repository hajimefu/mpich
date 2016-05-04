/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef CH4_SHM_QUEUE_H
#define CH4_SHM_QUEUE_H

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_debug.h */
/* ------------------------------------------------------- */

/*#define MPIDI_SIMPLE_YIELD_IN_SKIP*/
#ifdef MPIDI_SIMPLE_YIELD_IN_SKIP
#define MPIDI_SIMPLE_SKIP MPL_sched_yield()
#warning "MPIDI_SIMPLE_SKIP is yield"
#else /* MPIDI_SIMPLE_YIELD_IN_SKIP */
#define MPIDI_SIMPLE_SKIP do {} while (0)
/*#warning "MPIDI_SIMPLE_SKIP is do ...while" */
#endif /* MPIDI_SIMPLE_YIELD_IN_SKIP */

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_queue.h */
/* ------------------------------------------------------- */

/* Assertion macros for nemesis queues.  We don't use the normal
 * assertion macros because we don't usually want to assert several
 * times per queue operation.  These assertions serve more as structured
 * comments that can easily transformed into being real assertions */
#define MPIDI_SIMPLE_Q_assert(a_) \
    do {/*nothing*/} while (0)

#undef FUNCNAME
#define FUNCNAME MPIDI_SIMPLE_cell_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_SIMPLE_cell_init(MPIDI_SIMPLE_cell_ptr_t cell,int rank)
{
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SIMPLE_CELL_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SIMPLE_CELL_INIT);

    MPIDI_SIMPLE_SET_REL_NULL(cell->next);
    memset((void *) &cell->pkt, 0, sizeof(MPIDI_SIMPLE_pkt_header_t));
    cell->my_rank = rank;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SIMPLE_CELL_INIT);
}

#if defined(MPIDI_SIMPLE_USE_LOCK_FREE_QUEUES)

#undef FUNCNAME
#define FUNCNAME MPIDI_SIMPLE_queue_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_SIMPLE_queue_init(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);

    MPIDI_SIMPLE_SET_REL_NULL(qhead->head);
    MPIDI_SIMPLE_SET_REL_NULL(qhead->my_head);
    MPIDI_SIMPLE_SET_REL_NULL(qhead->tail);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);
}

#define MPIDI_SIMPLE_USE_SHADOW_HEAD

static inline MPIDI_SIMPLE_cell_rel_ptr_t MPIDI_SIMPLE_SWAP_REL(MPIDI_SIMPLE_cell_rel_ptr_t *ptr,
                                                                                  MPIDI_SIMPLE_cell_rel_ptr_t val)
{
    MPIDI_SIMPLE_cell_rel_ptr_t ret;
    OPA_store_ptr(&ret.p, OPA_swap_ptr(&(ptr->p), OPA_load_ptr(&val.p)));
    return ret;
}

/* do a compare-and-swap with MPIDI_SIMPLE_RELNULL */
static inline MPIDI_SIMPLE_cell_rel_ptr_t MPIDI_SIMPLE_CAS_REL_NULL(MPIDI_SIMPLE_cell_rel_ptr_t *ptr,
                                                                                      MPIDI_SIMPLE_cell_rel_ptr_t oldv)
{
    MPIDI_SIMPLE_cell_rel_ptr_t ret;
    OPA_store_ptr(&ret.p, OPA_cas_ptr(&(ptr->p), OPA_load_ptr(&oldv.p), MPIDI_SIMPLE_REL_NULL));
    return ret;
}

static inline void MPIDI_SIMPLE_queue_enqueue(MPIDI_SIMPLE_queue_ptr_t qhead, MPIDI_SIMPLE_cell_ptr_t element)
{
    MPIDI_SIMPLE_cell_rel_ptr_t r_prev;
    MPIDI_SIMPLE_cell_rel_ptr_t r_element = MPIDI_SIMPLE_ABS_TO_REL(element);

    /* the _dequeue can break if this does not hold */
    MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(element->next));

    /* Orders payload and e->next=NULL w.r.t. the SWAP, updating head, and
     * updating prev->next.  We assert e->next==NULL above, but it may have been
     * done by us in the preceding _dequeue operation.
     *
     * The SWAP itself does not need to be ordered w.r.t. the payload because
     * the consumer does not directly inspect the tail.  But the subsequent
     * update to the head or e->next field does need to be ordered w.r.t. the
     * payload or the consumer may read incorrect data. */
    OPA_write_barrier();

    /* enqueue at tail */
    r_prev = MPIDI_SIMPLE_SWAP_REL(&(qhead->tail), r_element);

    if(MPIDI_SIMPLE_IS_REL_NULL(r_prev)) {
        /* queue was empty, element is the new head too */

        /* no write barrier needed, we believe atomic SWAP with a control
         * dependence (if) will enforce ordering between the SWAP and the head
         * assignment */
        qhead->head = r_element;
    } else {
        /* queue was not empty, swing old tail's next field to point to
         * our element */
        MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(MPIDI_SIMPLE_REL_TO_ABS(r_prev)->next));

        /* no write barrier needed, we believe atomic SWAP with a control
         * dependence (if/else) will enforce ordering between the SWAP and the
         * prev->next assignment */
        MPIDI_SIMPLE_REL_TO_ABS(r_prev)->next = r_element;
    }
}

/* This operation is only safe because this is a single-dequeuer queue impl.
   Assumes that MPIDI_SIMPLE_queue_empty was called immediately prior to fix up any
   shadow head issues. */
static inline MPIDI_SIMPLE_cell_ptr_t MPIDI_SIMPLE_queue_head(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(qhead->head));
    return MPIDI_SIMPLE_REL_TO_ABS(qhead->my_head);
}

static inline int MPIDI_SIMPLE_queue_empty(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    /* outside of this routine my_head and head should never both
     * contain a non-null value */
    MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(qhead->my_head) || MPIDI_SIMPLE_IS_REL_NULL(qhead->head));

    if(MPIDI_SIMPLE_IS_REL_NULL(qhead->my_head)) {
        /* the order of comparison between my_head and head does not
         * matter, no read barrier needed here */
        if(MPIDI_SIMPLE_IS_REL_NULL(qhead->head)) {
            /* both null, nothing in queue */
            return 1;
        } else {
            /* shadow head null and head has value, move the value to
             * our private shadow head and zero the real head */
            qhead->my_head = qhead->head;
            /* no barrier needed, my_head is entirely private to consumer */
            MPIDI_SIMPLE_SET_REL_NULL(qhead->head);
        }
    }

    /* the following assertions are present at the beginning of _dequeue:
     * MPIDI_SIMPLE_Q_assert(!MPIDI_SIMPLE_IS_REL_NULL(qhead->my_head));
     * MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(qhead->head));
     */
    return 0;
}


/* Gets the head */
static inline void MPIDI_SIMPLE_queue_dequeue(MPIDI_SIMPLE_queue_ptr_t qhead, MPIDI_SIMPLE_cell_ptr_t *e)
{
    MPIDI_SIMPLE_cell_ptr_t _e;
    MPIDI_SIMPLE_cell_rel_ptr_t _r_e;

    /* _empty always called first, moving head-->my_head */
    MPIDI_SIMPLE_Q_assert(!MPIDI_SIMPLE_IS_REL_NULL(qhead->my_head));
    MPIDI_SIMPLE_Q_assert(MPIDI_SIMPLE_IS_REL_NULL(qhead->head));

    _r_e = qhead->my_head;
    _e = MPIDI_SIMPLE_REL_TO_ABS(_r_e);

    /* no barrier needed, my_head is private to consumer, plus
     * head/my_head and _e->next are ordered by a data dependency */
    if(!MPIDI_SIMPLE_IS_REL_NULL(_e->next)) {
        qhead->my_head = _e->next;
    } else {
        /* we've reached the end (tail) of the queue */
        MPIDI_SIMPLE_cell_rel_ptr_t old_tail;

        MPIDI_SIMPLE_SET_REL_NULL(qhead->my_head);
        /* no barrier needed, the caller doesn't need any ordering w.r.t.
         * my_head or the tail */
        old_tail = MPIDI_SIMPLE_CAS_REL_NULL(&(qhead->tail), _r_e);

        if(!MPIDI_SIMPLE_REL_ARE_EQUAL(old_tail, _r_e)) {
            /* FIXME is a barrier needed here because of the control-only dependency? */
            while(MPIDI_SIMPLE_IS_REL_NULL(_e->next)) {
                MPIDI_SIMPLE_SKIP;
            }

            /* no read barrier needed between loads from the same location */
            qhead->my_head = _e->next;
        }
    }

    MPIDI_SIMPLE_SET_REL_NULL(_e->next);

    /* Conservative read barrier here to ensure loads from head are ordered
     * w.r.t. payload reads by the caller.  The McKenney "whymb" document's
     * Figure 11 indicates that we don't need a barrier, but we are currently
     * unconvinced of this.  Further work, ideally using more formal methods,
     * should justify removing this.  (note that this barrier won't cost us
     * anything on many platforms, esp. x86) */
    OPA_read_barrier();

    *e = _e;
}

#else /* !defined(MPIDI_SIMPLE_USE_LOCK_FREE_QUEUES) */

/* FIXME We shouldn't really be using the MPID_Thread_mutex_* code but the
 * MPIDU_Process_locks code is a total mess right now.  In the long term we need
   to resolve this, but in the short run it should be safe on most (all?)
   platforms to use these instead.  Usually they will both boil down to a
   pthread_mutex_t and and associated functions. */
#define MPIDI_SIMPLE_queue_mutex_create MPID_Thread_mutex_create
#define MPIDI_SIMPLE_queue_mutex_lock   MPID_Thread_mutex_lock
#define MPIDI_SIMPLE_queue_mutex_unlock MPID_Thread_mutex_unlock

/* must be called by exactly one process per queue */
#undef FUNCNAME
#define FUNCNAME MPIDI_SIMPLE_queue_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_SIMPLE_queue_init(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);

    MPIDI_SIMPLE_SET_REL_NULL(qhead->head);
    MPIDI_SIMPLE_SET_REL_NULL(qhead->my_head);
    MPIDI_SIMPLE_SET_REL_NULL(qhead->tail);
    MPIDI_SIMPLE_queue_mutex_create(&qhead->lock, NULL);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SIMPLE_QUEUE_INIT);
}

static inline void MPIDI_SIMPLE_queue_enqueue(MPIDI_SIMPLE_queue_ptr_t qhead, MPIDI_SIMPLE_cell_ptr_t element)
{
    MPIDI_SIMPLE_cell_rel_ptr_t r_prev;
    MPIDI_SIMPLE_cell_rel_ptr_t r_element = MPIDI_SIMPLE_ABS_TO_REL(element);

    MPIDI_SIMPLE_queue_mutex_lock(&qhead->lock);

    r_prev = qhead->tail;
    qhead->tail = r_element;

    if(MPIDI_SIMPLE_IS_REL_NULL(r_prev)) {
        qhead->head = r_element;
    } else {
        MPIDI_SIMPLE_REL_TO_ABS(r_prev)->next = r_element;
    }

    MPIDI_SIMPLE_queue_mutex_unlock(&qhead->lock);
}

/* This operation is only safe because this is a single-dequeuer queue impl. */
static inline MPIDI_SIMPLE_cell_ptr_t MPIDI_SIMPLE_queue_head(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    return MPIDI_SIMPLE_REL_TO_ABS(qhead->my_head);
}

/* Assumption: regular loads & stores are atomic.  This may not be univerally
   true, but it's not uncommon.  We often need to use these "lock-ful" queues on
   platforms where atomics are not yet implemented, so we can't rely on the
   atomics to provide atomic load/store operations for us. */
static inline int MPIDI_SIMPLE_queue_empty(MPIDI_SIMPLE_queue_ptr_t qhead)
{
    if(MPIDI_SIMPLE_IS_REL_NULL(qhead->my_head)) {
        if(MPIDI_SIMPLE_IS_REL_NULL(qhead->head)) {
            return 1;
        } else {
            qhead->my_head = qhead->head;
            MPIDI_SIMPLE_SET_REL_NULL(qhead->head); /* reset it for next time */
        }
    }

    return 0;
}

static inline void MPIDI_SIMPLE_queue_dequeue(MPIDI_SIMPLE_queue_ptr_t qhead, MPIDI_SIMPLE_cell_ptr_t *e)
{
    MPIDI_SIMPLE_cell_ptr_t _e;
    MPIDI_SIMPLE_cell_rel_ptr_t _r_e;

    _r_e = qhead->my_head;
    _e = MPIDI_SIMPLE_REL_TO_ABS(_r_e);


    if(MPIDI_SIMPLE_IS_REL_NULL(_e->next)) {
        /* a REL_NULL _e->next or writing qhead->tail both require locking */
        MPIDI_SIMPLE_queue_mutex_lock(&qhead->lock);
        qhead->my_head = _e->next;

        /* We have to check _e->next again because it may have changed between
         * the time we checked it without the lock and the time that we acquired
         * the lock. */
        if(MPIDI_SIMPLE_IS_REL_NULL(_e->next)) {
            MPIDI_SIMPLE_SET_REL_NULL(qhead->tail);
        }

        MPIDI_SIMPLE_queue_mutex_unlock(&qhead->lock);
    } else {    /* !MPIDI_SIMPLE_IS_REL_NULL(_e->next) */
        /* We don't need to lock because a non-null _e->next can't be changed by
         * anyone but us (the dequeuer) and we don't need to modify qhead->tail
         * because we aren't removing the last element from the queue. */
        qhead->my_head = _e->next;
    }

    MPIDI_SIMPLE_SET_REL_NULL(_e->next);
    *e = _e;
}

#endif /* !defined(MPIDI_SIMPLE_USE_LOCK_FREE_QUEUES) */

#endif /* ifndef CH4_SHM_QUEUE_H */
