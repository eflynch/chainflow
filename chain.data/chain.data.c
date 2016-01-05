/**
 @file
 chain.data - a simple chain_data object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"
#include "ext_path.h"

#include "messages.h"
#include "chainworker.h"

typedef struct chain_data
{
    t_chain_worker s_worker;
    void *s_outlet;
    void *s_outlet2;
    t_symbol *s_savefile;
    long s_autosave;
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
void chain_data_read(t_chain_data *x, t_symbol *s);
void chain_data_write(t_chain_data *x, t_symbol *s);

// Helpers
void chain_data_output_resampled_list(t_chain_data *x);
void chain_data_output_resampled_interleaved_list(t_chain_data *x);

// Attribute Setters
void chain_data_set_site_name(t_chain_data *x, void *attr, long argc, t_atom *argv);

// File readers / writers
void chain_data_openfile(t_chain_data *x, char *filename, short path);
void chain_data_doread(t_chain_data *x, t_symbol *s);
void chain_data_dowrite(t_chain_data *x, t_symbol *s);
void chain_data_writefile(t_chain_data *x, char *filename, short path);

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
    class_addmethod(c, (method)chain_data_clear, "clear", 0);

    class_addmethod(c, (method)chain_data_read, "read", A_DEFSYM, 0);
    class_addmethod(c, (method)chain_data_write, "write", A_DEFSYM, 0);
   
    CLASS_ATTR_SYM(c, "savefile", 0L, t_chain_data, s_savefile);
    CLASS_ATTR_LONG(c, "autosave", 0L, t_chain_data, s_autosave);
    CLASS_ATTR_LONG(c, "normalize", 0L, t_chain_data, s_normalize);
    CLASS_ATTR_DOUBLE(c, "interval", 0, t_chain_data, s_interval);
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

    x->s_interval = 1.0;
    x->s_normalize = 0;
    x->s_savefile = NULL;

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

    // Load from savefile if listed
    if (x->s_savefile){
        chain_data_doread(x, x->s_savefile);
    }
    
    return x;
}

void chain_data_free(t_chain_data *x)
{
    chain_worker_release_site((t_chain_worker *)x);
    chain_data_clear(x);
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

void chain_data_read(t_chain_data *x, t_symbol *s)
{
    defer(x, (method)chain_data_doread, s, 0, NULL);
}

void chain_data_write(t_chain_data *x, t_symbol *s)
{
    defer(x, (method)chain_data_dowrite, s, 0, NULL);
}

void chain_data_doread(t_chain_data *x, t_symbol *s)
{
    long filetype = 'TEXT', outtype;
    char filename[512];
    short path;

    if (s == gensym("")){
        if (open_dialog(filename, &path, &outtype, &filetype, 1)){
            return;
        }
    } else {
        strcpy(filename, s->s_name);
        if (locatefile_extended(filename, &path, &outtype, &filetype, 1)){
            // chain_error("%s: not found", s->s_name);
            // CACHE MISS
            outlet_anything(x->s_outlet2, gensym("read_miss"), 0L, NULL);
            return;
        }
    }
    chain_data_openfile(x, filename, path);
    outlet_anything(x->s_outlet2, gensym("read_done"), 0L, NULL);
}

void chain_data_openfile(t_chain_data *x, char *filename, short path){
    t_filehandle fh;
    long err;

    if (path_opensysfile(filename, path, &fh, READ_PERM)){
        chain_error("Error opening: %s", filename);
        return;
    }

    long num_bytes = 1024 * 1024;
    char buf[1024 * 1024];
    err = sysfile_read(fh, &num_bytes, buf);

    long num_tokens = 1;
    for (long i=0; i < num_bytes; i++){
        if (buf[i] == ' '){
            num_tokens++;
        }
    }

    long num_samples = (num_tokens - 3) / 2;



    // Get metric name token
    char *token;
    char *savptr = NULL;
    token = strtok_r(buf, " ", &savptr);
    if (!token){
        chain_error("No metric_name in file");
        sysfile_close(fh);
        return;
    }
    x->s_metric_name = gensym(token);


    // Get start
    token = strtok_r(NULL, " ", &savptr);
    if (!token){
        chain_error("No start time in file");
        sysfile_close(fh);
        return;
    }
    x->s_start = strtol(token, NULL, 10);

    // Get end
    token = strtok_r(NULL, " ", &savptr);
    if (!token){
        chain_error("No end time in file");
        sysfile_close(fh);
        return;
    }
    x->s_end = strtol(token, NULL, 10);


    chain_data_clear(x);

    x->s_values = malloc(num_samples * sizeof(*(x->s_values)));
    x->s_offsets = malloc(num_samples * sizeof(*(x->s_offsets)));

    // Get the rest
    for (long i=0; i < num_samples; i++){
        // Get offset
        token = strtok_r(NULL, " ", &savptr);
        double offset = strtod(token, NULL);
        token = strtok_r(NULL, " ", &savptr);
        double value = strtod(token, NULL);
        *(x->s_values + i) = value;
        *(x->s_offsets + i) = offset;
    }

    x->s_num_samples = num_samples;

    sysfile_close(fh);
}

void chain_data_dowrite(t_chain_data *x, t_symbol *s)
{
    if (!x->s_values || !x->s_offsets || !x->s_num_samples){
        // Fail silently
        outlet_anything(x->s_outlet2, gensym("write_miss"), 0L, NULL);
        return;
    }

    char filename[512];
    short path;

    if (s == gensym("")){
        if (saveas_dialog(filename, &path, 0)){
            return;
        }
    } else {
        strcpy(filename, s->s_name);
        path = path_getdefault();
    }
    chain_data_writefile(x, filename, path);
    outlet_anything(x->s_outlet2, gensym("write_done"), 0L, NULL);
}

void chain_data_writefile(t_chain_data *x, char *filename, short path)
{
    long message_size = (x->s_num_samples * (44)) + (3 * 20);
    char buf[message_size];
    
    // Write Header
    char header[128];
    sprintf(header, "%s %ld %ld", x->s_metric_name->s_name, x->s_start, x->s_end);
    strncat(buf, header, 128);
    
    // Write Values
    for (long i=0; i < x->s_num_samples; i++){
        char pair[44];
        sprintf(pair, " %lf %lf", *(x->s_offsets + i), *(x->s_values + i));
        strncat(buf, pair, 44);
    }

    long err;
    t_filehandle fh;
    t_ptr_size size = strlen(buf);

    err = path_createsysfile(filename, path, 'TEXT', &fh);
    if (err) {chain_error("Error creating file: %s", filename);}
    err = sysfile_write(fh, &size, (void *)buf);
    if (err) {chain_error("Error writing to file: %s", filename);}
    
    sysfile_close(fh);
}

void chain_data_output_resampled_list(t_chain_data *x)
{
    if (!x->s_values || !x->s_offsets || !x->s_num_samples){
        // Fail silently
        chain_error("No data saved in chain.data");
        return;
    }

    long normalize = x->s_normalize;
    double interval = x->s_interval;
    long num_samples = x->s_num_samples;
    long duration = x->s_end - x->s_start;

    // if interval == 0, return all data without resampling to periodic intervals
    if (interval == 0.0){
        chain_error("Chain.data behavior for interval=0.0 is not yet implemented");
        return;
    }

    long ac = ceil(duration / interval);
    t_atom av[ac];

    // Initialize first before point at time=0.0 to values[0] regardless of offsets[0]
    double offset_before_interval = 0.0;
    double value_before_interval = x->s_values[0];

    double max = x->s_values[0];
    double min = x->s_values[0];

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
            break;
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
                value_before_interval = x->s_values[after_idx];
                after_idx++;
                offset_after_interval = x->s_offsets[after_idx];
                value_after_interval = x->s_values[after_idx];
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
            interval_value = value_before_interval + end_point_difference * (interval_distance / end_point_distance);
        }

        if (x->s_normalize){
            if (interval_value > max){max = interval_value;}
            if (interval_value < min){min = interval_value;}
        }

        atom_setfloat(av+i, interval_value);
    }

    if (x->s_normalize){
        for (int i=0; i < ac; i++){
            double v = atom_getfloat(av+i);
            v = (v - min) / (max - min);
            atom_setfloat(av+i, v);
        }
    }

    outlet_anything(x->s_outlet, ps_list, ac, av);
}

void chain_data_output_resampled_interleaved_list(t_chain_data *x)
{
    if (!x->s_values || !x->s_offsets || !x->s_num_samples){
        // Fail silently
        // chain_debug("No data saved in chain.data.");
        return;
    }
    // NOT IMPLMENTED !!!!!!!!!!!!!!!!!!!!!!!!!!
}

void chain_data_clear(t_chain_data *x){
    if (x->s_values){
        free(x->s_values);
        x->s_values = NULL;
    }
    if (x->s_offsets){
        free(x->s_offsets);
        x->s_offsets = NULL;
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

    // If autosave, save file
    if (x->s_autosave && x->s_savefile){
        chain_data_dowrite(x, x->s_savefile);
    }
}
