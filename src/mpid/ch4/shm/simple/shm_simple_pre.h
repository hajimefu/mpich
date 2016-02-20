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

#ifndef SHM_SIMPLE_PRE_H_INCLUDED
#define SHM_SIMPLE_PRE_H_INCLUDED

#include <mpi.h>

struct MPID_Request;

typedef struct {
    struct MPID_Request *next;
    struct MPID_Request *pending;
    int                  dest;
    int                  rank;
    int                  tag;
    int                  context_id;
    char                *user_buf;
    size_t               data_sz;
    int                  type;
    int                  user_count;
    MPI_Datatype         datatype;
    struct MPID_Segment *segment_ptr;
    size_t               segment_first;
    size_t               segment_size;
} MPIDI_CH4_SHMI_SIMPLE_Request_t;

typedef struct {
    int dummy;
} MPIDI_CH4_SHMI_SIMPLE_Comm_t;

#endif