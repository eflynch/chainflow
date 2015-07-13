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

/***********************************************/
// Data Structures

typedef struct chain_map
{
    t_jbox s_box;
    long s_state;
    char s_mouseover;
    char s_mousedowninside;
    char s_trackmouse;
    t_jrgba s_outline;
    t_jrgba s_background;
    t_jrgba s_hilite;
    t_systhread s_systhread_find_site;
    int s_find_site_cancel;
    void *s_outlet;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
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

// Attributes
void chain_map_set_site_name(t_chain_map *x, void *attr, long argc, t_atom *argv);

// Setup + Teardown
void chain_map_release_site(t_chain_map *x);
void chain_map_find_site(t_chain_map *x);
void *chain_map_find_site_threadproc(t_chain_map *x);

// Notify
void chain_map_notify(t_chain_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data);


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
    ps_free = gensym("free");
    ps_willfree = gensym("willfree");
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

    CLASS_ATTR_SYM(c, "name", 0, t_chain_map, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_map_set_site_name);

    CLASS_ATTR_CHAR(c, "trackmouse", 0, t_chain_map, s_trackmouse);
    CLASS_ATTR_STYLE_LABEL(c, "trackmouse", 0, "onoff", "Track Mouse");
    CLASS_ATTR_SAVE(c, "trackmouse", 0);
    CLASS_ATTR_CATEGORY(c, "trackmouse", 0, "Behavior");

    CLASS_ATTR_RGBA(c, "bgcolor", 0, t_chain_map, s_background);
    CLASS_ATTR_STYLE_LABEL(c, "rectcolor", 0, "rgba", "Rectangle Color");
    CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c, "bgcolor", 0, "1. 1. 1. 1."); 
    CLASS_ATTR_STYLE_LABEL(c,"bgcolor",0,"rgba","Background Color");
    
    CLASS_ATTR_RGBA(c, "bordercolor", 0, t_chain_map, s_outline); 
    CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c, "bordercolor", 0, "0.5 0.5 0.5 1."); 
    CLASS_ATTR_STYLE_LABEL(c,"bordercolor",0,"rgba","Border Color");
    
    CLASS_ATTR_RGBA(c, "hilitecolor", 0, t_chain_map, s_hilite); 
    CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c, "hilitecolor", 0, "0.5 0.5 0.5 1."); 
    CLASS_ATTR_STYLE_LABEL(c,"hilitecolor",0,"rgba","Hilite Color");

    class_register(CLASS_BOX, c);
    s_chain_map_class = c;

    init_globals();
    return 0;
}

void *chain_map_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_map *x = (t_chain_map *)object_alloc(s_chain_map_class);

    if (x) {
        
        x->s_outlet = outlet_new(x, NULL);
        x->s_find_site_cancel = false;
        long flags;
        flags = 0
                | JBOX_DRAWFIRSTIN 
                | JBOX_NODRAWBOX
                | JBOX_DRAWINLAST
                | JBOX_TRANSPARENT  
        //      | JBOX_NOGROW
                | JBOX_GROWY
        //      | JBOX_GROWBOTH
        //      | JBOX_IGNORELOCKCLICK
        //      | JBOX_HILITE
        //      | JBOX_BACKGROUND
                | JBOX_DRAWBACKGROUND
        //      | JBOX_NOFLOATINSPECTOR
        //      | JBOX_TEXTFIELD
                ;

        jbox_new(&x->s_box, flags, argc, argv);
        x->s_box.b_firstin = (t_object*) x;

        x->s_mousedowninside = x->s_mouseover = x->s_state = 0;

        long attrstart = attr_args_offset(argc, argv);
        t_symbol *site_name = NULL;
        if (attrstart && atom_gettype(argv) == A_SYM)
            site_name = atom_getsym(argv);
        attr_args_process(x, argc, argv);
        if (!x->s_site_name) {
            if (site_name)
                object_attr_setsym(x, ps_name, site_name);
        }

        jbox_ready(&x->s_box);
    }
    return x;
}

