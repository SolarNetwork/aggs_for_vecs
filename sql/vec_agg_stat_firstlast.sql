-- first/last stats int2
SELECT vec_agg_first(smallints ORDER BY sensor_id) AS firsts
	, vec_agg_last(smallints ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats int4
SELECT vec_agg_first(ints ORDER BY sensor_id) AS firsts
	, vec_agg_last(ints ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats int8
SELECT vec_agg_first(bigints ORDER BY sensor_id) AS firsts
	, vec_agg_last(bigints ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats float4
SELECT vec_agg_first(reals ORDER BY sensor_id) AS firsts
	, vec_agg_last(reals ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats float8
SELECT vec_agg_first(floats ORDER BY sensor_id) AS firsts
	, vec_agg_last(floats ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats numeric
SELECT vec_agg_first(nums ORDER BY sensor_id) AS firsts
	, vec_agg_last(nums ORDER BY sensor_id) AS lasts
FROM measurements;

-- first/last stats numeric 2
SELECT vec_agg_first(data_i ORDER BY ts) AS firsts
	, vec_agg_last(data_i ORDER BY ts) AS lasts
FROM measurements2;
