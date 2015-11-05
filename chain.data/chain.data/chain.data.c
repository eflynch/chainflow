/**
 @file
 chain.data - a simple chain_data object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "chainworker.h"

typedef struct chain_data
{
    t_chain_worker s_worker;
    void *s_outlet;
    void *s_outlet2;
    t_symbol *s_savefile;
    long s_normalize;
    double s_interval;
    // Data storage
    long s_num_samples;
    double *s_values;
    double *s_offsets;
    long s_start;
    long s_end;
    t_symbol *s_metric_name;
} t_chain_data;

// Create + Destroy
void *chain_data_new(t_symbol *s, long argc, t_atom *argv);
void chain_data_free(t_chain_data *x);


// Methods
void chain_data_int(t_chain_data *x, long n);
void chain_data_bang(t_chain_data *x);
void chain_data_clear(t_chain_data *x);
void chain_data_set(t_chain_data *x, t_symbol *s, long argc, t_atom *argv);

// Attribute Setters
void chain_data_set_site_name(t_chain_data *x, void *attr, long argc, t_atom *argv);

// Notify
void chain_data_notify(t_chain_data *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_data_class = NULL;

t_symbol *ps_list;


int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.data", (method)chain_data_new, (method)chain_data_free, sizeof(t_chain_data), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_data_bang, "bang", 0);
    class_addmethod(c, (method)chain_data_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_data_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)chain_data_set, "set", A_GIMME, 0);
    class_addmethod(c (method)chain_data_clear, "clear", 0);
   
    CLASS_ATTR_SYM(c, "savefile", 0L, t_chain_data, s_savefile);
    CLASS_ATTR_LONG(c, "normalize", 0L, t_chain_data, s_normalize);
    CLASS_ATTR_FLOAT(c, "interval", 0L, t_chain_data, s_interval);
    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_data, s_worker.s_site_name);

    class_register(CLASS_BOX, c);
    
    s_chain_data_class = c;

    ps_list = gensym("list");
    
    return 0;
}

void *chain_data_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_data *x = (t_chain_data *)object_alloc(s_chain_data_class);
    
    chain_worker_new((t_chain_worker *)x, s, argc, argv);

    x->s_interval = 0.0;
    x->normalize = 0;
    x->savefile = NULL;

    // Initialize data storage
    x->s_offsets = NULL;
    x->s_values = NULL;
    x->s_num_samples = 0;
    x->s_metric_name = NULL;
    x->s_start = 0;
    x->s_end = 0;
    
    attr_args_process(x, argc, argv);
    
    x->s_outlet2 = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    
    return x;
}

void chain_data_free(t_chain_data *x)
{
    chain_worker_release_site((t_chain_worker *)x);
}


void chain_data_notify(t_chain_data *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_data_int(t_chain_data *x, long n)
{
}

/*
Output resampled list and resampled list with interleaved normalized offsets
*/
void chain_data_bang(t_chain_data *x)
{
    chain_data_output_resampled_list(x);
    chain_data_output_resampled_interleaved_list(x);
}

void chain_data_output_resampled_list(t_chain_data *x)
{
    if (!x->s_values || !x->s_offsets || !x->s_num_samples){
        // Fail silently
        chain_debug("No data.");
        return;
    }

    long normalize = x->s_normalize;
    double interval = x->s_interval;
    long num_samples = x->s_num_samples;
    long duration = x->s_end - x->s_start;

    // if interval == 0, return all data without resampling to periodic intervals
    if (interval == 0.0){
        chain_error("Not implemented");
        return;
    }

    long ac = ceil(duration / interval);
    t_atom av[ac];

    // Initialize first before point at time=0.0 to values[0] regardless of offsets[0]
    double offset_before_interval = 0.0;
    double value_before_interval = x->s_values[0];

    // Initialize first after point at offsets[0] values[0]
    double offset_after_interval = x->s_offsets[0];
    double value_after_interval = x->s_values[0];
    long after_idx = 0;

    for (long i=0; i<ac; i++){
        double interval_offset = interval * i;

        // ensure before_offset is not after interval_offset
        // if before_offset is after interval_offset, then we've screwed up the implementation
        if (offset_before_interval > interval_offset){
            chain_error("before_interval surpassed interval_offset");
            return;
        }

        // ensure after_offset is not before interval_offset
        // if it is before interval_offset, advance the before and after values until it is
        while (offset_after_interval < interval_offset){
            // if after_idx = num_samples - 1
            // then we force the after point to be at t=end value=values[num_samples-1]
            // (this should only happen for the last interval)
            if (after_idx >= num_samples - 1){
                offset_before_interval = x->s_offsets[num_samples - 1];
                value_before_interval = x->s_values[num_samples - 1];
                offset_after_interval = x->s_end;
                value_after_interval = x->s_values[num_samples - 1];
                break;
            } else {
                offset_before_interval = x->s_offsets[after_idx];
                value_before_interval = x->s_offsets[after_idx];
                after_idx++;
                offset_after_interval = x->s_offsets[after_idx];
                value_after_interval = x->s_offsets[after_idx];
            }
        }

        // interpolate between before and after
        // if before_offset == after_offset return the before_value
        double interval_value;
        if (offset_before_interval == offset_after_interval){
            interval_value = value_before_interval;
        } else {
            // (x1,y1).......(x3,y3)........(x2,y2)
            // y3 = y1 + (y2-y1)/(x2-x1)*(x3-x1)
            double end_point_difference = (value_after_interval - value_before_interval);
            double end_point_distance = (offset_after_interval - offset_before_interval);
            double interval_distance = (interval_offset - offset_before_interval);
            interval_value = value_before_interval + end_point_difference * interval_distance / end_point_distance;
        }

        atom_setfloat(av+i, interval_value);
    }

    outlet_anything(x->s_outlet, ps_list, ac, av);
}

void chain_data_output_resampled_interleaved_list(t_chain_data *x)
{
    if (!x->s_values || !x->s_offsets || !x->s_num_samples){
        // Fail silently
        chain_debug("No data.");
        return;
    }
}

void chain_data_clear(t_chain_data *x){
    if (x->s_values){
        free(x->s_values);
    }
    if (x->s_offsets){
        free(x->s_offsets);
    }
    x->s_num_samples = 0;
}


/*
Input: [metric_name] [start second] [end second] ([time delta] [value])
*/
void chain_data_set(t_chain_data *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc < 3 || argc % 2 == 0){
        chain_error("Invalid number of arguments for data stream");
        return;
    }

    // clear current data
    chain_data_clear(x);

    // allocate new data storage
    x->s_num_samples = (argc - 3) / 2;
    x->s_offsets = malloc(sizeof(double) * x->s_num_samples);
    x->s_values = malloc(sizeof(double) * x->s_num_samples);

    // unpack argv
    // preamble
    x->s_metric_name = atom_getsym(argv);
    x->s_start = atom_getlong(argv+1);
    x->s_end = atom_getlong(argv+2);

    t_atom *samples = argv + 3;

    // samples
    for (long i=0; i < x->s_num_samples; i++){
        t_atom *time_delta = samples + 2*i;
        t_atom *value = samples + 2*i + 1;

        double d_time_delta = atom_getfloat(time_delta);
        double d_value = atom_getfloat(value);

        *(x->s_offsets + i) = d_time_delta;
        *(x->s_values + i) = d_value;
    }
}
