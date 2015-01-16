/*
 * LEGAL NOTICE: This source code:
 *
 * a) is a proprietary trade secret of Endace Technology Limited, a New
 *    Zealand company, and its suppliers and licensors ("Endace"). You
 *    must keep it strictly confidential, and you must not copy, modify,
 *    disclose or distribute any part of it to anyone else, without the
 *    prior written authorisation of Endace Technology Limited;
 *
 * b) may also be part of inventions that are protected by patents and
 *    patent applications; and
 *
 * c) is (C) copyright to Endace, 2011 to 2012. All rights reserved.
 *
 */

#include <sys/param.h>
#include <inttypes.h>
#include <stdlib.h>

#ifdef __unit_test__
#  define int32 int
#  define int64 int64_t
#  define uint32 unsigned int
#  define PGSQL_AF_INET 0
#  define PGSQL_AF_INET6 1
#else
#  include "postgres.h"
#  include "utils/inet.h"
#endif

#include "dagcrc.h"
#include "cpd_ffe_hash.h"

#define FLOW_VOLUME_VERSION 1
#define NON_IP_HASH_CONST 318843

/* Internal state */
typedef struct FlowVolumeStateData {
	int32 vl_len_;
	int a1, a2;
	int64 ca1, ca2;
	int b1, b2;
	int64 cb1, cb2;
	int a, b;
} FlowVolumeStateData;
typedef FlowVolumeStateData *FlowVolumeState;

inline int64
flow_volume_val_( FlowVolumeState s );


inline uint32 flow_volume_flow_hash_(
	unsigned char af, uint32 *sip, uint32 *dip, uint32 sport, uint32 dport, uint32 proto);

inline uint32 flow_volume_flow_hash_(
	unsigned char af, uint32 *sip, uint32 *dip, uint32 sport, uint32 dport, uint32 proto)
{   // everything except for ports come in network byte order

    if(!sip || !dip)
        return NON_IP_HASH_CONST;
    uint32 v = (htons(sport) << 16) ^ (htons(dport) << 16) ^ (proto << 24) ^ (sip[0] ^ dip[0]);
    if (af == PGSQL_AF_INET6) {
            v ^= (sip[1] ^ dip[1] ^ sip[2] ^ dip[2] ^ sip[3] ^ dip[3]);
    }
	return (dagcrc_aal5_crc(CRC_INIT, (const char *) &v, sizeof(v))) & 0xffffff;

}


static inline uint64 flow_volume_flow_blm_hashes_(
    unsigned char af, const uint32 *sip, const uint32 *dip,
    uint32 sport, uint32 dport, uint32 proto, uint16 vlan)
{
    // Everything except for ports and vlan come in network byte order.  The
    // hash wants the ports in network byte order and the vlan in host byte
    // order.
    //
    uint16_t vlan_real = (vlan == ((uint16_t)-1)) ? 0:vlan;
      
    /* return cpd_fee_hash_ipv4(0,0,0,0,0,0) constant whenever IP addresses are
     * not present.  Returns a hash when a VLAN is present without IP addresses.
     */
    if(!sip || !dip) 
    {
        if(!vlan_real)
            return 0;
        else
            return cpd_ffe_hash_ipv4(0,0,0,0,0,vlan_real);
    }

    sport = htons(sport);
    dport = htons(dport);

    if (af != PGSQL_AF_INET6)
        return cpd_ffe_hash_ipv4(
            proto, *sip, *dip, sport, dport, vlan_real);
    else
        return cpd_ffe_hash_ipv6(
            proto, sip, dip, sport, dport, vlan_real);
}


inline int64
flow_volume_val_( FlowVolumeState s )
{
	int64 volume=0;
	int64 ra, rb, res;
	if (!s) { return 0; }
	// for state to be valid we need:
	// s->a1 < a <= s->a2
	// s->b1 < b <= s->b2

	// Check if a2 is valid
	if (s->a2 < s->a || s->b1 >= s->b) { return 0; }

	// if a2 is valid, then either b1 is valid or sample is outside range    (1)

	// and if a1 was never set, then it retains the start_time of the flow
	// therfore we have the following cases:
	//                         [a............b]
	// a1 set            [s...............l]                  b2 not set
	// a1 set            [s........................l]         b2 set
	// a1 is start_time            [s.....l]                  b2 not set
	// a1 is start_time            [s..............l]         b2 set
	// a1 is start_time                                       b1 not set
	rb = 0;
	if (s->b2 >= s->b) {
		volume += s->cb2 - ((s->b2 - s->b)*(s->cb2 - s->cb1 )/(s->b2 - s->b1));
		rb = ( (s->cb2-s->cb1)*(s->b2 - s->b) )%(s->b2 -s->b1);
	} else if (s->b1 >= s->a){
		volume += s->cb1;
	}

	ra = 0;
	if (s->a1 < s->a){ // a1 <= a2 always (because if a1 not set, then start_time <= last_time)
		volume -= s->ca1 + ((s->a - s->a1)*(s->ca2 - s->ca1 )/(s->a2 - s->a1));
		ra = ( (s->ca2-s->ca1)*(s->a-s->a1) )%(s->a2 -s->a1);
	} 
	// "residual":
	res = 0;
	if (rb != 0 && ra != 0) {
		res  -= rb*(s->a2 -s->a1) + ra*(s->b2 -s->b1);
	} else if (rb == 0 && ra == 0){
		res = 0;
	} else {
		res = 0;
		if( ra != 0 && (ra > ((s->a2 -s->a1)>>1)) ){
			volume--;
		}
		if ( rb != 0 && (rb > ((s->b2 - s->b1)>>1)) ){
			volume--;
		}
	}
	if ( res != 0 && abs(res) > (((s->b2 -s->b1)*(s->a2 -s->a1))>>1) ) {
		if (res < 0) 
			volume--;
		else
			volume++;// should never happen though...
	}

	return volume;

}
