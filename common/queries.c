#include "ext_database.h"

#include "messages.h"
#include "chainlib.h"
#include "queries.h"

static const char *insert_device_query = \
"INSERT INTO devices"
"(name, href, latitude, longitude, elevation, x, z) "
"VALUES (\"%s\", \"%s\", %f, %f, %f, %f, %f)";

static const char *update_sensor_query = \
"UPDATE sensors SET timestamp = \"%s\", value = %lf WHERE href=(\"%s\"); "
"SELECT devices.name FROM devices, sensors WHERE sensors.device_id = devices.device_id "
"AND sensors.href=(\"%s\");";

static const char *insert_metric_query = \
"INSERT OR IGNORE INTO metrics (name) VALUES (\"%s\");"
"SELECT metric_id FROM metrics WHERE name=(\"%s\");";

static const char *insert_sensor_query = \
"INSERT INTO sensors"
"(metric_id, device_id, href, unit, sensorType, dataType, value, timestamp) "
"VALUES (%ld, %ld, \"%s\", \"%s\", \"%s\", \"%s\", %lf, \"%s\")";

static const char *create_devices_table = \
"CREATE TABLE devices"
"("
"device_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name VARCHAR(255),"
"href VARCHAR(255),"
"latitude FLOAT,"
"longitude FLOAT,"
"elevation FLOAT,"
"x FLOAT,"
"z FLOAT"
")";

static const char *create_sensors_table = \
"CREATE TABLE sensors"
"("
"sensor_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"metric_id INTEGER REFERENCES metrics(metric_id) ON UPDATE CASCADE ON DELETE CASCADE,"
"device_id INTEGER REFERENCES devices(device_id) ON UPDATE CASCADE ON DELETE CASCADE,"
"href VARCHAR(255),"
"unit VARCHAR(255),"
"sensorType VARCHAR(255),"
"dataType VARCHAR(255),"
"value DOUBLE,"
"timestamp VARCHAR(255)"
")";

static const char *create_metrics_table = \
"CREATE TABLE metrics"
"("
"metric_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name VARCHAR(255) UNIQUE"
")";

static const char *list_metrics_by_device_name = \
"SELECT metrics.name FROM sensors, devices, metrics WHERE "
"sensors.device_id=devices.device_id AND "
"devices.name=(\"%s\") AND "
"sensors.metric_id=metrics.metric_id";

static const char *list_nearest_devices = \
"SELECT name, x, z, "
"(((x - %lf) * (x - %lf)) + ((z - %lf) * (z - %lf))) AS distance "
"FROM devices "
"ORDER BY distance "
"LIMIT %ld";

static const char *list_devices_near_point = \
"SELECT name, x, z, "
"(((x - %lf) * (x - %lf)) + ((z - %lf) * (z - %lf))) AS distance "
"FROM devices WHERE distance <= %lf * %lf "
"ORDER BY distance";



static const char *list_metrics = "SELECT name FROM metrics";

static const char *list_devices = "SELECT name, x, z FROM devices";

static const char *get_metric = "SELECT name FROM metrics WHERE metric_id=%ld";

static const char *get_device_by_name = "SELECT device_id FROM devices WHERE name=\"%s\"";

static const char *list_sensors = "SELECT href FROM sensors";

static const char *delete_device = \
"DELETE FROM devices WHERE "
"device_id=%ld";

static const char *delete_sensors_by_device = \
"DELETE FROM sensors WHERE "
"device_id=%ld";

static const char *get_device_location = \
"SELECT latitude, longitude, elevation, x, z "
"FROM devices WHERE name=\"%s\"";

static const char *get_data_by_device_name = \
"SELECT sensors.value, sensors.timestamp, metrics.name FROM sensors, devices, metrics "
"WHERE sensors.device_id=devices.device_id AND "
"devices.name=(\"%s\") AND "
"sensors.metric_id=metrics.metric_id";

static const char *get_data_by_sensor_href = \
"SELECT sensors.value, sensors.timestamp, metrics.name FROM sensors, metrics "
"WHERE sensors.metric_id=metrics.metric_id AND "
"sensors.href = (\"%s\")";

static const char *get_data_by_device_name_metric_name = \
"SELECT sensors.value, sensors.timestamp FROM sensors, devices, metrics "
"WHERE sensors.device_id=devices.device_id AND "
"sensors.metric_id=metrics.metric_id AND "
"devices.name=(\"%s\") AND "
"metrics.name=(\"%s\")";

static const char *get_near_data_by_metric_name = \
"SELECT sensors.value, devices.x, devices.z, devices.name "
"(((devices.x - %lf) * (devices.x - %lf)) + ((devices.z - %lf) * (devices.z - %lf))) AS distance "
"FROM sensors, devices, metrics WHERE "
"distance <= %lf * %lf AND "
"sensors.device_id=devices.device_id AND "
"sensors.metric_id=metrics.metric_id AND "
"metrics.name=(\"%s\") "
"ORDER BY distance";

