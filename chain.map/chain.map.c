/**
    @file
    chain.map - a simple chain_map object
 
    @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"
#include "jgraphics.h"
#include "jpatcher_api.h"

#include "messages.h"
#include "chainworker.h"
#include "queries.h"

/***********************************************/
// Data Structures

typedef struct chain_map
{
    t_chain_worker s_worker;
    int s_loaded;
    double s_scale;
    t_symbol *s_hover_device;
    t_symbol *s_selected_device;
    double s_borderwidth;
    double s_mousex;
    double s_mousey;
    char s_mouseover;
    char s_mousedowninside;
    char s_trackmouse;
    t_jrgba s_outline;
    t_jrgba s_background;
    t_jrgba s_devicehover;
    t_jrgba s_deviceselected;
    t_jrgba s_devicecolor;
    t_jrgba s_listenercolor;
    t_jfont *s_font;
    t_jtextlayout *s_layout;
    float s_pos_x;
    float s_pos_y;
    float s_pos_z;
    float s_ang_azi;
    float s_ang_ele;
    void *s_outlet;
} t_chain_map;

/************************************************************/
// Prototypes


// Create + Destroy
void *chain_map_new(t_symbol *s, long argc, t_atom *argv);
void chain_map_free(t_chain_map *x);

// Methods
void chain_map_int(t_chain_map *x, long n);
void chain_map_bang(t_chain_map *x);
void chain_map_paint(t_chain_map *x, t_object *view);
void chain_map_getdrawparams(t_chain_map *x, t_object *patcherview, t_jboxdrawparams *params);
void chain_map_mousedown(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);
void chain_map_mousedrag(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);
void chain_map_mouseup(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);
void chain_map_mouseenter(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);
void chain_map_mouseleave(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);
void chain_map_mousemove(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers);

// Attribute Setters
void chain_map_set_pos_x(t_chain_map *x, void *attr, long argc, t_atom *argv);
void chain_map_set_pos_y(t_chain_map *x, void *attr, long argc, t_atom *argv);
void chain_map_set_pos_z(t_chain_map *x, void *attr, long argc, t_atom *argv);
void chain_map_set_ang_azi(t_chain_map *x, void *attr, long argc, t_atom *argv);
void chain_map_set_ang_ele(t_chain_map *x, void *attr, long argc, t_atom *argv);
void chain_map_pos(t_chain_map *x, t_symbol *s, long argc, t_atom *argv);
void chain_map_ang(t_chain_map *x, t_symbol *s, long argc, t_atom *argv);

