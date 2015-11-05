#include "chain.site.h"

#include <string.h>
#include <time.h>
#include "libwebsockets.h"
#include "jansson.h"
#include "messages.h"
#include "chainevent.h"
#include "pseudoclock.h"

static int chain_callback(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
              enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len){
    switch (reason){
        case LWS_CALLBACK_CLOSED:
            chain_error("Websocket was closed!");
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            t_chain_site *x = (t_chain_site *)libwebsocket_context_user(ctx);

            char text[len+1];
            json_t *root, *timestamp, *value, *links, *ch_sensor, *href;
            json_error_t error;
            const char *timestamp_text;
            const char *href_text;
            const char *device_name_text;
            double d_value;
            
            strncpy(text, (char *)in, len);
            text[len] = '\0';

            root = json_loads(text, 0, &error);

            if(!json_is_object(root))
            {
                chain_error("JSON root is not an object");
                json_decref(root);
                return 1;
            }

            timestamp = json_object_get(root, "timestamp");
            value = json_object_get(root, "value");
            links = json_object_get(root, "_links");
            ch_sensor = json_object_get(links, "ch:sensor");
            href = json_object_get(ch_sensor, "href");
            timestamp_text = json_string_value(timestamp);
            d_value = json_real_value(value);
            href_text = json_string_value(href);

            float fracsecs;
            time_t unix_timestamp = time_from_string(timestamp_text, &fracsecs);

            t_chain_event e = chain_new_event(unix_timestamp, href_text, timestamp_text, d_value);

            int err = chain_site_update_sensors(x, &e);
            if (err){
                json_decref(root);
                return 1;
            }
            break;
        }
        default:
            break;
    }
    return 0;
}


static struct libwebsocket_protocols protocols[] = {
    {
        "chain-protocol",
        chain_callback,
        0,
    },
    { /* end of list */
        NULL,
        NULL,
        0
    }
};

struct libwebsocket_context *chain_websocket_connect(t_chain_site *x){
    struct lws_context_creation_info info = {
        .port = CONTEXT_PORT_NO_LISTEN,
        .iface = NULL,
        .protocols = protocols,
        .extensions = NULL,
        .token_limits = NULL,
        .ssl_cert_filepath = NULL,
        .ssl_private_key_filepath = NULL,
        .ssl_ca_filepath = NULL,
        .ssl_cipher_list = NULL,
        .gid = -1,
        .uid = -1,
        .options = 0,
        .user = x,
        .ka_time = 0,
        .ka_probes = 0,
        .ka_interval = 0,
        .provided_client_ssl_ctx = NULL
    };
    // char address[1024];
    // memcpy(address, x->s_url->s_name+5, strlen(x->s_url->s_name)-4);


    int port = 80;
    // int port = 8000;
    int use_ssl = 0;
    struct libwebsocket_context *context;
    struct libwebsocket *wsi;
    int ietf_version = -1;

    
    context = libwebsocket_create_context(&info);

    // TODO: Get this from url by parsing rather than hardcoding!!
    const char *address = "chain-api.media.mit.edu";
    // const cha
    const char *path = "/ws/site-7";
    // const char *path = "/";
    const char *host = address;
    const char *origin = address;
    wsi = libwebsocket_client_connect(context, address, port, use_ssl, path,
                                           host, origin, "chain-protocol", ietf_version);


    if (wsi == NULL){
        chain_error("websocket connect failed");
        return NULL;
    }

    return context;
}

