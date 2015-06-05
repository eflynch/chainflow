#include "ext_database.h"

#include "messages.h"
#include "jansson.h"
#include "requests.h"
#include "queries.h"

void chain_load_summary(const char *url, t_database *db){
    char *text;
    json_t *root;
    json_error_t error;

    char summary_url[1024];
    strcpy(summary_url, url);
    strcat(summary_url, "/summary");

    text = chain_request(summary_url);
    if(!text){
        chain_error("Request failed totally dude...");
        return;
    }
    root = json_loads(text, 0, &error);
    free(text);

    if(!root){
        chain_error("Request did not return valid JSON.");
        return;
    }

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
        json_t *device, *name, *href, *sensor_array;
        const char *name_text, *href_text;
        long device_id;

        device = json_array_get(device_array, i);
        if(!json_is_object(device)){
            chain_error("JSON device is not an object");
            json_decref(root);
            return;
        }
        name = json_object_get(device, "name");
        name_text = json_string_value(name);
        href = json_object_get(device, "href");
        href_text = json_string_value(href);
        device_id = query_insert_device(db, name_text, 0.0f, 0.0f, 0.0f);

        sensor_array = json_object_get(device, "sensors");
        for (int j=0; j < json_array_size(sensor_array); j++){
            json_t *sensor, *metric, *href, *unit, *sensor_type, *data_type;
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

            sensor_id = query_insert_sensor(db, metric_id, device_id, href_text, unit_text, sensor_type_text, data_type_text);
        }
    }
    json_decref(root);
}
