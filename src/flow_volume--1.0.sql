-- ----------------------------------------------------------------------
--
--  flow_volumen.sql
--      flow_volume() aggregate function working on flow_samples table
-- 
--  Copyright (c) 2012, Endace Inc
-- 
-- ----------------------------------------------------------------------

/*

DROP AGGREGATE IF EXISTS flow_volume ( bigint, int, int, int, int);
DROP AGGREGATE IF EXISTS bigintsum(bigint, int, int, int, int);
DROP AGGREGATE IF EXISTS bigintsum(bigint);
DROP FUNCTION IF EXISTS flow_volume_version();
DROP FUNCTION IF EXISTS flow_volume_available();
DROP FUNCTION IF EXISTS flow_volume_ffunc( internal);
DROP FUNCTION IF EXISTS flow_volume_sfunc( internal, bigint, int, int, int, int );
DROP FUNCTION IF EXISTS flow_volume_val( bigint, bigint, int, int, int, int, int );
DROP FUNCTION IF EXISTS flow_volume_ffunc( bytea);
DROP FUNCTION IF EXISTS flow_volume_sfunc( bytea, bigint, int, int, int, int );
DROP FUNCTION IF EXISTS rounddiv( bigint, int );

*/
SET client_min_messages=ERROR;

-- DROP FUNCTION IF EXISTS flow_volume_version();
CREATE OR REPLACE FUNCTION flow_volume_version()
RETURNS int
AS '$libdir/flow_volume', 'flow_volume_version'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS rounddiv(bigint,int);
CREATE OR REPLACE FUNCTION rounddiv(bigint,int)
RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_int8_rounddiv'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_available();
CREATE OR REPLACE FUNCTION flow_volume_available()
RETURNS boolean
AS '$libdir/flow_volume', 'flow_volume_available'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_ffunc( internal);
CREATE OR REPLACE FUNCTION flow_volume_ffunc( internal )
RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_ffunc'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_sfunc( internal, bigint, int, int, int, int );
CREATE OR REPLACE FUNCTION flow_volume_sfunc( internal, bigint, int, int, int, int )
RETURNS internal
AS '$libdir/flow_volume', 'flow_volume_sfunc'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_val( bigint, bigint, int, int, int, int, int );
CREATE OR REPLACE FUNCTION flow_volume_val( 
  int, int, bigint, bigint, int, int, bigint, bigint, int, int)
RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_val'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_single_sfun( bigint, bigint, int, int, int, int );
CREATE OR REPLACE FUNCTION flow_volume_single_sfun( bigint, bigint, int, int, int, int )
RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_single_sfun'
LANGUAGE C IMMUTABLE;

-- DROP FUNCTION IF EXISTS flow_volume_int8_sum( bigint, bigint);
CREATE OR REPLACE FUNCTION flow_volume_int8_sum( bigint, bigint)
RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_int8_sum'
LANGUAGE C IMMUTABLE;

/*
$1 - byte/packet_count;
$2 - start_time_sec;
$3 - last_time_sec;
$4 - start TS;
$5 - end TS;
*/
DROP AGGREGATE IF EXISTS flow_volume ( bigint, int, int, int, int);
CREATE AGGREGATE flow_volume( bigint, int, int, int, int)
(
    SFUNC = flow_volume_sfunc,
    STYPE = internal,
    FINALFUNC = flow_volume_ffunc
);

/*
$1 - byte/packet_count;
$2 - start_time_sec;
$3 - last_time_sec;
$4 - start TS;
$5 - end TS;
*/
-- Single sample flow volumen:
DROP AGGREGATE IF EXISTS short_flow_volume(bigint,int,int,int,int);
CREATE AGGREGATE short_flow_volume(bigint,int,int,int,int)
(
  SFUNC = flow_volume_single_sfun,
  STYPE = bigint
);

DROP AGGREGATE IF EXISTS bigintsum(bigint);
CREATE AGGREGATE bigintsum(bigint) (
  SFUNC = flow_volume_int8_sum,
  STYPE = bigint
);


DROP FUNCTION IF EXISTS flow_hash(inet,inet,uint2,uint2,uint1);
DROP FUNCTION IF EXISTS flow_hash(anyelement);

/*
$1 - ip_addr_1;
$2 - ip_addr_2;
$3 - port_1;
$4 - port_2;
$5 - ip_protocol;
*/
CREATE OR REPLACE FUNCTION flow_hash(inet,inet,uint2,uint2,uint1) RETURNS int
AS '$libdir/flow_volume', 'flow_volume_flow_hash'
LANGUAGE C;

/*
$1 - any composite type with fields: ip_addr_1, ip_addr_2, port_1, port_2, ip_protocol;
*/
CREATE OR REPLACE FUNCTION flow_hash(anyelement) RETURNS int
AS '$libdir/flow_volume', 'flow_volume_flow_hash_row'
LANGUAGE C STRICT;


DROP FUNCTION IF EXISTS flow_blm_hashes(inet,inet,uint2,uint2,uint1,smallint);
DROP FUNCTION IF EXISTS flow_blm_hashes(anyelement);

/*
$1 - ip_addr_1;
$2 - ip_addr_2;
$3 - port_1;
$4 - port_2;
$5 - ip_protocol;
$6 - vlan_id
*/
CREATE OR REPLACE FUNCTION flow_blm_hashes(inet,inet,uint2,uint2,uint1,smallint) RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_flow_blm_hashes'
LANGUAGE C;

/*
$1 - any composite type with fields: ip_addr_1, ip_addr_2, port_1, port_2, ip_protocol, vlan_id;
*/
CREATE OR REPLACE FUNCTION flow_blm_hashes(anyelement) RETURNS bigint
AS '$libdir/flow_volume', 'flow_volume_flow_blm_hashes_row'
LANGUAGE C STRICT;


/*
$1 - sample flags
*/
CREATE OR REPLACE FUNCTION is_ipv6(uint1) RETURNS boolean
AS '$libdir/flow_volume', 'flow_volume_is_ipv6'
LANGUAGE C STRICT;

/*
$1 - sample family
*/
CREATE OR REPLACE FUNCTION sample_family(uint1) RETURNS int
AS '$libdir/flow_volume', 'flow_volume_sample_family'
LANGUAGE C STRICT;
