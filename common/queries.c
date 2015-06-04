#include "ext_database.h"

#include "messages.h"

void query_init_database(t_database *db);
void query_clear_database(t_database *db);
void query_insert_device(t_database *db, const char *name, float lat, float lon, float ele);

static const char *insert_device_query = \
"INSERT INTO devices"
"(name, latitude, longitude, elevation) "
"VALUES (\"%s\", %f, %f, %f)";


void query_init_database(t_database *db){
    db_query_table_new(db, "devices");
    db_query_table_new(db, "sensors");
    db_query_table_new(db, "metrics");
    db_query_table_addcolumn(db, "devices", "name", "varchar(255)", NULL);
    db_query_table_addcolumn(db, "devices", "latitude", "float", NULL);
    db_query_table_addcolumn(db, "devices", "longitude", "float", NULL);
    db_query_table_addcolumn(db, "devices", "elevation", "float", NULL);
}

void query_clear_database(t_database *db){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, "DELETE FROM devices");
    err = db_query(db, NULL, "DELETE FROM sensors");
    err = db_query(db, NULL, "DELETE FROM metrics");
    if (err)
        chain_error("Error clearing database");
}

void query_insert_device(t_database *db, const char *name, float lat, float lon, float ele){
    t_max_err err = MAX_ERR_NONE;
    err = db_query(db, NULL, insert_device_query, name, lat, lon, ele);
    if (err)
        chain_error("Error inserting devices");
    // chain_info(insert_device_query, name, lat, lon, ele);
}
