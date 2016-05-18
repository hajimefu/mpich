/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef NETMOD_UCX_AM_H_INCLUDED
#define NETMOD_UCX_AM_H_INCLUDED

#include "ucx_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_reg_hdr_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_reg_hdr_handler(int handler_id,
                                               MPIDI_NM_am_origin_handler_fn origin_handler_fn,
                                               MPIDI_NM_am_target_handler_fn target_handler_fn)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_REG_HDR_HANDLER);

    MPIDI_UCX_global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_UCX_global.send_cmpl_handlers[handler_id] = origin_handler_fn;
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline void MPIDI_UCX_send_am_callback(void *request, ucs_status_t status)
{
    MPIDI_UCX_ucp_request_t* ucp_request = (MPIDI_UCX_ucp_request_t*) request;

    if(ucp_request->req){
        MPIR_Request *req = ucp_request->req;
        int handler_id = req->dev.ch4.ch4u.netmod_am.ucx.handler_id;

        if (req->dev.ch4.ch4u.netmod_am.ucx.pack_buffer) {
            MPL_free(req->dev.ch4.ch4u.netmod_am.ucx.pack_buffer);
        }
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](req);
        ucp_request->req = NULL;
    } else {
        ucp_request->req = (void *)TRUE;
    }

fn_exit:
    return;
fn_fail:
    goto fn_exit;
}

static inline void MPIDI_UCX_inject_am_callback(void *request, ucs_status_t status)
{
    MPIDI_UCX_ucp_request_t* ucp_request = (MPIDI_UCX_ucp_request_t*) request;

    if(ucp_request->req){
        MPL_free(ucp_request->req);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = (void *)TRUE;
    }

fn_exit:
    return;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_hdr(int           rank,
                                           MPIR_Comm    *comm,
                                           int           handler_id,
                                           const void   *am_hdr,
                                           size_t        am_hdr_sz,
                                           MPIR_Request *sreq,
                                           void         *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = 0;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_send_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am(int rank,
                                       MPIR_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
                                       MPI_Count count,
                                       MPI_Datatype datatype,
                                       MPIR_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPIR_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM);

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = data_sz;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIR_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);

        ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_UCX_send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    }

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_amv_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_amv_hdr(int rank,
                                        MPIR_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        MPIR_Request * sreq, void *netmod_context)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_amv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_amv(int rank,
                                        MPIR_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype,
                                        MPIR_Request * sreq, void *netmod_context)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_hdr_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_hdr_reply(MPIR_Context_id_t context_id,
                                                 int src_rank,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_reply(MPIR_Context_id_t context_id,
                                             int src_rank,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data, MPI_Count count,
                                             MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPIR_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_UCX_am_header_t ucx_hdr;
    MPIR_Comm *use_comm;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM);

    use_comm = MPIDI_CH4U_context_id_to_comm(context_id);
    ep = MPIDI_UCX_COMM_TO_EP(use_comm, src_rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = data_sz;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIR_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);

        ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_UCX_send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    }

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
        
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_NM_send_amv_reply(MPIR_Context_id_t context_id,
                                              int src_rank,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data, MPI_Count count,
                                              MPI_Datatype datatype, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_NM_am_hdr_max_sz(void)
{
    MPIR_Assert(0);
    return 0;
}

static inline int MPIDI_NM_inject_am_hdr(int rank,
                                             MPIR_Comm * comm,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = 0;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_inject_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        /* inject is done */
        MPL_free(send_buf);
    } else if (ucp_request->req) {
        MPL_free(send_buf);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = send_buf;
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_NM_inject_am_hdr_reply(MPIR_Context_id_t context_id,
                                                   int src_rank,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;
    MPIR_Comm *use_comm;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);

    use_comm = MPIDI_CH4U_context_id_to_comm(context_id);
    ep = MPIDI_UCX_COMM_TO_EP(use_comm, src_rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_inject_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        /* inject is done */
        MPL_free(send_buf);
    } else if (ucp_request->req) {
        MPL_free(send_buf);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = send_buf;
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_NM_am_inject_max_sz(void)
{
    MPIR_Assert(0);
    return 0;
}


#endif /* NETMOD_UCX_AM_H_INCLUDED */