void chain_map_free(t_chain_map *x)
{
    if (x->s_dictionary){
        dictobj_release(x->s_dictionary);
    }

    jbox_free(&x->s_box);

    unsigned int ret;
    if (x->s_systhread_find_site){
        x->s_find_site_cancel = true;
        systhread_join(x->s_systhread_find_site, &ret);
        x->s_systhread_find_site = NULL;
    }
}

void chain_map_release_site(t_chain_map *x)
{
    if (x->s_systhread_find_site){
        x->s_find_site_cancel = true;
        unsigned int ret;
        systhread_join(x->s_systhread_find_site, &ret);
        x->s_systhread_find_site = NULL;
    }

    if (x->s_dictionary){
        dictobj_release(x->s_dictionary);
        x->s_dictionary = NULL;
    }

    object_unsubscribe(ps_maxchain, x->s_site_name, NULL, x); 
}

void chain_map_find_site(t_chain_map *x)
{
    // Make sure it is released
    chain_map_release_site(x);

    // Find a site
    systhread_create((method) chain_map_find_site_threadproc, x,
                     0, 0, 0, &x->s_systhread_find_site);
}

void *chain_map_find_site_threadproc(t_chain_map *x)
{
    while(1){
        if (x->s_find_site_cancel){
            x->s_find_site_cancel = false;
            break;
        }

        x->s_dictionary = dictobj_findregistered_retain(x->s_site_name);
        if (x->s_dictionary){
            // Cache pointer to database
            dictionary_getobject(x->s_dictionary, ps_db, &x->s_db);

            // Attach object to site
            object_subscribe(ps_maxchain, x->s_site_name, NULL, x);
            break;
        }

        systhread_sleep(1000);
    }
    

    x->s_systhread_find_site = NULL;
    systhread_exit(0);
    return NULL;
}

void chain_map_notify(t_chain_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if (s == x->s_site_name && msg == ps_free){
        chain_map_find_site(x);
    } 

    if (s == x->s_site_name && msg == ps_willfree){
        chain_map_release_site(x);
    }
}

void chain_map_set_site_name(t_chain_map *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || x->s_site_name!=site_name){
        x->s_site_name = site_name;

        chain_map_find_site(x);
    }
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

    // paint outline
    jgraphics_set_source_jrgba(g, &x->s_outline);
    jgraphics_set_line_width(g, 1.0);
    jgraphics_rectangle(g, 0., 0., rect.width, rect.height);
    jgraphics_stroke(g);

    // paint "inner highlight" to indicate mouseover
    if (x->s_mouseover && !x->s_mousedowninside) {
        jgraphics_set_source_jrgba(g, &x->s_hilite);
        jgraphics_set_line_width(g, 1.);
        jgraphics_rectangle(g, 1., 1., rect.width - 2, rect.height - 2);
        jgraphics_stroke(g);
    }
    if (x->s_mousedowninside && !x->s_state) {
        jgraphics_set_source_jrgba(g, &x->s_hilite);
        jgraphics_rectangle(g, 1., 1., rect.width - 2, rect.height - 2);
        jgraphics_fill(g);
    }

    // paint "check" (actually a filled rectangle if state is on
    if (x->s_state) {
        t_jrgba col;

        jbox_get_color((t_object *)x, &col);
        jgraphics_set_source_jrgba(g, &col);
        if (x->s_mousedowninside)
            jgraphics_rectangle(g, 3., 3., rect.width - 6, rect.height - 6);
        else
            jgraphics_rectangle(g, 4., 4., rect.width - 8, rect.height - 8);
        jgraphics_fill(g);
    }
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
        x->s_state = !x->s_state;
        chain_map_bang(x);
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
    jbox_redraw((t_jbox *)x);
}

void chain_map_mousemove(t_chain_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    // nothing to do here, but could track mouse position inside object
}
