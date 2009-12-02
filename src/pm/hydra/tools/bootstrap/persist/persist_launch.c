/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2008 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "hydra_utils.h"
#include "bsci.h"
#include "bscu.h"
#include "persist_client.h"

static int *client_fds;

HYD_status HYDT_bscd_persist_launch_procs(
    char **args, struct HYD_node *node_list,
    HYD_status(*stdout_cb) (void *buf, int buflen),
    HYD_status(*stderr_cb) (void *buf, int buflen))
{
    struct HYD_node *node;
    int num_nodes, fd;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    num_nodes = 0;
    for (node = node_list; node; node = node->next)
        num_nodes++;

    for (node = node_list; node; node = node->next) {
        /* connect to hydserv on each node */
        status = HYDU_sock_connect(node->hostname, PERSIST_DEFAULT_PORT, &fd);
        HYDU_ERR_POP(status, "unable to connect to the main server\n");

        /* send information about the executable */
    }

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
