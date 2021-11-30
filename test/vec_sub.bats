load test_helper

@test "int16 sub" {
  result="$(query "SELECT vec_sub(ARRAY[1,2,3]::smallint[], ARRAY[3,2,1]::smallint[])")";
  echo $result;
  [ "$result" = "{-2,0,2}" ]
}

@test "int32 sub" {
  result="$(query "SELECT vec_sub(ARRAY[1,2,3]::integer[], ARRAY[3,2,1]::integer[])")";
  echo $result;
  [ "$result" = "{-2,0,2}" ]
}

@test "int64 sub" {
  result="$(query "SELECT vec_sub(ARRAY[1,2,3]::bigint[], ARRAY[3,2,1]::bigint[])")";
  echo $result;
  [ "$result" = "{-2,0,2}" ]
}

@test "float32 sub" {
  result="$(query "SELECT vec_sub(ARRAY[1.0,2.2,3.4]::real[], ARRAY[3.4,2.2,1.0]::real[])")";
  echo $result;
  [ "$result" = "{-2.4,0,2.4}" ]
}

@test "float64 sub" {
  result="$(query "SELECT vec_sub(ARRAY[1.1,2.2,3.4]::float[], ARRAY[3.4,2.2,1.1]::float[])")";
  echo $result;
  [ "$result" = "{-2.3,0,2.3}" ]
}

@test "numeric sub" {
  result="$(query "SELECT vec_sub(ARRAY[1.23,2.234,3.456]::numeric[], ARRAY[3.456,2.234,1.123]::numeric[])")";
  echo $result;
  [ "$result" = "{-2.226,0.000,2.333}" ]
}

@test "numeric sub measurements" {
  result="$(query "SELECT diff FROM (SELECT vec_sub(nums, lag(nums) OVER (ORDER BY sensor_id)) AS diff FROM measurements d WHERE sensor_id IN (3,4)) d WHERE d.diff IS NOT NULL")";
  echo $result;
  [ "$result" = "{0.00,NULL,-1.11}" ]
}
