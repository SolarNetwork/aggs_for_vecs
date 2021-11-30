load test_helper

@test "int16 add" {
  result="$(query "SELECT vec_add(ARRAY[1,3,5]::smallint[], ARRAY[2,4,6]::smallint[])")";
  echo $result;
  [ "$result" = "{3,7,11}" ]
}

@test "int32 add" {
  result="$(query "SELECT vec_add(ARRAY[1,3,5]::integer[], ARRAY[2,4,6]::integer[])")";
  echo $result;
  [ "$result" = "{3,7,11}" ]
}

@test "int64 add" {
  result="$(query "SELECT vec_add(ARRAY[1,3,5]::bigint[], ARRAY[2,4,6]::bigint[])")";
  echo $result;
  [ "$result" = "{3,7,11}" ]
}

@test "float32 add" {
  result="$(query "SELECT vec_add(ARRAY[1.0,2.4,3.4]::real[], ARRAY[3.4,0.5,0.9]::real[])")";
  echo $result;
  [ "$result" = "{4.4,2.9,4.3}" ]
}

@test "float64 add" {
  result="$(query "SELECT vec_add(ARRAY[1.1,2.2,3.4]::float[], ARRAY[3.4,2.2,1.1]::float[])")";
  echo $result;
  [ "$result" = "{4.5,4.4,4.5}" ]
}

@test "numeric add" {
  result="$(query "SELECT vec_add(ARRAY[1.23,2.234,3.456]::numeric[], ARRAY[3.456,2.234,1.123]::numeric[])")";
  echo $result;
  [ "$result" = "{4.686,4.468,4.579}" ]
}

@test "numeric add measurements" {
  result="$(query "SELECT tot FROM (SELECT vec_add(nums, lag(nums) OVER (ORDER BY sensor_id)) AS tot FROM measurements d WHERE sensor_id IN (3,4)) d WHERE d.tot IS NOT NULL")";
  echo $result;
  [ "$result" = "{2.46,NULL,5.79}" ]
}