// Notify
t_max_err chain_map_notify(t_chain_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

// Drawing
void chain_map_paint_devices(t_chain_map *x, t_object *view, t_rect *rect);
void chain_map_paint_background(t_chain_map *x, t_object *view, t_rect *rect);
void chain_map_paint_circles(t_chain_map *x, t_object *view, t_rect *rect);
void chain_map_paint_position(t_chain_map *x, t_object *view, t_rect *rect);
void chain_map_paint_listener(t_chain_map *x, t_object *view, t_rect *rect);
void chain_map_setscale(t_chain_map *x, t_rect *rect);


/*************************************************************/
// Globals and Statics

static t_class *s_chain_map_class = NULL;

t_symbol *ps_url, *ps_name, *ps_db, *ps_devices, *ps_metrics;
t_symbol *ps_maxchain, *ps_free, *ps_willfree;

static void init_globals(void){
    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_db = gensym("db");
    ps_metrics = gensym("metrics");
    ps_devices = gensym("devices");
    ps_maxchain = gensym("maxchain");
}

/************************************************************/
// Class Definition and Live Cycle

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.map",
                  (method)chain_map_new,
                  (method)chain_map_free,
                  sizeof(t_chain_map),
                  (method)0L,
                  A_GIMME,
                  0L);
    c->c_flags |= CLASS_FLAG_NEWDICTIONARY;

    jbox_initclass(c, 0);

    class_addmethod(c, (method)chain_map_bang, "bang", 0);
    class_addmethod(c, (method)chain_map_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_map_paint, "paint", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mousedown, "mousedown", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mousedrag, "mousedrag", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mouseup, "mouseup", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mouseenter, "mouseenter", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mouseleave, "mouseleave", A_CANT, 0);
    class_addmethod(c, (method)chain_map_mousemove, "mousemove", A_CANT, 0);

    class_addmethod(c, (method)chain_map_notify, "notify", A_CANT, 0);

    class_addmethod(c, (method)chain_map_pos, "pos", A_GIMME, 0);
    class_addmethod(c, (method)chain_map_ang, "ang", A_GIMME, 0);

    CLASS_ATTR_FLOAT(c, "pos_x", 0, t_chain_map, s_pos_x);
    CLASS_ATTR_ACCESSORS(c, "pos_x", NULL, chain_map_set_pos_x);
    CLASS_ATTR_FLOAT(c, "pos_y", 0, t_chain_map, s_pos_y);
    CLASS_ATTR_ACCESSORS(c, "pos_y", NULL, chain_map_set_pos_y);
    CLASS_ATTR_FLOAT(c, "pos_z", 0, t_chain_map, s_pos_z);
    CLASS_ATTR_ACCESSORS(c, "pos_z", NULL, chain_map_set_pos_z);
    CLASS_ATTR_FLOAT(c, "ang_azi", 0, t_chain_map, s_ang_azi);
    CLASS_ATTR_ACCESSORS(c, "ang_azi", NULL, chain_map_set_ang_azi);
    CLASS_ATTR_FLOAT(c, "ang_ele", 0, t_chain_map, s_ang_ele);
    CLASS_ATTR_ACCESSORS(c, "ang_ele", NULL, chain_map_set_ang_ele);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_map, s_worker.s_site_name);

    CLASS_ATTR_CHAR(c, "trackmouse", 0, t_chain_map, s_trackmouse);
    CLASS_ATTR_STYLE_LABEL(c, "trackmouse", 0, "onoff", "Track Mouse");
    CLASS_ATTR_SAVE(c, "trackmouse", 0);
    CLASS_ATTR_CATEGORY(c, "trackmouse", 0, "Behavior");

    CLASS_ATTR_RGBA(c, "bgcolor", 0, t_chain_map, s_background);
    CLASS_ATTR_STYLE_LABEL(c,"bgcolor",0,"rgba","Background Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bgcolor", 0, "1. 1. 1. 1."); 
    
    CLASS_ATTR_RGBA(c, "outlinecolor", 0, t_chain_map, s_outline); 
    CLASS_ATTR_STYLE_LABEL(c,"outlinecolor",0,"rgba","Border Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "outlinecolor", 0, "0.0 0.0 0.0 1."); 
    
    CLASS_ATTR_RGBA(c, "devicecolor", 0, t_chain_map, s_devicecolor); 
    CLASS_ATTR_STYLE_LABEL(c,"devicecolor",0,"rgba","Device Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "devicecolor", 0, "0.91 0.49 0.21 1."); 

    CLASS_ATTR_RGBA(c, "listenercolor", 0, t_chain_map, s_listenercolor); 
    CLASS_ATTR_STYLE_LABEL(c,"listenercolor",0,"rgba","Listener Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "listenercolor", 0, "0.10 0.10 0.30 1."); 

    CLASS_ATTR_RGBA(c, "devicehover", 0, t_chain_map, s_devicehover); 
    CLASS_ATTR_STYLE_LABEL(c,"devicehover",0,"rgba","Hover Device Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "devicehover", 0, "0.3 0.3 0.3 0.9"); 

    CLASS_ATTR_RGBA(c, "deviceselected", 0, t_chain_map, s_deviceselected); 
    CLASS_ATTR_STYLE_LABEL(c,"deviceselected",0,"rgba","Selected Device Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "deviceselected", 0, "0.1 0.05 0.05 1."); 

    CLASS_ATTR_DEFAULT(c, "rect", 0, "0. 0. 256. 256.");

    class_register(CLASS_BOX, c);
    s_chain_map_class = c;

    init_globals();
    return 0;
}

void *chain_map_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_map *x = (t_chain_map *)object_alloc(s_chain_map_class);
    t_dictionary *d = NULL;

    if (!(d = object_dictionaryarg(argc, argv)))
        return NULL;

    if (x) {
        chain_worker_new_dict((t_chain_worker *)x, s, argc, argv);

        long flags;
        flags = 0
                | JBOX_DRAWFIRSTIN 
        //      | JBOX_NODRAWBOX
        //      | JBOX_DRAWINLAST
                | JBOX_TRANSPARENT  
        //      | JBOX_NOGROW
        //      | JBOX_GROWY
                | JBOX_GROWBOTH
        //      | JBOX_IGNORELOCKCLICK
        //      | JBOX_HILITE
        //      | JBOX_BACKGROUND
        //      | JBOX_DRAWBACKGROUND
        //      | JBOX_NOFLOATINSPECTOR
        //      | JBOX_TEXTFIELD
        ;

        jbox_new(&x->s_worker.s_box, flags, argc, argv);
        x->s_worker.s_box.b_firstin = (t_object*) x;

        x->s_mousedowninside = x->s_mouseover = false;
        x->s_borderwidth = 25.0;
        x->s_mousex = 0.0;
        x->s_mousey = 0.0;
        x->s_scale = 1.0;
        x->s_loaded = false;

        x->s_pos_x = x->s_pos_y = x->s_pos_z = x->s_ang_azi = x->s_ang_ele = 0.0;

        attr_dictionary_process((t_object *)x, d);

        x->s_font = jfont_create("Helvetica", JGRAPHICS_FONT_SLANT_NORMAL,
                                 JGRAPHICS_FONT_WEIGHT_NORMAL, 8.0);
        x->s_layout = jtextlayout_create();

        jbox_ready(&x->s_worker.s_box);

        x->s_outlet = outlet_new(x, NULL);
    }
    return x;
}

void chain_map_free(t_chain_map *x)
{
    chain_worker_release_site((t_chain_worker *)x);

    if (x->s_layout){
        jtextlayout_destroy(x->s_layout);
        jfont_destroy(x->s_font);
    }

    jbox_free(&x->s_worker.s_box);
}


t_max_err chain_map_notify(t_chain_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);

    if (s == x->s_worker.s_site_name){
        if (msg == gensym("siteload")){
            x->s_loaded = true;
            jbox_invalidate_layer((t_object *)x, NULL, gensym("device_layer"));
            jbox_redraw((t_jbox *)x);
        }
    }

    if (msg == gensym("attr_modified")){
        t_symbol *name = (t_symbol *)object_method((t_object *)data, gensym("getname"));
        if (name == gensym("bgcolor"))
            jbox_invalidate_layer((t_object *)x, NULL, gensym("background_layer"));
        if (name == gensym("devicecolor"))
            jbox_invalidate_layer((t_object *)x, NULL, gensym("device_layer"));
        if (name == gensym("deviceselected") || name == gensym("devicehover"))
            jbox_invalidate_layer((t_object *)x, NULL, gensym("circle_layer"));
    }
    return jbox_notify((t_jbox *)x, s, msg, sender, data);
}

void chain_map_pos(t_chain_map *x, t_symbol *s, long argc, t_atom *argv){
    if (argc != 3){
        chain_error("chain.zone: pos got %ld args, expected 3.", argc);
        return;
    }
    float f_x = atom_getfloat(argv);
    float f_y = atom_getfloat(argv+1);
    float f_z = atom_getfloat(argv+2);
    x->s_pos_x = f_x;
    x->s_pos_y = f_y;
    x->s_pos_z = f_z;

    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}

void chain_map_ang(t_chain_map *x, t_symbol *s, long argc, t_atom *argv){
    if (argc != 2){
        chain_error("chain.zone: ang got %ld args, expected 2.", argc);
        return;
    }
    float f_azi = atom_getfloat(argv);
    float f_ele = atom_getfloat(argv+1);
    x->s_ang_azi = f_azi;
    x->s_ang_ele = f_ele;

    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}

void chain_map_set_pos_x(t_chain_map *x, void *attr, long argc, t_atom *argv){
    float f_x = atom_getfloat(argv);
    x->s_pos_x = f_x;
    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}

void chain_map_set_pos_y(t_chain_map *x, void *attr, long argc, t_atom *argv){
    float f_y = atom_getfloat(argv);
    x->s_pos_y = f_y;
    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}


void chain_map_set_pos_z(t_chain_map *x, void *attr, long argc, t_atom *argv){
    float f_z = atom_getfloat(argv);
    x->s_pos_z = f_z;
    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}

void chain_map_set_ang_azi(t_chain_map *x, void *attr, long argc, t_atom *argv){
    float f_azi = atom_getfloat(argv);
    x->s_ang_azi = f_azi;
    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}


void chain_map_set_ang_ele(t_chain_map *x, void *attr, long argc, t_atom *argv){
    float f_ele = atom_getfloat(argv);
    x->s_ang_ele = f_ele;
    jbox_invalidate_layer((t_object *)x, NULL, gensym("listener_layer"));
    jbox_redraw((t_jbox *)x);
}

void chain_map_int(t_chain_map *x, long n)
{
}

void chain_map_bang(t_chain_map *x)
{
}

void chain_map_paint(t_chain_map *x, t_object *view)
{
    t_rect rect;
    t_jgraphics *g;

    g = (t_jgraphics*) patcherview_get_jgraphics(view);
    jbox_get_rect_for_view((t_object *)x, view, &rect);

    // paint background
    chain_map_paint_background(x, view, &rect);

    // paint position information
    if (x->s_worker.s_db && x->s_mouseover)
        chain_map_paint_position(x, view, &rect);

    
    // paint devices
    if (x->s_loaded){
        chain_map_paint_devices(x, view, &rect);
        chain_map_paint_listener(x, view, &rect);
        chain_map_paint_circles(x, view, &rect);
    }
}

void chain_map_paint_position(t_chain_map *x, t_object *view, t_rect *rect){
    t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("position_layer"),
                                      rect->width, rect->height);

    if (g) {
        char str[30];
        sprintf(str, "%.2lf, %.2lf", x->s_mousex, x->s_mousey);
        jtextlayout_set(x->s_layout, str, x->s_font, rect->width - 100, rect->height - 16, 100, 16, JGRAPHICS_TEXT_JUSTIFICATION_CENTERED, 0L);
        jtextlayout_draw(x->s_layout, g);


        jbox_end_layer((t_object *)x, view, gensym("position_layer"));
    }

    jbox_paint_layer((t_object *)x, view, gensym("position_layer"), 0.0, 0.0);
}

void chain_map_paint_listener(t_chain_map *x, t_object *view, t_rect *rect){
   t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("listener_layer"),
                                      rect->width, rect->height);
    if (g) {
        double x_min, z_min;
        dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
        dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);

        chain_map_setscale(x, rect);
        float d_x = (x->s_pos_x - x_min) * x->s_scale + x->s_borderwidth;
        float d_z = (x->s_pos_z - z_min) * x->s_scale + x->s_borderwidth;

        if (d_z >= 0 && d_x >= 0 && d_x < rect->width && d_z < rect->height){
            jgraphics_set_source_jrgba(g, &x->s_listenercolor);
            jbox_get_rect_for_view((t_object *)x, view, rect);
            jgraphics_new_path(g);
            jgraphics_translate(g, d_x, rect->height - d_z);
            jgraphics_rotate(g, x->s_ang_azi);
            jgraphics_move_to(g, -4, 4);
            jgraphics_line_to(g, 4, 4);
            jgraphics_line_to(g, 0, -6);
            jgraphics_line_to(g, -4, 4);
            jgraphics_close_path(g);
            jgraphics_rotate(g, -x->s_ang_azi);
            jgraphics_translate(g, -d_x, -(rect->height - d_z));
            jgraphics_fill(g);
        }
        jbox_end_layer((t_object *)x, view, gensym("listener_layer"));
    }

    jbox_paint_layer((t_object *)x, view, gensym("listener_layer"), 0.0, 0.0);
}


