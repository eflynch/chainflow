#include "ext_database.h"

#include "messages.h"
#include "jansson.h"
#include "requests.h"
#include "queries.h"

void chainapi_load_summary(const char *url, t_database *db){
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
        json_t *device, *name;
        const char *name_text;

        device = json_array_get(device_array, i);
        if(!json_is_object(device)){
            chain_error("JSON device is not an object");
            json_decref(root);
            return;
        }
        name = json_object_get(device, "name");
        name_text = json_string_value(name);
        query_insert_device(db, name_text, 0.0f, 0.0f, 0.0f);
    }
    json_decref(root);
}
