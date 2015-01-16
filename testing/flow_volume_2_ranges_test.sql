WITH rng AS (
    SELECT
      mid-120 as a1, mid+120 as b1,
      -- mid-12 as a1, mid+12 as b1,
      -- mid-12000 as a1, mid+12000 as b1,
      -- mid-5 as a1, mid+5 as b1,
      mid AS mid FROM (
    SELECT ((MIN(last_time_sec)::bigint+MAX(last_time_sec))/2)::int AS mid,
      MIN(last_time_sec) AS min_ts, MAX(last_time_sec) AS max_ts
    FROM flow_samples ) AS m )
,td1 AS (
  SELECT
    flow_id,
    last(application_id) AS application_id,
    flow_volume(byte_count,start_time_sec, last_time_sec,(SELECT a1 FROM rng), (SELECT b1 FROM rng)) AS sum_bytes_all
  FROM flow_samples
  WHERE last_time_sec BETWEEN (SELECT a1 FROM rng)-30 AND (SELECT b1 FROM rng)+70 AND start_time_sec<=(SELECT b1 FROM rng)
    -- AND flow_id in (1625651,1625645)
  GROUP BY flow_id )
,td2 AS (
SELECT flow_id, application_id, SUM(sum_bytes_all) sum_bytes_all FROM (
  SELECT
    flow_id,
    last(application_id) AS application_id,
    flow_volume(byte_count,start_time_sec, last_time_sec,(SELECT a1 FROM rng), (SELECT mid FROM rng) ) AS sum_bytes_all
  FROM flow_samples
  WHERE last_time_sec  BETWEEN (SELECT a1 FROM rng)-30 AND (SELECT mid FROM rng)+70 AND start_time_sec<=(SELECT mid FROM rng)
    -- AND flow_id in (1625651,1625645)
  GROUP BY flow_id

    UNION ALL 

  SELECT
    flow_id,
    last(application_id) AS application_id,
    flow_volume(byte_count,start_time_sec, last_time_sec,(SELECT mid FROM rng), (SELECT b1 FROM rng)) AS sum_bytes_all
  FROM flow_samples
  WHERE last_time_sec  BETWEEN (SELECT mid FROM rng)-30 AND (SELECT b1 FROM rng)+70 AND start_time_sec<=(SELECT b1 FROM rng)
    -- AND flow_id in (1625651,1625645)
  GROUP BY flow_id

  ) AS comb
GROUP BY flow_id, application_id
)
SELECT SUM(ABS(diff)) FROM (
SELECT td1.flow_id, td1.application_id, td2.application_id, td1.application_id<>td2.application_id,
  td1.sum_bytes_all,
  td2.sum_bytes_all,
  td1.sum_bytes_all-td2.sum_bytes_all AS diff
FROM td1 FULL JOIN td2 ON td2.flow_id=td1.flow_id
) AS r