void chain_map_paint_devices(t_chain_map *x, t_object *view, t_rect *rect){
   t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("device_layer"),
                                      rect->width, rect->height);
    if (g) {
        // query all devices
        long i;
        t_db_result *db_result = NULL;
        query_list_devices(x->s_worker.s_db, &db_result);
        long numrecords = db_result_numrecords(db_result);

        double x_min, z_min;
        dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
        dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);

        chain_map_setscale(x, rect);

        for (i=0; i<numrecords; i++){
            const char *name;
            name = db_result_string(db_result, i, 0);
            double d_x = db_result_float(db_result, i, 1);
            double d_z = db_result_float(db_result, i, 2);
            
            d_x = (d_x - x_min) * x->s_scale + x->s_borderwidth;
            d_z = (d_z - z_min) * x->s_scale + x->s_borderwidth;

            if (d_z < 0 || d_x < 0 || d_x > rect->width || d_z > rect->height){
                continue;
            }

            jgraphics_set_source_jrgba(g, &x->s_devicecolor);
            jbox_get_rect_for_view((t_object *)x, view, rect);
            jgraphics_arc(g, d_x, rect->height - d_z, 3, 0.0, 2.0 * 3.1415926535);
            jgraphics_fill(g);
        }
        jbox_end_layer((t_object *)x, view, gensym("device_layer"));
    }

    jbox_paint_layer((t_object *)x, view, gensym("device_layer"), 0.0, 0.0);
}

