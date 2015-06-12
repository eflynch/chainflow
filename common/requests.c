/*
 * Adapted from Petri Lehtinen <petri@digip.org>
 * 
 * Jansson example code github_commits.c ( Copyright (c) 2009-2014 )
 *
 */

#include <curl/curl.h>
#include "messages.h"

#define BUFFER_SIZE  (20 * 1024 * 1024)  /* 20 MB */
#define TIMEOUT 1000 * 10


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

static char *request(const char *url)
{
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl){
        chain_error("Problem occured with CURL.");
        goto error;
    }

    data = malloc(BUFFER_SIZE);
    if(!data){
        chain_error("Problem occured with DATA.");
        goto error;
    }

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    headers = curl_slist_append(headers, "User-Agent: MaxChain");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TIMEOUT);
    status = curl_easy_perform(curl);
    if (status != 0){
        chain_error("Failed to connect to URL %s with status: %ld", url, status);
        goto error;
    }
    chain_info("Connected to URL: %s", url);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200)
    {
        chain_error("Server responded with code %ld", code);
        goto error;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[write_result.pos] = '\0';
    return data;
error:
    if(data)
        free(data);
    if(curl)
        curl_easy_cleanup(curl);
    if(headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();
    return NULL;
}

char *chain_request(const char *url){
    return request(url);
}
