
void query_clear_database(t_database *db);
void query_init_database(t_database *db);

long query_insert_device(t_database *db, const char *name, const char *href, float lat, float lon, float ele);
long query_insert_sensor(t_database *db, long metric_id, long device_id, const char *href,
                         const char *unit, const char *sensor_type, const char *data_type,
                         double value, const char *timestamp);
long query_insert_metric(t_database *db, const char *metric);

void query_update_sensor(t_database *db, const char *sensor_href, const char *timestamp, double value, t_db_result **db_result);

void query_list_metrics(t_database *db, t_db_result **db_result);
void query_list_devices(t_database *db, t_db_result **db_result);
void query_list_devices_near_point(t_database *db, double x, double z, double s, t_db_result **db_result);
void query_list_nearest_devices(t_database *db, double x, double z, long n, t_db_result **db_result);
void query_metric_by_id(t_database *db, long metric_id, t_db_result **db_result);
void query_list_metrics_by_device_name(t_database *db, const char *device_name, t_db_result **db_result);

long query_device_by_name(t_database *db, const char *device_name);
void query_data_by_device_name(t_database *db, const char *device_name, t_db_result **db_result);
void query_data_by_sensor_href(t_database *db, const char *sensor_href, t_db_result **db_result);
void query_data_by_device_name_metric_name(t_database *db, const char *device_name,
                                           const char *metric_name, t_db_result **db_result);
void query_sensor_href_by_device_name_metric_name(t_database *db, const char *device_name,
                                                  const char *metric_name, t_db_result **db_result);
   