void chain_map_paint_circles(t_chain_map *x, t_object *view, t_rect *rect){
    t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("circle_layer"),
                                      rect->width, rect->height);

    if (g) {
        if (x->s_hover_device){
            t_db_result *db_result = NULL;
            query_get_device_location(x->s_worker.s_db, x->s_hover_device->s_name, &db_result);

            double x_min, z_min;
            dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
            dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);

            double d_x = db_result_float(db_result, 0, 3);
            double d_z = db_result_float(db_result, 0, 4);
            
            d_x = (d_x - x_min) * x->s_scale + x->s_borderwidth;
            d_z = (d_z - z_min) * x->s_scale + x->s_borderwidth;

            jgraphics_set_source_jrgba(g, &x->s_devicehover);
            jbox_get_rect_for_view((t_object *)x, view, rect);
            jgraphics_set_line_width(g, 2.0);
            jgraphics_arc(g, d_x, rect->height - d_z, 6, 0.0, 2.0 * 3.1415926535);
            jgraphics_stroke(g);

        }
        if (x->s_selected_device){
            t_db_result *db_result = NULL;
            query_get_device_location(x->s_worker.s_db, x->s_selected_device->s_name, &db_result);

            double x_min, z_min;
            dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
            dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);

            double d_x = db_result_float(db_result, 0, 3);
            double d_z = db_result_float(db_result, 0, 4);
            
            d_x = (d_x - x_min) * x->s_scale + x->s_borderwidth;
            d_z = (d_z - z_min) * x->s_scale + x->s_borderwidth;

            jgraphics_set_source_jrgba(g, &x->s_deviceselected);
            jbox_get_rect_for_view((t_object *)x, view, rect);
            jgraphics_set_line_width(g, 2.0);
            jgraphics_arc(g, d_x, rect->height - d_z, 6, 0.0, 2.0 * 3.1415926535);
            jgraphics_stroke(g);

        }
        jbox_end_layer((t_object *)x, view, gensym("circle_layer"));
    }

    jbox_paint_layer((t_object *)x, view, gensym("circle_layer"), 0.0, 0.0);
}

