BEGIN; 

DROP TABLE IF EXISTS measurements;
CREATE TABLE measurements (
  sensor_id INTEGER PRIMARY KEY,
  floats FLOAT[],
  ints INT[],
  nums NUMERIC[]
);
CREATE INDEX idx_measurements_sensor_id ON measurements (sensor_id);

INSERT INTO measurements
VALUES
(1, NULL, NULL, NULL),
(2, ARRAY[NULL, NULL, NULL]::float[], ARRAY[NULL, NULL, NULL]::int[], ARRAY[NULL, NULL, NULL]::numeric[]),
(3, ARRAY[1, 2, 3], ARRAY[1, 2, 3], ARRAY[1, 2, 3]),
(4, ARRAY[1, NULL, 2], ARRAY[1, NULL, 2], ARRAY[1, NULL, 2]),
(5, ARRAY[1, 2, 4], ARRAY[1, 2, 4], ARRAY[1, 2, 4]),
(6, ARRAY[1, 1, 4], ARRAY[1, 1, 4], ARRAY[1, 1, 4])
;

COMMIT;
