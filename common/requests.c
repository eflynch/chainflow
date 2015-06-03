#include "jansson.h"
#include <curl/curl.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */


struct write_result
{
    char *data;
    int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl){
        object_post(x, "Something went wrong with CURL");
        x->s_systhread_cancel = true;
    }

    data = malloc(BUFFER_SIZE);
    if(!data){
        object_post(x, "Something went wrong with Data");
        x->s_systhread_cancel = true;
    }

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    headers = curl_slist_append(headers, "User-Agent: MaxChain");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
    curl_easy_setopt(curl, CURLOPT_URL, x->s_url->s_name);
    status = curl_easy_perform(curl);
    if (status != 0){
        object_post(x, "Failed to connect to URL %s", x->s_url->s_name);
        x->s_systhread_cancel = true;
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200)
    {
        object_post(x, "error: server responded with code %ld", code);
        x->s_systhread_cancel = true
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();


