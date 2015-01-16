/*----------------------------------------------------------------------
 *
 * flow_volume.c
 *     flow_volume() aggregate function working on flow_samples table
 *
 * Copyright (c) 2012, Endace Inc
 *
 *----------------------------------------------------------------------
 */

#include <inttypes.h>

#include "flow_volume.h"
#include "fmgr.h"
#include "executor/executor.h"  /* for GetAttributeByName() */

//#define DEBUG

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
Compile this module in the following way:

   gcc -g -I ../../../bin/end/include -I $(pg_config --includedir-server) -fpic -c MODULE.c
   gcc -shared  -o MODULE.so MODULE.o -L$ENDACE_INSTALL_ROOT/output/product-endace-x86_64/build/3rd_party/end/lib -ldag

To verify dependencies:

   ldd MODULE.so

This will also place the compiled module in the correct location.
*/


extern Datum flow_volume_sfunc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_sfunc);

extern Datum flow_volume_single_sfun(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_single_sfun);

extern Datum flow_volume_ffunc(PG_FUNCTION_ARGS); PG_FUNCTION_INFO_V1(flow_volume_ffunc);
extern Datum flow_volume_val(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_val);

extern Datum flow_volume_available(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_available);

extern Datum flow_volume_int8_sum(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_int8_sum);

extern Datum flow_volume_int8_rounddiv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_int8_rounddiv);

extern Datum flow_volume_version(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_version);

extern Datum flow_volume_flow_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_flow_hash);

extern Datum flow_volume_flow_hash_row(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_flow_hash_row);

extern Datum flow_volume_flow_blm_hashes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_flow_blm_hashes);

extern Datum flow_volume_flow_blm_hashes_row(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_flow_blm_hashes_row);

extern Datum flow_volume_is_ipv6(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_is_ipv6);

extern Datum flow_volume_sample_family(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(flow_volume_sample_family);

Datum
flow_volume_available(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL( true );
}

Datum
flow_volume_version(PG_FUNCTION_ARGS)
{
	PG_RETURN_INT32( FLOW_VOLUME_VERSION );
}
Datum
flow_volume_int8_rounddiv(PG_FUNCTION_ARGS)
{
	PG_RETURN_INT64( ( PG_GETARG_INT64(0) + (PG_GETARG_INT32(1)>>1) ) / PG_GETARG_INT32(1) );
}

/* subroutine to initialize state */
static FlowVolumeState makeFlowVolumeState(FunctionCallInfo fcinfo)
{
	FlowVolumeState	s;
	MemoryContext aggcontext;
	MemoryContext oldcontext;

	if (!AggCheckCallContext(fcinfo, &aggcontext))
	{
		/* cannot be called directly because of internal-type argument */
		elog(ERROR, "flow_volume_sfunc called in non-aggregate context");
	}

	/*
	 * Create state in aggregate context.  It'll stay there across subsequent
	 * calls.
	 */
	oldcontext = MemoryContextSwitchTo(aggcontext);
	s = (FlowVolumeState)palloc(sizeof(FlowVolumeStateData));
	SET_VARSIZE( s, sizeof(s));
	MemoryContextSwitchTo(oldcontext);

	return s;
}


#ifdef DEBUG
#define log_stat(prefix,s) do {\
	elog( NOTICE, #prefix " state: %i, %i, %" PRId64 ", %" PRId64 " // %i, %i, %" PRId64 ", %" PRId64 ", %i, %i", \
		s->a1, s->a2, s->ca1, s->ca2, \
		s->b1, s->b2, s->cb1, s->cb2, \
		s->a, s->b); \
} while(0);
#else
#define log_stat(prefix,s)
#endif

