#include "ext_database.h"

#include "messages.h"

static const char *insert_device_query = \
"INSERT INTO devices"
"(name, latitude, longitude, elevation) "
"VALUES (\"%s\", %f, %f, %f)";

static const char *insert_metric_query = \
"INSERT OR IGNORE INTO metrics (name) VALUES (\"%s\");"
"SELECT metrics_id FROM metrics WHERE name=(\"%s\");";

static const char *insert_sensor_query = \
"INSERT INTO sensors"
"(metric_id, device_id, href, unit, sensorType, dataType) "
"VALUES (%ld, %ld, \"%s\", \"%s\", \"%s\", \"%s\")";

static const char *create_devices_table = \
"CREATE TABLE devices"
"("
"devices_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name VARCHAR(255),"
"href VARCHAR(255),"
"latitude FLOAT,"
"longitude FLOAT,"
"elevation FLOAT"
")";

static const char *create_sensors_table = \
"CREATE TABLE sensors"
"("
"sensors_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"metric_id INTEGER REFERENCES metrics(metrics_id) ON UPDATE CASCADE ON DELETE CASCADE,"
"device_id INTEGER REFERENCES devices(devices_id) ON UPDATE CASCADE ON DELETE CASCADE,"
"href VARCHAR(255),"
"unit VARCHAR(255),"
"sensorType VARCHAR(255),"
"dataType VARCHAR(255),"
"value FLOAT"
")";

static const char *create_metrics_table = \
"CREATE TABLE metrics"
"("
"metrics_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name VARCHAR(255) UNIQUE"
")";

static const char *list_metrics = "SELECT name FROM metrics";

static const char *list_devices = "SELECT name FROM devices";

void query_init_database(t_database *db){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, NULL, create_devices_table);
    err = db_query(db, NULL, create_metrics_table);
    err = db_query(db, NULL, create_sensors_table);
    if (err)
        chain_error("Error setting up tables");
}

void query_clear_database(t_database *db){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, "DELETE FROM devices");
    err = db_query(db, NULL, "DELETE FROM sensors");
    err = db_query(db, NULL, "DELETE FROM metrics");
    if (err)
        chain_error("Error clearing database");
}

long query_insert_device(t_database *db, const char *name, float lat, float lon, float ele){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, insert_device_query, name, lat, lon, ele);
    if (err)
        chain_error("Error inserting devices");
    long id;
    db_query_getlastinsertid(db, &id);

    return id;
}

long query_insert_metric(t_database *db, const char *metric){
    t_max_err err = MAX_ERR_NONE;
    t_db_result *db_result = NULL;

    err = db_query(db, &db_result, insert_metric_query, metric, metric);
    if (err)
        chain_error("Error inserting metric");
    long id;
    id = db_result_long(db_result, 0, 0);

    return id;
}

long query_insert_sensor(t_database *db, long metric_id, long device_id, const char *href, const char *unit,
                         const char *sensor_type, const char *data_type){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, insert_sensor_query, metric_id, device_id, href, unit, sensor_type, data_type);
    if (err)
        chain_error("Error inserting sensor");
    long id;
    db_query_getlastinsertid(db, &id);

    return id;
}

void query_list_metrics(t_database *db, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_metrics);
    if (err)
        chain_error("Error getting metrics list");
}

void query_list_devices(t_database *db, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_devices);
    if (err)
        chain_error("Error getting devices list");
}
