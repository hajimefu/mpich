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
#ifndef NETMOD_DIRECT_H_INCLUDED
#define NETMOD_DIRECT_H_INCLUDED

#include "am.h"
#include "events.h"
#include "comm.h"
#include "proc.h"
#include "progress.h"
#include "unimpl.h"
#include "init.h"
#include "coll.h"

#ifdef USE_OFI_TAGGED
#include "probe.h"
#include "recv.h"
#include "send.h"
#include "win.h"
#include "rma.h"
#include "spawn.h"
#else
#include "am_probe.h"
#include "am_recv.h"
#include "am_send.h"
#include "am_win.h"
#include "am_rma.h"
#include "am_spawn.h"
#endif /* USE_OFI_TAGGED */

#endif /* NETMOD_DIRECT_H_INCLUDED */