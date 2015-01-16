-- Calculation over individual flows:
WITH test_data AS ( SELECT * FROM (VALUES
(	1	,	994	),
(	11	,	12966	),
(	21	,	26616	),
(	31	,	30890	),
(	38	,	33994	),
(	50	,	39636	),
(	60	,	43046	),
(	71	,	49184	),
(	81	,	67154	),
(	91	,	80606	),
(	101	,	97084	),
(	111	,	123490	),
(	121	,	386942	),
(	130	,	445868	),
(	141	,	455414	),
(	151	,	468624	),
(	160	,	741270	),
(	170	,	774296	),
(	180	,	777494	),
(	190	,	792704	),
(	200	,	971028	),
(	210	,	1106664	),
(	220	,	1122168	),
(	230	,	1256380	),
(	240	,	1456752	),
(	250	,	1469716	),
(	261	,	1509458	),
(	267	,	1805092	),
(	279	,	1817244	)) AS test_data(last_time_sec, byte_count) ),
td2 AS ( SELECT * FROM (VALUES
(	1	,	10	),
(	1	,	100	)) AS test_data(last_time_sec, byte_count) ),
td3 AS ( SELECT * FROM (VALUES
(	1	,	168	),
(	4	,	420	)) AS td(last_time_sec, byte_count) ),
td4 AS ( SELECT * FROM (VALUES
(	0	,	988	)) AS td(last_time_sec, byte_count) ),
td5 AS ( SELECT * FROM (VALUES
(	1	,	168	),
(	10	,	420	)) AS td(last_time_sec, byte_count) ),
td6 AS ( SELECT * FROM (VALUES
(	0, 1864	),
(	0, 1864	)) AS td(last_time_sec, byte_count) ),
test AS (
-- -95;-25;45
  SELECT 1 AS a,205 AS b, 90 AS ev, flow_volume(byte_count,0, last_time_sec,1,205) AS v FROM td2
  UNION ALL
  SELECT -95 AS a, 5 AS b,
    flow_volume(byte_count,0, last_time_sec,-13,39) AS ev, 
    flow_volume(byte_count,0, last_time_sec,-13,0)
     +flow_volume(byte_count,0, last_time_sec,0,39) AS v FROM td6
  UNION ALL
  SELECT -95 AS a, 5 AS b,
    flow_volume(byte_count,0, last_time_sec,-95,5) AS ev, 
    flow_volume(byte_count,0, last_time_sec,-95,-25)+flow_volume(byte_count,0, last_time_sec,-25,5) AS v FROM td5
  UNION ALL
  SELECT -95 AS a, 45 AS b, 
    flow_volume(byte_count,0, last_time_sec,-95,-25)+flow_volume(byte_count,0, last_time_sec,-25,45) AS ev, 
    flow_volume(byte_count,0, last_time_sec,-95,45) AS v FROM td3
  UNION ALL
  SELECT -95 AS a, 45 AS b, 
    flow_volume(byte_count,0, last_time_sec,-95,-25)+flow_volume(byte_count,0, last_time_sec,-25,45) AS ev, 
    flow_volume(byte_count,0, last_time_sec,-95,45) AS v FROM td4
  UNION ALL
  SELECT 280 AS a,300 AS b, 0 AS ev, flow_volume(byte_count,0, last_time_sec,280,300) AS v FROM test_data
  UNION ALL
  SELECT 15 AS a,35 AS b, 14238 AS ev, flow_volume(byte_count,0, last_time_sec,15,35) AS v FROM test_data
  UNION ALL
  SELECT 101 AS a,111 AS b, 26406 AS ev, flow_volume(byte_count,0, last_time_sec,101,111) AS v FROM test_data
  UNION ALL
  SELECT 101 AS a,121 AS b, 289858 AS ev, flow_volume(byte_count,0, last_time_sec,101,121) AS v FROM test_data
  UNION ALL
  SELECT 101 AS a,300 AS b, 1720160 AS ev, flow_volume(byte_count,0, last_time_sec,101,300) AS v FROM test_data
  UNION ALL
  SELECT 76 AS a,205 AS b, 980677 AS ev, flow_volume(byte_count,0, last_time_sec,76,205) AS v FROM test_data
  UNION ALL
  SELECT 40 AS a, 48 AS b, 
    flow_volume(byte_count,0, last_time_sec,40,44)+flow_volume(byte_count,0, last_time_sec,44,48) AS ev, 
    flow_volume(byte_count,0, last_time_sec,40,48) AS v FROM test_data
  UNION ALL
  SELECT 1 AS a,205 AS b, 
    flow_volume(byte_count,0, last_time_sec,1,95)+flow_volume(byte_count,0, last_time_sec,95,205) AS ev, 
    flow_volume(byte_count,0, last_time_sec,1,205) AS v FROM test_data

)
SELECT test.*, ev=v AS "Match (should be t)" FROM test;

-- Test with overlapping flows:
WITH lt AS ( SELECT * FROM (VALUES
  ( 1, 1	,	994	),
  ( 1, 11	,	12966	),
  ( 1, 21	,	26616	),
  ( 1, 31	,	30890	),
  ( 1, 38	,	33994	),
  ( 1, 50	,	39636	),
  ( 2, 55	,	33	),
  ( 1, 60	,	43046	),
  ( 1, 71	,	49184	),
  ( 3, 55	,	44	),
  ( 3, 56	,	444	),
  ( 1, 81	,	67154	),
  ( 1, 91	,	80606	),
  ( 1, 101,	97084	)) AS test_data(
  flow_id, last_time_sec, byte_count) ),
test AS (
  SELECT flow_id, flow_volume(byte_count,0, last_time_sec,51,69) AS v 
  FROM lt
  WHERE last_time_sec BETWEEN 21 AND 99
  GROUP BY flow_id
),
expected AS ( SELECT * FROM (VALUES
  ( 1,8091  ),
  ( 2,2     ),
  ( 3,403   )) AS output
  ( flow_id, byte_sum) )
SELECT t.*, e.byte_sum=v AS "Match (should be t)" FROM test AS t JOIN expected AS e
  ON e.flow_id = t.flow_id;