void chain_map_paint_background(t_chain_map *x, t_object *view, t_rect *rect){
    t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("background_layer"),
                                      rect->width, rect->height);
    if (g){
        jgraphics_set_source_jrgba(g, &x->s_background);
        jgraphics_rectangle_fill_fast(g, 0., 0., rect->width, rect->height);
        jbox_end_layer((t_object*)x, view, gensym("background_layer"));
    }
    jbox_paint_layer((t_object *)x, view, gensym("background_layer"), 0., 0.);
}

// mouse interaction
void chain_map_mousedown(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    x->s_mousedowninside = true;    // wouldn't get a click unless it was inside the box
    jbox_redraw((t_jbox *)x);
}

void chain_map_mousedrag(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_rect rect;
    
    // test to see if mouse is still inside the object
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
    
    // redraw if changed
    if (pt.x >= 0 && pt.x <= rect.width && pt.y >= 0 && pt.y <= rect.height) {
        if (!x->s_mousedowninside) {
            x->s_mousedowninside = true;
            jbox_redraw((t_jbox *)x);
        }
    } else {
        if (x->s_mousedowninside) {
            x->s_mousedowninside = false;
            jbox_redraw((t_jbox *)x);
        }
    }
}

void chain_map_mouseup(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    if (x->s_mousedowninside) {
        if (x->s_hover_device){
            x->s_selected_device = x->s_hover_device;
            t_atom av;
            atom_setsym(&av, x->s_selected_device);
            outlet_anything(x->s_outlet, gensym("device_name"), 1, &av);
        } else {
            x->s_selected_device = NULL;
        }
        jbox_invalidate_layer((t_object *)x, NULL, gensym("circle_layer"));
        x->s_mousedowninside = false;
        jbox_redraw((t_jbox *)x);
    }
}