static const char *get_data_by_metric_name = \
"SELECT sensors.value, devices.x, devices.z, devices.name "
"FROM sensors, devices, metrics WHERE "
"sensors.device_id=devices.device_id AND "
"sensors.metric_id=metrics.metric_id AND "
"metrics.name=(\"%s\") ";

static const char *get_sensor_href_by_device_name_metric_name = \
"SELECT sensors.href FROM sensors, devices, metrics "
"WHERE sensors.device_id=devices.device_id AND "
"sensors.metric_id=metrics.metric_id AND "
"devices.name=(\"%s\") AND "
"metrics.name=(\"%s\")";


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

long query_insert_device(t_database *db, const char *name, const char *href,
                         float lat, float lon, float ele){
    t_max_err err = MAX_ERR_NONE;
    double xz[2];
    chain_convert_geo_xz(lat, lon, xz);
    err = db_query(db, NULL, insert_device_query, name, href, lat, lon, ele, xz[0], xz[1]);
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

long query_insert_sensor(t_database *db, long metric_id, long device_id, const char *href,
                         const char *unit, const char *sensor_type, const char *data_type,
                         double value, const char *timestamp){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, insert_sensor_query, metric_id, device_id, href, unit,
                   sensor_type, data_type, value, timestamp);
    if (err)
        chain_error("Error inserting sensor");
    long id;
    db_query_getlastinsertid(db, &id);

    return id;
}

void query_update_sensor(t_database *db, const char *sensor_href, const char *timestamp, double value,
                         t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, update_sensor_query, timestamp, value, sensor_href, sensor_href); 

    if (err)
        chain_error("Error updating sensor");


}

void query_list_metrics(t_database *db, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_metrics);
    if (err)
        chain_error("Error getting metrics list");
}

void query_list_metrics_by_device_name(t_database *db, const char *device_name, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_metrics_by_device_name, device_name);
    if (err)
        chain_error("Error getting metrics list by device name");
}

void query_list_devices(t_database *db, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_devices);
    if (err)
        chain_error("Error getting devices list");
}

void query_list_sensors(t_database *db, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_sensors);
    if (err)
        chain_error("Error getting sensors list");
}

void query_list_devices_near_point(t_database *db, double x, double z, double s, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_devices_near_point, x, x, z, z, s, s);
    if (err)
        chain_error("Error getting devices near %ld, %ld", x, z);
}

void query_list_nearest_devices(t_database *db, double x, double z, long n, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, list_nearest_devices, x, x, z, z, n);
    if (err)
        chain_error("Error getting nearest devices");
}

void query_metric_by_id(t_database *db, long metric_id, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_metric, metric_id);
    if (err)
        chain_error("Error getting metric");
}

long query_device_by_name(t_database *db, const char *device_name){
    t_max_err err = MAX_ERR_NONE;

    t_db_result *db_result = NULL;

    err = db_query(db, &db_result, get_device_by_name, device_name);
    if (err)
        chain_error("Error getting device");

    if(!db_result_numrecords(db_result)){
        chain_error("Could not find device");
        return 0;
    }

    long id = db_result_long(db_result, 0, 0);

    return id;
}


void query_get_device_location(t_database *db, const char *device_name, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_device_location, device_name);
    if (err)
        chain_error("Error getting device location");
}

void query_data_by_device_name(t_database *db, const char *device_name, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_data_by_device_name, device_name);
    if (err)
        chain_error("Error getting device data");
}

void query_data_by_sensor_href(t_database *db, const char *sensor_href, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_data_by_sensor_href, sensor_href);
    if (err)
        chain_error("Error getting device data");
}

void query_data_by_device_name_metric_name(t_database *db, const char *device_name,
                                           const char *metric_name, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_data_by_device_name_metric_name, device_name, metric_name);
    if (err)
        chain_error("Error getting device-metric data");
}

void query_sensor_href_by_device_name_metric_name(t_database *db, const char *device_name,
                                                  const char *metric_name, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_sensor_href_by_device_name_metric_name, device_name, metric_name);
    if (err)
        chain_error("Error getting sensor href");
}

void query_delete_device(t_database *db, long device_id){
    t_max_err err = MAX_ERR_NONE;
    t_db_result *db_result = NULL;

    err = db_query(db, &db_result, delete_device, device_id);
    if (err)
        chain_error("Error deleting device");

    err = db_query(db, &db_result, delete_sensors_by_device, device_id);
    if (err)
        chain_error("Error deleting sensors");
}

void query_near_data_by_metric_name(t_database *db, double x, double z, double s,
                                    const char *metric, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_near_data_by_metric_name, x, x, z, z, s, s, metric);
    if (err)
        chain_error("Error getting data");
    long num_results = db_result_numrecords(*db_result);
    if (!num_results){
        object_free(*db_result);
        *db_result = NULL;
        query_data_by_metric_name(db, metric, db_result);
    }
}

void query_data_by_metric_name(t_database *db, const char *metric, t_db_result **db_result){
    t_max_err err = MAX_ERR_NONE;

    err = db_query(db, db_result, get_data_by_metric_name, metric);
    if (err)
        chain_error("Error getting data");
}
