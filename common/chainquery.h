
void chain_load_summary(const char *url, t_database *db);
void chain_get_websocket(const char *url, const char **wstext);
void chain_get_data(const char *url, long start, long end, double **data, long *data_len);