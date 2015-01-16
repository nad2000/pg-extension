include ${PROD_TREE_ROOT}/src/mk/common.mk

EXTVERSION   = $(shell grep default_version flow_volume.control | \
			   sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
NUMVERSION   = $(shell echo $(EXTVERSION) | sed -e 's/\([[:digit:]]*[.][[:digit:]]*\).*/\1/')

# MODULE_big instead of MODULES to link with external libs
MODULE_big   = flow_volume
OBJS         = flow_volume.o
EXTENSION    = flow_volume
DATA         = flow_volume--1.0.sql uninstall_flow_volume.sql
REGRESS      = flow_volume

override CPPFLAGS := $(CPPFLAGS) -I${PROD_BUILD_3RD_PARTY}/end/usr/include -std=gnu99

INCLUDE_PATH = ${PROD_TREE_ROOT}/customer/endace/src/include
ifneq ("$(wildcard $(INCLUDE_PATH))","")
    PG_CPPFLAGS+= -I${INCLUDE_PATH}
else
    PG_CPPFLAGS+= -I${PROD_CUSTOMER_ROOT}/${PROD_PRODUCT_LC}/src/include
endif
PG_CPPFLAGS+= -Wno-declaration-after-statement -std=gnu99 

# all options for external libs
SHLIB_LINK= \
	-L${PROD_BUILD_3RD_PARTY}/end/usr/lib -ldag			\
	-L${PROD_BUILD_ROOT}/image/custlib/libblm_filter -lblm_filter 	\
	-L${PROD_BUILD_ROOT}/image/custlib/libeda_support -leda_support

ifndef PG_CONFIG
    PG_CONFIG = ${PROD_BUILD_3RD_PARTY}/postgresql/bin/pg_config
    ifeq ("$(wildcard $(PG_CONFIG))","")
        PG_CONFIG = pg_config
    endif
endif

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
