#include <string.h>

#include "ext_database.h"

#include "messages.h"
#include "jansson.h"
#include "requests.h"
#include "queries.h"

#include "chainquery.h"
#include "pseudoclock.h"

static int chain_query(const char *url, json_t **root){
    char *text;
    json_error_t error;

    text = chain_request(url);
    if(!text){
        chain_error("Request to %s failed.", url);
        return 1;
    }
    *root = json_loads(text, 0, &error);
    free(text);
    if(!root){
        chain_error("Request did not return valid JSON.");
        return 1;
    }
    return 0;
}

void chain_get_data(const char *url, time_t start, time_t end, t_chain_event **events, long *num_events){
    // Get data history url from sensor url

    json_t *sensor_root;
    int err = 0;
    err = chain_query(url, &sensor_root);
    if (err){
        chain_error("Getting sensor root failed");
        return;
    }
    json_t *_links, *ch_dataHistory, *href;
    _links = json_object_get(sensor_root, "_links");
    ch_dataHistory = json_object_get(_links, "ch:dataHistory");
    href = json_object_get(ch_dataHistory, "href");

    const char *data_history_url = json_string_value(href);

    // Format data query url
    char rangequery[1024];
    sprintf(rangequery, "&timestamp__gte=%ld&timestamp__lt=%ld", start, end);

    char data_url[2048];
    strcpy(data_url, data_history_url);
    strcat(data_url, rangequery);

    // Parse result
    json_t *data_root;
    err = chain_query(data_url, &data_root);
    if (err){
        chain_error("Getting data root failed");
        return;
    }

    json_t *data_node = json_object_get(data_root, "data");

    long array_size = json_array_size(data_node);

    *events = malloc(sizeof(t_chain_event) * array_size);

    for (int i=0; i<array_size; i++){
        // Parse event
        json_t *data_point, *timestamp, *value;
        data_point = json_array_get(data_node, i);
        timestamp = json_object_get(data_point, "timestamp");
        value = json_object_get(data_point, "value");
        double value_double = json_real_value(value);
        const char *timestamp_text = json_string_value(timestamp);
        float fracsecs;
        time_t unix_timestamp = time_from_string(timestamp_text, &fracsecs);

        if (unix_timestamp <= 0){
            chain_error("Received a wonky timestamp: %s", timestamp_text);
        }

        // Create event
        t_chain_event e = chain_new_event(unix_timestamp, url, timestamp_text, value_double);
        memcpy(*events + i, &e, sizeof(t_chain_event));
    }

    *num_events = array_size;

    json_decref(sensor_root);
    json_decref(data_root);
}

void chain_get_websocket(const char *url, const char **wstext){
    json_t *root;
    chain_query(url, &root);

    json_t *_links = json_object_get(root, "_links");
    json_t *websocketStream = json_object_get(_links, "ch:websocketStream");
    json_t *jtwshref = json_object_get(websocketStream, "href");
    const char *wshreftext = json_string_value(jtwshref);
    *wstext = (char *)malloc((strlen(wshreftext) + 1) * sizeof(char));
    strcpy((char *)*wstext, wshreftext);

    json_decref(root);
}

void chain_load_summary(const char *url, t_database *db, long include_nonactive){
    json_t *root;

    char summary_url[1024];
    strcpy(summary_url, url);
    strcat(summary_url, "/summary");

    chain_query(summary_url, &root);

    if(!json_is_object(root))
    {
        chain_error("JSON root is not an object");
        json_decref(root);
        return;
    }
    json_t *device_array = json_object_get(root, "devices");

    if(!json_is_array(device_array))
    {
        chain_error("JSON device_array is not an array");
        json_decref(root);
        return;
    }

    query_clear_database(db);
    for (int i=0; i < json_array_size(device_array); i++){
        json_t *device, *name, *href, *sensor_array, *geoLocation, *latitude, *longitude, *elevation;
        const char *name_text_temp, *href_text;
        char name_text[256];
        long device_id;
        float lat, lon, ele;

        device = json_array_get(device_array, i);
        if(!json_is_object(device)){
            chain_error("JSON device is not an object");
            json_decref(root);
            return;
        }
        name = json_object_get(device, "name");
        name_text_temp = json_string_value(name);
        sprintf(name_text, "`%s`", name_text_temp);
        href = json_object_get(device, "href");
        href_text = json_string_value(href);

        geoLocation = json_object_get(device, "geoLocation");
        if(!json_is_object(geoLocation)){
            lat = 0L;
            lon = 0L;
            ele = 0L;
        } else {
            latitude = json_object_get(geoLocation, "latitude");
            longitude = json_object_get(geoLocation, "longitude");
            elevation = json_object_get(geoLocation, "elevation");
            lat = json_real_value(latitude);
            lon = json_real_value(longitude);
            ele = json_real_value(elevation);
        }

        device_id = query_insert_device(db, name_text, href_text, lat, lon, ele);

        int found_data = false;
        if (include_nonactive){
            found_data = true;
        }

        sensor_array = json_object_get(device, "sensors");
        for (int j=0; j < json_array_size(sensor_array); j++){
            json_t *sensor, *metric, *href, *unit, *sensor_type, *data_type, *data_array;
            const char *metric_text, *href_text, *unit_text, *sensor_type_text, *data_type_text;
            long metric_id, sensor_id;

            sensor = json_array_get(sensor_array, j);
            if(!json_is_object(sensor)){
                chain_error("JSON sensor is not an object");
                json_decref(root);
                return;
            }
            metric = json_object_get(sensor, "metric");
            metric_text = json_string_value(metric);

            metric_id = query_insert_metric(db, metric_text);

            href = json_object_get(sensor, "href");
            href_text = json_string_value(href);
            unit = json_object_get(sensor, "unit");
            unit_text = json_string_value(unit);
            sensor_type = json_object_get(sensor, "sensor-type");
            sensor_type_text = json_string_value(sensor_type);
            data_type = json_object_get(sensor, "dataType");
            data_type_text = json_string_value(data_type);
            data_array = json_object_get(sensor, "data");

            json_t *data, *value, *timestamp;
            double d_value;
            const char *timestamp_text;

            if (json_array_size(data_array)){
                data = json_array_get(data_array, json_array_size(data_array) - 1);
                if(!json_is_object(data)){
                    chain_error("JSON data is not an object");
                    json_decref(root);
                    return;
                }
                timestamp = json_object_get(data, "timestamp");
                timestamp_text = json_string_value(timestamp);
                value = json_object_get(data, "value");
                d_value = json_real_value(value);
                found_data = true;
            } else { 
                timestamp_text = NULL;
                d_value = 0;
            }

            sensor_id = query_insert_sensor(db, metric_id, device_id, href_text, unit_text,
                                            sensor_type_text, data_type_text, d_value, timestamp_text);
        }

        if (!found_data){
            query_delete_device(db, device_id);
        }
    }
    json_decref(root);
}
