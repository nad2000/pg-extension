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

// Minimalistic Unit Test framework:
#define __unit_test__
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) \
	do { \
		char *message = test(); tests_run++; \
		printf( "\n*** Running %s\n", #test ); \
		if (message) return fprintf(stdout,message); \
	} while (0)

#define mu_run_flow_vol_test(label,test_data,expected_output) \
	do { \
		char *message = flow_vol_test(test_data,expected_output); \
		tests_run++; \
		printf( "\n*** Running %s\n", #label ); \
		if (message) fprintf(stdout,message); \
	} while (0)

/* #define mu_run_flow_hash_test(label, ...) \ */
	do { \
		char *message = flow_hash_test(__VA_ARGS__); \
		tests_run++; \
		printf( "\n*** Running %s\n", #label ); \
		if (message) return message; \
	} while (0)

#define mu_run_flow_blm_hashes_test(label, ...) \
    do { \
        char *message = flow_blm_hashes_test(__VA_ARGS__); \
        tests_run++; \
        printf( "\n*** Running %s\n", #label ); \
        if (message) return message; \
    } while (0)

/*
Build:
gcc flow_volume_test.c -g -o test  -I$PROD_TREE_ROOT/customer/endace/src/include -L$PROD_OUTPUT_ROOT/product-endace-x86_64/build/3rd_party/end/lib -ldag -L$PROD_OUTPUT_ROOT/product-endace-x86_64/build/image/custlib/libblm_filter -lblm_filter -L$PROD_OUTPUT_ROOT/product-endace-x86_64/build/image/custlib/libeda_support -leda_support

Run:
LD_LIBRARY_PATH=$PROD_OUTPUT_ROOT/product-endace-x86_64/build/3rd_party/end/lib:$LD_LIBRARY_PATH ./test

*/


#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#ifndef uint16
    typedef uint16_t uint16;
#endif
#ifndef uint64
    typedef uint64_t uint64;
#endif

#include "flow_volume.h"

#define TEST_MSG_FRM_STR \
		"Test failed ... Expected: %" PRId64 "," \
		" received %" PRId64 "."


extern int tests_run;
int tests_run = 0;

static char msg[200];

static char * flow_vol_test(
	FlowVolumeState td,	// Test data
	int64_t e)		// Expected output
{

	int64_t s = flow_volume_val_( td);

	snprintf( msg, 200, TEST_MSG_FRM_STR, e, s);
	mu_assert( msg, s == e );
	return 0;
}

static char * flow_hash_test(
	unsigned char af, uint32 *sip, uint32 *dip, uint32 sport, uint32 dport, uint32 proto,	// Test data
	uint32 e)	// Expected output
{

	uint32 fh = flow_volume_flow_hash_(af, sip, dip, sport, dport, proto);

	snprintf( msg, 200, TEST_MSG_FRM_STR, e, fh);
	mu_assert( msg, fh == e );
	return 0;
}

static char * flow_blm_hashes_test(
    unsigned char af, uint32 *sip, uint32 *dip, uint32 sport, uint32 dport, uint32 proto, uint16 vlan, // Test data
    uint32 e1, uint32 e2)   // Expected outputs
{

    uint64 fh = flow_volume_flow_blm_hashes_(af, sip, dip, sport, dport, proto, vlan);
    uint64 e = (((uint64)e1) << 32) | e2;

    snprintf( msg, 200, TEST_MSG_FRM_STR, e, fh);
    mu_assert( msg, fh == e );
    return 0;
}

static char * all_tests() {

    mu_run_flow_blm_hashes_test(FBH001, PGSQL_AF_INET,
                                (uint32[]){0x6f84a8c0},(uint32[]){0x2b81a8c0},5632,26047,6,777,
                                0xc849ee48, 0x49d560f);
    mu_run_flow_blm_hashes_test(FBH002, PGSQL_AF_INET,
                                (uint32[]){0x6284a8c0},(uint32[]){0x0100ffef},60116,51477,17,888,
                                0x36ff4c75, 0xe984b088);
    mu_run_flow_blm_hashes_test(FBH003, PGSQL_AF_INET,
                                (uint32[]){0x7184a8c0},(uint32[]){0x470b02ef},51489,7602,17,999,
                                0xb6304dd0, 0xca894c81);
    mu_run_flow_blm_hashes_test(FBH004, PGSQL_AF_INET,
                                (uint32[]){0x6284a8c0},(uint32[]){0x0100ffef},60116,51477,17,1010,
                                0x776d45da, 0x793b8b6);
    mu_run_flow_blm_hashes_test(FBH005, PGSQL_AF_INET,
                                (uint32[]){0x6f84a8c0},(uint32[]){0x2b81a8c0},20480,63627,6,1212,
                                0xcdcef0e9, 0x94a2c7a7);

	mu_run_flow_hash_test(FH001, PGSQL_AF_INET,(uint32[]){0x6f84a8c0},(uint32[]){0x2b81a8c0},5632,26047,6, 3343788);
	mu_run_flow_hash_test(FH002, PGSQL_AF_INET,(uint32[]){0x6284a8c0},(uint32[]){0x0100ffef},60116,51477,17,158023);
	mu_run_flow_hash_test(FH003, PGSQL_AF_INET,(uint32[]){0x7184a8c0},(uint32[]){0x470b02ef},51489,7602,17,7614853);
	mu_run_flow_hash_test(FH004, PGSQL_AF_INET,(uint32[]){0x6284a8c0},(uint32[]){0x0100ffef},60116,51477,17,158023);
	mu_run_flow_hash_test(FH005, PGSQL_AF_INET,(uint32[]){0x6f84a8c0},(uint32[]){0x2b81a8c0},20480,63627,6,3475896);

	mu_run_flow_vol_test( test01, NULL, 0);
	mu_run_flow_vol_test( test02, &((FlowVolumeStateData){0, 0,1,0,168,1,10,168,420,-24,6}), 280);
	mu_run_flow_vol_test( test03, &((FlowVolumeStateData){0, 38,50,33994,39636,38,50,33994,39636,41,49}), 3761);
	mu_run_flow_vol_test( test04, &((FlowVolumeStateData){0, 0,10,10,20,10,10,20,20,20,30}), 0);
	mu_run_flow_vol_test( test05, &((FlowVolumeStateData){0, 11,21,12966,26616,31,38,30890,33994,16,36}), 14238);
	mu_run_flow_vol_test( test06, &((FlowVolumeStateData){0, 0,1,0,10,1,1,100,100,2,206}), 0);
	mu_run_flow_vol_test( test07, &((FlowVolumeStateData){0, 0,10, 0,10,10,20,10,20, 11,31}), 10);
	mu_run_flow_vol_test( test08, &((FlowVolumeStateData){0, 0, 10, 0, 10, 20, 30, 25, 30, 11, 41}), 20);
	mu_run_flow_vol_test( test09, &((FlowVolumeStateData){0, 0,1,0,168,1,10,168,420,-24,6}), 280);
	mu_run_flow_vol_test( test10, &((FlowVolumeStateData){0, 279,279,1817244,1817244,279,279,1817244,1817244,281,301}), 0);
	mu_run_flow_vol_test( test12, &((FlowVolumeStateData){0, 11,21,12966,26616,31,38,30890,33994,16,36}), 14238);
	mu_run_flow_vol_test( test13, &((FlowVolumeStateData){0, 91,101,80606,97084,101,111,97084,123490,102,112}), 26406);
	mu_run_flow_vol_test( test14, &((FlowVolumeStateData){0, 91,101,80606,97084,111,121,123490,386942,102,122}), 289858);
	mu_run_flow_vol_test( test15, &((FlowVolumeStateData){0, 91,101,80606,97084,279,279,1817244,1817244,102,301}), 1720160);
	mu_run_flow_vol_test( test16, &((FlowVolumeStateData){0, 71,81,49184,67154,200,210,971028,1106664,77,206}), 980677);
	mu_run_flow_vol_test( test17, &((FlowVolumeStateData){0, 0,1,0,10,1,1,100,100,2,206}), 0);
	mu_run_flow_vol_test( test18, &((FlowVolumeStateData){0, 38,50,33994,39636,38,50,33994,39636,45,49}), 1881);
	mu_run_flow_vol_test( test19, &((FlowVolumeStateData){0, 38,50,33994,39636,38,50,33994,39636,41,45}), 1881);
	mu_run_flow_vol_test( test20, &((FlowVolumeStateData){0, 0,1,0,994,200,210,971028,1106664,2,206}), 1037852);
	mu_run_flow_vol_test( test21, &((FlowVolumeStateData){0, 91,101,80606,97084,200,210,971028,1106664,96,206}), 951649);
	mu_run_flow_vol_test( test22, &((FlowVolumeStateData){0, 0,1,0,994,91,101,80606,97084,2,96}), 86203);
	mu_run_flow_vol_test( test23, &((FlowVolumeStateData){0, 0,5,0,7082644,105,161,290388186,449039290,2,155}), 427791373);
	mu_run_flow_vol_test( test24, &((FlowVolumeStateData){0, 0,7,0,39662760,1842,1891,18234950759,18373770475,6,1885}), 18325608544);
	mu_run_flow_vol_test( test25, &((FlowVolumeStateData){0, 0,11,0,31163610,1846,1903,18226451609,18710903826,6,1888}), 18560751961);

	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
