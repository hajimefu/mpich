/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  *  (C) 2016 by Argonne National Laboratory.
 *   *      See COPYRIGHT in top-level directory.
 *    *
 *    */

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "mpidimpl.h"
#include "portals4.h"

/* Portals 4 Limits */
#define MPIDI_CH4_NMI_PTL_EVENT_COUNT          (1024*64)
#define MPIDI_CH4_NMI_PTL_UNEXPECTED_HDR_COUNT (1024*64)
#define MPIDI_CH4_NMI_PTL_LIST_SIZE            (1024*64)

/* Active Message Stuff */
#define MPIDI_CH4_NMI_PTL_NUM_OVERFLOW_BUFFERS (8)
#define MPIDI_CH4_NMI_PTL_OVERFLOW_BUFFER_SZ   (1024*1024)
#define MPIDI_CH4_NMI_PTL_MAX_AM_EAGER_SZ      (64*1024)
#define MPIDI_CH4_NMI_PTL_AM_TAG               (1 << 28)
#define MPIDI_CH4_NMI_PTL_MAX_AM_HANDLERS      (64)

typedef struct {
    union {
        uint64_t val;
        struct {
            uint32_t context_id;
            uint32_t src_rank;
        } data;
    };
} MPIDI_CH4_NMI_PTL_am_reply_token_t;

typedef struct {
    MPID_Node_id_t *node_map;
    MPID_Node_id_t  max_node_id;
    char *kvsname;
    char pname[MPI_MAX_PROCESSOR_NAME];
    void           *overflow_bufs[MPIDI_CH4_NMI_PTL_NUM_OVERFLOW_BUFFERS];
    ptl_handle_me_t overflow_me_handles[MPIDI_CH4_NMI_PTL_NUM_OVERFLOW_BUFFERS];
    MPIDI_CH4_NM_am_target_handler_fn am_handlers[MPIDI_CH4_NMI_PTL_MAX_AM_HANDLERS];
    MPIDI_CH4_NM_am_origin_handler_fn send_cmpl_handlers[MPIDI_CH4_NMI_PTL_MAX_AM_HANDLERS];
    ptl_handle_ni_t ni;
    ptl_ni_limits_t ni_limits;
    ptl_handle_eq_t eqs[2];
    ptl_pt_index_t  pt;
    ptl_handle_md_t md;
} MPIDI_CH4_NMI_PTL_global_t;

typedef struct {
    ptl_process_t  process;
    ptl_pt_index_t pt;
} MPIDI_CH4_NMI_PTL_addr_t;

extern MPIDI_CH4_NMI_PTL_global_t       MPIDI_CH4_NMI_PTL_global;
extern MPIDI_CH4_NMI_PTL_addr_t        *MPIDI_CH4_NMI_PTL_addr_table;

#define MPIDI_CH4_NMI_PTL_CONTEXT_ID_BITS 32
#define MPIDI_CH4_NMI_PTL_TAG_BITS 32

#define MPIDI_CH4_NMI_PTL_TAG_MASK      (0x00000000FFFFFFFFULL)
#define MPIDI_CH4_NMI_PTL_CTX_MASK      (0xFFFFFFFF00000000ULL)
#define MPIDI_CH4_NMI_PTL_TAG_SHIFT     (MPIDI_CH4_NMI_PTL_TAG_BITS)

static inline ptl_match_bits_t MPIDI_CH4_NMI_PTL_init_tag(MPIU_Context_id_t contextid, int tag)
{
    ptl_match_bits_t match_bits = 0;
    match_bits = contextid;
    match_bits <<= MPIDI_CH4_NMI_PTL_TAG_SHIFT;
    match_bits |= (MPIDI_CH4_NMI_PTL_TAG_MASK & tag);
    return match_bits;
}

#define MPIDI_CH4_NMI_PTL_MSG_SZ_MASK   (0x00000000FFFFFFFFULL)
#define MPIDI_CH4_NMI_PTL_SRC_RANK_MASK (0x00FFFFFF00000000ULL)

static inline ptl_hdr_data_t MPIDI_CH4_NMI_PTL_init_am_hdr(int handler_id, int rank, size_t msg_sz)
{
    ptl_hdr_data_t hdr = 0;
    hdr = (ptl_hdr_data_t)handler_id << 56;
    hdr |= (MPIDI_CH4_NMI_PTL_SRC_RANK_MASK & ((ptl_hdr_data_t)rank << 32));
    hdr |= (MPIDI_CH4_NMI_PTL_MSG_SZ_MASK & msg_sz);
    return hdr;
}

#endif
