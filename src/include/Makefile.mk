# -*- Mode: Makefile; -*-
#
# See COPYRIGHT in top-level directory.
#

prefixdir = ${prefix}

nobase_prefix_HEADERS = \
			include/common/zm_common.h \
			include/lock/zm_lock_types.h \
			include/lock/zm_ticket.h

noinst_HEADERS = \
		include/zm_config.h
