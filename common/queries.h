
void query_clear_database(t_database *db);
void query_init_database(t_database *db);

long query_insert_device(t_database *db, const char *name, float lat, float lon, float ele);
long query_insert_sensor(t_database *db, long metric_id, long device_id, const char *href, const char *unit, const char *sensor_type, const char *data_type);
long query_insert_metric(t_database *db, const char *metric);

void query_list_metrics(t_database *db, t_db_result **db_result);
void query_list_devices(t_database *db, t_db_result **db_result);
