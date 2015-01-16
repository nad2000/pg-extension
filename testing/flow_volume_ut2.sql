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
  ON e.flow_id = t.flow_id