Datum
flow_volume_sfunc(PG_FUNCTION_ARGS)
{
	FlowVolumeState s;
	int32 a, b, start_time, last_time;
	int64 count;

	b = PG_GETARG_INT32(5);
	start_time = PG_GETARG_INT32(2);

	if (b <= start_time) PG_RETURN_NULL();

	count = PG_GETARG_INT64(1);

	//because the count includes all packets in [last_time,last_time+1)
	last_time = PG_GETARG_INT32(3) + 1;
	a = PG_GETARG_INT32(4);

#ifdef DEBUG
	elog( NOTICE, "INPUT: (%i, %i, %" PRId64 ")", start_time, last_time, count);
#endif

	if (PG_ARGISNULL(0)) {
		s = makeFlowVolumeState(fcinfo);
		// for state to be valid we need:
		// s->a1 < a <= s->a2
		// s->b1 < b <= s->b2
		// so we initialise as follows so that if any a1,a2,b1,b2
		// are not set, this will be apparent in flow_volume_val_
		s->a1 = s->b1 = start_time;
		s->a2 = s->b2 = last_time;
		s->ca1 = s->cb1 = 0;
		s->ca2 = s->cb2 = count;
		s->a = a;
		s->b = b;

		log_stat( ** flow_volume_info **, s);

	} else {
		s = (FlowVolumeState) PG_GETARG_POINTER(0);
	}
	if ( last_time > b && (s->b2 < b || last_time <= s->b2)){
		if (s->b2 != last_time || count > s->cb2){
			s->cb2 = count;
			s->b2 = last_time;
		}
	} else if ( last_time < b && (s->b1 >= b || last_time > s->b1 || count > s->cb1)){
		s->cb1 = count;
		s->b1 = last_time;
	} else if (last_time == b) {
		if (s->b2 != last_time || count > s->cb2){
			s->cb2 = count;
			s->b2 = last_time;
		}
	}
	
	if ( last_time > a && (s->a2 < a || last_time <= s->a2)){
		if (s->a2 != last_time || count > s->ca2){
			s->ca2 = count;
			s->a2 = last_time;
		}
	} else if ( last_time < a && (s->a1 >= a  || last_time > s->a1 || count > s->ca1)){
		s->ca1 = count;
		s->a1 = last_time;
	} else if (last_time == a){
		if (s->a2 != last_time || count > s->ca2){
			s->ca2 = count;
			s->a2 = last_time;
		}
	}
			
	log_stat( -, s);


	PG_RETURN_POINTER(s);
}

Datum
flow_volume_ffunc(PG_FUNCTION_ARGS)
{
	FlowVolumeState s;

	if ( PG_ARGISNULL(0) ) {
		PG_RETURN_INT64 ((int64)0);
	}
	else {
		s = (FlowVolumeState) PG_GETARG_POINTER(0);

		log_stat( =, s);

		PG_RETURN_INT64 ( flow_volume_val_( s) );
	}
}

Datum
flow_volume_val(PG_FUNCTION_ARGS)
{
	FlowVolumeStateData sd = {
		0,
		PG_GETARG_INT32(0),
		PG_GETARG_INT32(1),
		PG_GETARG_INT64(2),
		PG_GETARG_INT64(3),

		PG_GETARG_INT32(4),
		PG_GETARG_INT32(5),
		PG_GETARG_INT64(6),
		PG_GETARG_INT64(7),

		PG_GETARG_INT32(8),
		PG_GETARG_INT32(9)
	};

	PG_RETURN_INT64( flow_volume_val_( &sd) );
}

Datum
flow_volume_single_sfun(PG_FUNCTION_ARGS)
{
	int32 a, b, start, last, delta;
	int64 count, sum;

	start = PG_GETARG_INT32(2);
	last = PG_GETARG_INT32(3);

	count = PG_GETARG_INT64(1);

	if (PG_ARGISNULL(0)) sum=0;
	else sum=PG_GETARG_INT64(0);

	// if the range [a,b] doesn't overlap with the sample [start,last]:
	if ((PG_ARGISNULL(5) || PG_GETARG_INT32(5) > start)
		&& (PG_ARGISNULL(4) || PG_GETARG_INT32(4) <= last) ) {

		count = PG_GETARG_INT64(1);

		// not a subsecond sample:
		if (start != last) {

			a = PG_ARGISNULL(4) ? start : PG_GETARG_INT32(4);
			b = PG_ARGISNULL(5) ? last : PG_GETARG_INT32(5);
			delta = last+1-start;

#ifdef DEBUG
			elog( NOTICE, "* start: %i, last: %i, A: %i, B: %i, count: %" PRId64 ", sum: %" PRId64,
				start, last, a, b, count, sum );
#endif
			// [s ... ... l]
			//   [a ... b]
			if ( start < a && b < last+1 ) {
				count = (2*count*(b-a)+delta)/(2*delta);
			}
			// [s  ...    ...]
			//     [a ... ...]
			else if ( start < a ) {
				count = (2*count*(last+1-a)+delta)/(2*delta);
			}
			// [...     ...  l]
			// [... ... b]
			else if ( b < last+1 ) {
				count = (2*count*(b-start)+delta)/(2*delta);
			}
#ifdef DEBUG
			elog( NOTICE, "start: %i, last: %i, A: %i, B: %i, count: %" PRId64 ", sum: %" PRId64,
				start, last, a, b, count, sum );
#endif
		}
		sum += count;
	}
	PG_RETURN_INT64(sum);
}