void chain_map_mouseenter(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    x->s_mouseover = true;
    jbox_redraw((t_jbox *)x);
}

void chain_map_mouseleave(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    x->s_mouseover = false;
    if (x->s_hover_device){
        x->s_hover_device = NULL;
        jbox_invalidate_layer((t_object *)x, NULL, gensym("circle_layer"));
    }
    jbox_redraw((t_jbox *)x);
}

void chain_map_setscale(t_chain_map *x, t_rect *rect)
{
    // Get drawing scale
    double x_max, x_min, z_max, z_min;
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmax"), &x_max);
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmax"), &z_max);

    double x_scale = ((double)rect->width - 2 * x->s_borderwidth) / (x_max - x_min);
    double z_scale = ((double)rect->height - 2 * x->s_borderwidth) / (z_max - z_min);
    double scale;
    if (x_scale < z_scale){
        scale = x_scale;
    } else {
        scale = z_scale;
    }
    if (scale <= 0.0){
        scale = 1.0;
    }
    x->s_scale = scale;
}

void chain_map_mousemove(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    if (!x->s_loaded){
        return;
    }
    t_rect rect;
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);

    double x_min, z_min;
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("xmin"), &x_min);
    dictionary_getfloat(x->s_worker.s_dictionary, gensym("zmin"), &z_min);

    double d_x = (pt.x - x->s_borderwidth) / x->s_scale + x_min;
    double d_z = ((rect.height - pt.y) - x->s_borderwidth) / x->s_scale + z_min;
    double d_r = 6.0 / x->s_scale;

    x->s_mousex = d_x;
    x->s_mousey = d_z;

    // query devices near mouse
    t_db_result *db_result = NULL;
    query_list_devices_near_point(x->s_worker.s_db, d_x, d_z, d_r, &db_result);
    long numrecords = db_result_numrecords(db_result);
    if (numrecords){
        const char *name;
        name = db_result_string(db_result, 0, 0);
        t_symbol *name_sym = gensym(name);
        if (name_sym != x->s_hover_device){
            x->s_hover_device = gensym(name);
            jbox_invalidate_layer((t_object *)x, NULL, gensym("circle_layer"));
        }
    } else {
        if (x->s_hover_device){
            x->s_hover_device = NULL;
            jbox_invalidate_layer((t_object *)x, NULL, gensym("circle_layer"));
        }
    }
    jbox_invalidate_layer((t_object *)x, NULL, gensym("position_layer"));
    jbox_redraw((t_jbox *)x);
}
