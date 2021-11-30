load test_helper

@test "int16 div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[2,4,6]::smallint[], ARRAY[1,3,5]::smallint[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,1,1} | smallint[]" ]
}

@test "int32 div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[2,4,6]::integer[], ARRAY[1,3,5]::integer[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,1,1} | integer[]" ]
}

@test "int64 div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[2,4,6]::bigint[], ARRAY[1,3,5]::bigint[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,1,1} | bigint[]" ]
}

@test "float32 div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[3.4,1.0,8.5]::real[], ARRAY[2.0,0.5,4.0]::real[]) AS r) r")";
  echo $result;
  [ "$result" = "{1.7,2,2.125} | real[]" ]
}

@test "float32 div zero" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[1.0,2.0,3.0]::real[], ARRAY[0.0,1.0,2.0]::real[]) AS r) r")";
  echo $result;
  [ "$result" = "{Infinity,2,1.5} | real[]" ]
}

@test "float64 div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[3.4,1.0,8.5]::float[], ARRAY[2.0,0.5,4.0]::float[]) AS r) r")";
  echo $result;
  [ "$result" = "{1.7,2,2.125} | double precision[]" ]
}

@test "float64 div zero" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[1.0,2.0,3.0]::float[], ARRAY[0.0,1.0,2.0]::float[]) AS r) r")";
  echo $result;
  [ "$result" = "{Infinity,2,1.5} | double precision[]" ]
}

@test "numeric div" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[3.4,1.0,8.5]::numeric[], ARRAY[2.0,0.5,4.0]::numeric[]) AS r) r")";
  echo $result;
  [ "$result" = "{1.7000000000000000,2.0000000000000000,2.1250000000000000} | numeric[]" ]
}

@test "numeric div zero" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_div(ARRAY[1.0,2.0,3.0]::numeric[], ARRAY[0.0,1.0,2.0]::numeric[]) AS r) r")";
  echo $result;
  # TODO: for PG 14+ we expect Infinity, not NaN; how model that requirement in test?
  # [ "$result" = "{Infinity,2.0000000000000000,1.5000000000000000} | numeric[]" ]
  [ "$result" = "{NaN,2.0000000000000000,1.5000000000000000} | numeric[]" ]
}

@test "numeric div measurements" {
  result="$(query "SELECT tot FROM (SELECT vec_div(nums, lag(nums) OVER (ORDER BY sensor_id)) AS tot FROM measurements d WHERE sensor_id IN (3,4)) d WHERE d.tot IS NOT NULL")";
  echo $result;
  [ "$result" = "{1.00000000000000000000,NULL,0.67826086956521739130}" ]
}
