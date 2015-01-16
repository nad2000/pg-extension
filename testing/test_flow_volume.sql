WITH
rng AS (
    SELECT
      -- mid-120 as a1, mid+120 as b1,
      -- mid-12 as a1, mid+12 as b1,
      mid-12000 as a1, mid+12000 as b1,
      -- mid-5 as a1, mid+5 as b1,
      mid AS mid,
      first_ts, 
      last_ts
    FROM (
    SELECT ((MIN(last_time_sec)::bigint+MAX(last_time_sec))/2)::int AS mid,
      MIN(last_time_sec) AS first_ts, MAX(last_time_sec) AS last_ts
    FROM flow_samples ) AS m ),

tbc AS (
  SELECT application_id, SUM(byte_count) AS byte_count FROM (
    SELECT last(application_id order by last_time_sec) AS application_id, MAX(byte_count) AS byte_count
  FROM flow_samples GROUP BY flow_id) AS fd 
  GROUP BY application_id ),

total AS (SELECT SUM(byte_count) AS total FROM tbc),

tbc2 AS (
SELECT application_id, 
   SUM(byte_count) AS byte_count
  FROM (
SELECT last(application_id order by last_time_sec) AS application_id,
flow_volume( byte_count, start_time_sec, last_time_sec, 0, (SELECT last_ts FROM rng)+1000 ) AS byte_count
FROM flow_samples GROUP BY flow_id) AS fd 
GROUP BY application_id ),

total2 AS (SELECT SUM(byte_count) AS total FROM tbc2)

SELECT 
  application_guidname AS "App", tbc.byte_count, tbc2.byte_count,
  tbc.byte_count-tbc2.byte_count AS "Diff",
  ( tbc.byte_count/(SELECT total FROM total) ) AS pc,
  ( tbc2.byte_count/(SELECT total FROM total2) ) AS pc2,
  (SELECT total FROM total) AS "Total", (SELECT total FROM total2) AS "Total 2"
FROM tbc 
  FULL JOIN tbc2 ON tbc2.application_id = tbc.application_id
  LEFT JOIN applications AS a ON a.application_id = COALESCE( tbc.application_id, tbc2.application_id)
ORDER BY tbc.byte_count DESC
-- LIMIT 10 
