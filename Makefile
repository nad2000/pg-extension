#  Makefile
#
#  Filename:  $Source$
#  Revision:  $Revision$
#  Date:      $Date$
#  Author:    $Author$
#
#  (C) Copyright 2012 Endace Technology Ltd
#  All rights reserved.
#
#

include ${PROD_TREE_ROOT}/src/mk/common.mk

PKG_NAME=postgresql
PKG_PATH=customer/endace/src/lib/flow_volume/src
PKG_NEED_CLEAN=yes
PKG_CFG_IN_PLACE=yes

INSTALL_DIRECTIVES=  \
  "abs_file_copy ${PKG_BLD_BASE}/opt/endace/pgsql/share/extension/flow_volume--1.0.sql      opt/endace/pgsql/share/extension/flow_volume--1.0.sql" \
  "abs_file_copy ${PKG_BLD_BASE}/opt/endace/pgsql/share/extension/flow_volume.control      opt/endace/pgsql/share/extension/flow_volume.control" \
  "abs_file_copy ${PKG_BLD_BASE}/opt/endace/pgsql/share/extension/uninstall_flow_volume.sql opt/endace/pgsql/share/extension/uninstall_flow_volume.sql" \
  "abs_file_copy ${PKG_BLD_BASE}/opt/endace/pgsql/lib/flow_volume.so                        opt/endace/pgsql/lib/flow_volume.so"

include ${PROD_TREE_ROOT}/src/mk/third_party.mk
