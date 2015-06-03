/**
 @file
 chain.test - a simple chain_site object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_strings.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"
#include "ext_dictobj.h"

#include "jansson.h"

#include "messages.h"
#include "requests.h"

#define URL_SIZE 1024

typedef struct chain_site
{
    t_object s_obj;
    t_systhread s_systhread;
    t_systhread_mutex s_mutex;
    int s_systhread_cancel;
    void *s_qelem;
    void *s_outlet;
    int s_sleeptime;
    int s_value;
    t_symbol *s_dict_name;
    t_symbol *s_url;
    t_dictionary *s_dictionary;

} t_chain_site;

void *chain_site_new(t_symbol *s, long argc, t_atom *argv);
void chain_site_free(t_chain_site *x);
void chain_site_int(t_chain_site *x, long n);
void chain_site_bang(t_chain_site *x);
void chain_site_sleeptime(t_chain_site *x, long sleeptime);
void chain_site_stop(t_chain_site *x);
void *chain_site_threadproc(t_chain_site *x);
void chain_site_qfn(t_chain_site *x);
void chain_site_set_dict_name(t_chain_site *x, void *attr, long argc, t_atom *argv);
void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv);

static t_class *s_chain_site_class = NULL;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.site", (method)chain_site_new, (method)chain_site_free, \
                  sizeof(t_chain_site), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_site_bang, "bang", 0);
    class_addmethod(c, (method)chain_site_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_site_sleeptime, "sleeptime", A_DEFLONG, 0);
    class_addmethod(c, (method)chain_site_stop, "cancel", 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_site, s_dict_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_site_set_dict_name);

    CLASS_ATTR_SYM(c, "url", 0, t_chain_site, s_url);
    CLASS_ATTR_ACCESSORS(c, "url", NULL, chain_site_set_url);

    class_register(CLASS_BOX, c);
    
    s_chain_site_class = c;
    
    return 0;
}

void *chain_site_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_site *x = (t_chain_site *)object_alloc(s_chain_site_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *dict_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        dict_name = atom_getsym(argv);

    if (!x->s_dict_name) {
        if (dict_name)
            object_attr_setsym(x, gensym("name"), dict_name);
        else
            object_attr_setsym(x, gensym("name"), symbol_unique());
    }

    attr_args_process(x, argc, argv);

    x->s_outlet = outlet_new(x, NULL);
    x->s_qelem = qelem_new(x, (method)chain_site_qfn);
    x->s_systhread = NULL;
    systhread_mutex_new(&x->s_mutex,0);
    x->s_sleeptime = 1000;
    x->s_value = 0;
    
    return x;
}

void chain_site_set_dict_name(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *dict_name = atom_getsym(argv);
    if (!x->s_dict_name || !dict_name || x->s_dict_name!=dict_name){
        if (x->s_dictionary)
            object_free(x->s_dictionary);
        x->s_dictionary = dictionary_new();
        x->s_dictionary = dictobj_register(x->s_dictionary, &dict_name);
        x->s_dict_name = dict_name;
    }
}

void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *url_sym = atom_getsym(argv);

    if (!x->s_url || !url_sym || x->s_url!=url_sym){
        dictionary_deleteentry(x->s_dictionary, gensym("url"));
        dictionary_appendsym(x->s_dictionary, gensym("url"), url_sym);
        x->s_url = url_sym;
        char url_str[URL_SIZE];
        strncpy_zero(url_str, url_sym->s_name, sizeof(url_str));
        chain_debug("Set url to: %s", url_str);
    }
}

void chain_site_free(t_chain_site *x)
{
    chain_site_stop(x);

    if (x->s_dictionary)
        object_free(x->s_dictionary);

    if (x->s_qelem)
        qelem_free(x->s_qelem);

    if (x->s_mutex)
        systhread_mutex_free(x->s_mutex);
}

void chain_site_sleeptime(t_chain_site *x, long sleeptime)
{
    if (sleeptime<10)
        sleeptime = 10;
    x->s_sleeptime = (int)sleeptime;
}

void chain_site_int(t_chain_site *x, long n)
{
    systhread_mutex_lock(x->s_mutex);
    x->s_value = (int)n;
    systhread_mutex_unlock(x->s_mutex);
}

void chain_site_bang(t_chain_site *x)
{
    // chain_site_stop(x);

    // if (x->s_systhread == NULL) {
    //     post("Starting a new thread");
    //     systhread_create((method) chain_site_threadproc, x, 0, 0, 0, &x->s_systhread);
    // }

    char *text;
    json_t *root;
    json_error_t error;

    text = chain_request(x->s_url->s_name);
    if(!text){
        object_error(&x->s_obj, "Request failed totally dude...");
        return;
    }
    root = json_loads(text, 0, &error);
    free(text);

    if(!root){
        object_error(&x->s_obj, "Request did not return valid JSON.");
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
        chain_info("Got device name: %s", name_text);
    }

    json_decref(root);

}

void chain_site_stop(t_chain_site *x)
{
    unsigned int ret;

    if (x->s_systhread) {
        post("stopping our thread");
        x->s_systhread_cancel = true;
        systhread_join(x->s_systhread, &ret);
        x->s_systhread = NULL;
    }
}

void chain_site_qfn(t_chain_site *x){

    int myValue;

    systhread_mutex_lock(x->s_mutex);
    myValue = x->s_value;
    systhread_mutex_unlock(x->s_mutex);

    outlet_int(x->s_outlet, myValue);
}

void *chain_site_threadproc(t_chain_site *x) {
    while(!x->s_systhread_cancel){
        systhread_mutex_lock(x->s_mutex);
        x->s_value++;
        systhread_mutex_unlock(x->s_mutex);

        qelem_set(x->s_qelem);

        systhread_sleep(x->s_sleeptime);
    }

    x->s_systhread_cancel = false;

    systhread_exit(0);
    return NULL;
}