Datum
flow_volume_int8_sum(PG_FUNCTION_ARGS)
{
	int64		sum;

	if (PG_ARGISNULL(0))
	{
		/* No non-null input seen so far... */
		if (PG_ARGISNULL(1))
			PG_RETURN_NULL();	/* still no non-null */
		sum = PG_GETARG_INT64(1);
	}
	else {
		sum = PG_GETARG_INT64(0);

		if (!PG_ARGISNULL(1))
			sum += PG_GETARG_INT64(1);
	}

#ifdef DEBUG
	elog( NOTICE, "SUM: %" PRId64, sum );
#endif

	PG_RETURN_INT64(sum);
}

#define ADDR_FAMILY(INET_V) (INET_V->inet_data.family)
#define ADDR_OCTETES(INET_V) ((uint32*)INET_V->inet_data.ipaddr)

// workaround for bug in DatumGetInetP(X) - it's fixed in PG 9.2:
#undef DatumGetInetP
#define DatumGetInetP(X) ((inet *) PG_DETOAST_DATUM(X))
#define GET_INET(name) ((inet *) PG_DETOAST_DATUM(GetAttributeByName(t,name,&isnull)))

Datum
flow_volume_flow_hash_row(PG_FUNCTION_ARGS)
{
	bool isnull;
	HeapTupleHeader  t = PG_GETARG_HEAPTUPLEHEADER(0);

	PG_RETURN_INT32(flow_volume_flow_hash_(
                                ADDR_FAMILY(GET_INET("ip_addr_1")),
                                ADDR_OCTETES(GET_INET("ip_addr_1")),
                                ADDR_OCTETES(GET_INET("ip_addr_2")),
                                (uint32)GetAttributeByName(t,"port_1",&isnull),
                                (uint32)GetAttributeByName(t,"port_2",&isnull),
                                (uint32)GetAttributeByName(t,"ip_protocol",&isnull)
	));
}

Datum
flow_volume_flow_hash(PG_FUNCTION_ARGS)
{

	PG_RETURN_INT32(flow_volume_flow_hash_(
				PG_ARGISNULL(0) ? 0 : ADDR_FAMILY(DatumGetInetP(PG_GETARG_DATUM(0))),
				PG_ARGISNULL(0) ? 0 : ADDR_OCTETES(DatumGetInetP(PG_GETARG_DATUM(0))),
				PG_ARGISNULL(1) ? 0 : ADDR_OCTETES(DatumGetInetP(PG_GETARG_DATUM(1))),
				PG_ARGISNULL(2) ? 0 : PG_GETARG_UINT32(2),
				PG_ARGISNULL(3) ? 0 : PG_GETARG_UINT32(3),
				PG_ARGISNULL(4) ? 0 : PG_GETARG_UINT32(4) ));
}

Datum
flow_volume_flow_blm_hashes_row(PG_FUNCTION_ARGS)
{
    bool isnull;
    HeapTupleHeader  t = PG_GETARG_HEAPTUPLEHEADER(0);

    PG_RETURN_INT64(flow_volume_flow_blm_hashes_(
                        ADDR_FAMILY(GET_INET("ip_addr_1")),
                        ADDR_OCTETES(GET_INET("ip_addr_1")),
                        ADDR_OCTETES(GET_INET("ip_addr_2")),
                        (uint32)GetAttributeByName(t,"port_1",&isnull),
                        (uint32)GetAttributeByName(t,"port_2",&isnull),
                        (uint32)GetAttributeByName(t,"ip_protocol",&isnull),
                        (uint16)GetAttributeByName(t,"vlan_id",&isnull)
    ));
}

Datum
flow_volume_flow_blm_hashes(PG_FUNCTION_ARGS)
{

    PG_RETURN_INT64(flow_volume_flow_blm_hashes_(
                        PG_ARGISNULL(0) ? 0 : ADDR_FAMILY(DatumGetInetP(PG_GETARG_DATUM(0))),
                        PG_ARGISNULL(0) ? 0 : ADDR_OCTETES(DatumGetInetP(PG_GETARG_DATUM(0))),
                        PG_ARGISNULL(1) ? 0 : ADDR_OCTETES(DatumGetInetP(PG_GETARG_DATUM(1))),
                        PG_ARGISNULL(2) ? 0 : PG_GETARG_UINT32(2),
                        PG_ARGISNULL(3) ? 0 : PG_GETARG_UINT32(3),
                        PG_ARGISNULL(4) ? 0 : PG_GETARG_UINT32(4),
                        PG_ARGISNULL(5) ? 0 : PG_GETARG_UINT16(5) ));
}


Datum
flow_volume_is_ipv6(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL( PG_GETARG_CHAR(0) & (char)0x10 ) ;
}

Datum
flow_volume_sample_family(PG_FUNCTION_ARGS)
{
	PG_RETURN_INT32( PG_GETARG_CHAR(0) & (char)0x10 ? 6 : 4 ) ;
}
