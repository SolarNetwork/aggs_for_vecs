load test_helper

@test "int16 mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1,3,5]::smallint[], ARRAY[2,4,6]::smallint[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,12,30} | smallint[]" ]
}

@test "int32 mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1,3,5]::integer[], ARRAY[2,4,6]::integer[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,12,30} | integer[]" ]
}

@test "int64 mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1,3,5]::bigint[], ARRAY[2,4,6]::bigint[]) AS r) r")";
  echo $result;
  [ "$result" = "{2,12,30} | bigint[]" ]
}

@test "float32 mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1.0,2.4,3.4]::real[], ARRAY[3.4,0.5,0.9]::real[]) AS r) r")";
  echo $result;
  [ "$result" = "{3.4,1.2,3.06} | real[]" ]
}

@test "float64 mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1.1,2.2,3.4]::float[], ARRAY[3.4,2.0,1.1]::float[]) AS r) r")";
  echo $result;
  [ "$result" = "{3.74,4.4,3.74} | double precision[]" ]
}

@test "numeric mul" {
  result="$(query "SELECT r, pg_typeof(r) FROM (SELECT vec_mul(ARRAY[1.23,2.234,3.456]::numeric[], ARRAY[3.456,2.234,1.123]::numeric[]) AS r) r")";
  echo $result;
  [ "$result" = "{4.25088,4.990756,3.881088} | numeric[]" ]
}

@test "numeric mul measurements" {
  result="$(query "SELECT tot FROM (SELECT vec_mul(nums, lag(nums) OVER (ORDER BY sensor_id)) AS tot FROM measurements d WHERE sensor_id IN (3,4)) d WHERE d.tot IS NOT NULL")";
  echo $result;
  [ "$result" = "{1.5129,NULL,8.0730}" ]
}
