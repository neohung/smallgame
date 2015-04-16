#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#endif


#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <stdio.h>

static GdkPixmap *neo;

GtkWidget *window_root, *magic_map;
GtkBuilder *dialog_xml, *window_xml;
GdkGC *mapgc;

typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char uint8;
typedef signed char sint8;

typedef struct PixmapInfo {
    void        *icon_mask, *icon_image;
    uint16      icon_width, icon_height;
    void        *map_mask, *map_image;
    uint16      map_width, map_height;
    void        *fog_image;
    uint16      smooth_face;
#ifdef HAVE_OPENGL
    //GLuint      map_texture, fog_texture;
#endif
} PixmapInfo;

#define MAXPIXMAPNUM 10000
static PixmapInfo *pixmaps[MAXPIXMAPNUM];

GdkColor root_color[13];
const char *const colorname[13] = {
    "Black",                /* 0  */
    "White",                /* 1  */
    "Navy",                 /* 2  */
    "Red",                  /* 3  */
    "Orange",               /* 4  */
    "DodgerBlue",           /* 5  */
    "DarkOrange2",          /* 6  */
    "SeaGreen",             /* 7  */
    "DarkSeaGreen",         /* 8  *//* Used for window background color */
    "Grey50",               /* 9  */
    "Sienna",               /* 10 */
    "Gold",                 /* 11 */
    "Khaki"                 /* 12 */
};


struct MapCellLayer {
    sint16 face;
    sint8 size_x;
    sint8 size_y;

    /* Link into animation information.
     * animation is provided to us from the server in the map2 command.
     * animation_speed is also provided.
     * animation_left is how many ticks until animation changes - generated
     *  by client.
     * animation_phase is current phase.
     */
    sint16  animation;
    uint8   animation_speed;
    uint8   animation_left;
    uint8   animation_phase;
};

struct MapCell
{
    struct MapCellLayer heads[3];
    struct MapCellLayer tails[3];
    uint16 smooth[3];
    uint8 darkness;         /* darkness: 0=fully illuminated, 255=pitch black */
    uint8 need_update:1;    /* set if tile should be redrawn */
    uint8 have_darkness:1;  /* set if darkness information was set */
    uint8 need_resmooth:1;  /* same has need update but for smoothing only */
    uint8 cleared:1;        /* If set, this is a fog cell. */
};

struct Map
{
    int x;
    int y;
    struct MapCell **cells;
};

struct Map the_map;


void error_dialog(char *description, char *information) {
    GtkWidget *dialog;

    gtk_init(NULL, NULL);
    dialog =
    gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
                               description);
    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),
            "%s", information);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void keyfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
	switch(event->keyval){
		case 65307:
			//ESC
			exit(0);
			break; 
		case 65362:
		//UP
		break; 
		case 65364:
		//Down
		break; 
		case 65361:
		//Left
		break; 
		case 65363:
		//Right
		break; 
	}
}

void keyrelfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
printf("keyrelfunc\n");
}

void on_window_destroy_event(GtkObject *object, gpointer user_data) {
#ifdef WIN32
    //script_killall();
#endif

    printf("main.c::client_exit");
    exit(0);
}

static void init_ui() {
	int i;
	GError *error = NULL;
	 dialog_xml = gtk_builder_new();
	 if (!gtk_builder_add_from_file(dialog_xml, "dialogs.ui", &error)) {
        error_dialog("Couldn't load UI dialogs.", error->message);
        //g_warning("Couldn't load UI dialogs: %s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
    }

    window_xml = gtk_builder_new();

    //if (!gtk_builder_add_from_file(window_xml, "gtk-v1.ui", &error)) {
    if (!gtk_builder_add_from_file(window_xml, "neo2.glade", &error)) {
            error_dialog("Couldn't load window.", error->message);
            g_error_free(error);
            exit(EXIT_FAILURE);
    }

    window_root = GTK_WIDGET(gtk_builder_get_object(window_xml, "window_root"));

    g_signal_connect_swapped((gpointer) window_root, "key_press_event",
                             G_CALLBACK(keyfunc), GTK_OBJECT(window_root));
    g_signal_connect_swapped((gpointer) window_root, "key_release_event",
                             G_CALLBACK(keyrelfunc), GTK_OBJECT(window_root));
    g_signal_connect((gpointer) window_root, "destroy",
                     G_CALLBACK(on_window_destroy_event), NULL);

    //setup color
    for (i = 0; i < 13; i++) {
        if (!gdk_color_parse(colorname[i], &root_color[i])) {
            fprintf(stderr, "gdk_color_parse failed (%s)\n", colorname[i]);
        }
        if (!gdk_color_alloc(gtk_widget_get_colormap(window_root), &root_color[i])) {
            fprintf(stderr, "gdk_color_alloc failed\n");
        }
    }
 
}

int do_timeout() {
/*
    if (cpl.showmagic) {
        magic_map_flash_pos();
    }
    if (cpl.spells_updated) {
        update_spell_information();
    }
    if (!tick) {
        inventory_tick();
        mapdata_animation();
    }
    */
    return TRUE;
}


void event_loop() {
	printf("loop\n");
	gtk_timeout_add(10, (GtkFunction) do_timeout, NULL);
#ifdef WIN32
    //gtk_timeout_add(25, (GtkFunction) do_scriptout, NULL);
#endif
    gtk_main();
}

GtkWidget *map_drawing_area, *map_notebook;
static GdkBitmap *dark1, *dark2, *dark3;
static GdkPixmap *dark;

void draw_splash(void)
{
	#include "crossfiretitle.xpm"
	static GdkPixmap *splash;
	int x,y, w, h;
	GdkBitmap *aboutgdkmask;
	splash = gdk_pixmap_create_from_xpm_d(map_drawing_area->window,
                                                  &aboutgdkmask, NULL,
                                                  (gchar **)crossfiretitle_xpm);
	
	gdk_window_clear(map_drawing_area->window);
    gdk_drawable_get_size(splash, &w, &h);
    //gdk_gc_set_clip_mask(mapgc, NULL);
 	gdk_draw_pixmap(map_drawing_area->window, mapgc, splash, 0, 0,
                        0, 0, w, h);
 	//
    GdkGC   *darkgc;
    
    dark = gdk_pixmap_new(map_drawing_area->window, 32, 32, -1);
    gdk_draw_rectangle(dark, map_drawing_area->style->black_gc, TRUE, 0, 0, 32, 32);
    dark1 = gdk_pixmap_new(map_drawing_area->window, 32, 32, 1);
    dark2 = gdk_pixmap_new(map_drawing_area->window, 32, 32, 1);
    dark3 = gdk_pixmap_new(map_drawing_area->window, 32, 32, 1);
    darkgc = gdk_gc_new(dark1);
    gdk_gc_set_foreground(darkgc, &root_color[1]);

    for (x=0; x<32; x++) {
    	for (y=0; y<32; y++) {
    	        if ((x+y) % 2) {
                    gdk_draw_point(dark1, darkgc, x, y);
                }
                if ((x+y) %3) {
                    gdk_draw_point(dark2, darkgc, x, y);
                }
                if ((x+y) % 4) {
                    gdk_draw_point(dark3, darkgc, x, y);
                }
    	}
    }
    gdk_gc_unref(darkgc);
 
}

static void draw_pixmap(int srcx, int srcy, int dstx, int dsty, int clipx, int clipy,
                        void *mask, void *image, int sizex, int sizey)
{
    gdk_gc_set_clip_mask(mapgc, mask);
    gdk_gc_set_clip_origin(mapgc, clipx, clipy);
    gdk_draw_pixmap(map_drawing_area->window, mapgc, image, srcx, srcy, dstx, dsty, sizex, sizey);
}


void init_image_cache_data(void)
{
    
	int i;
    GtkStyle *style;
	#include "question.xpm"
	style = gtk_widget_get_style(window_root);
	pixmaps[0] = malloc(sizeof(PixmapInfo));
    pixmaps[0]->icon_image = gdk_pixmap_create_from_xpm_d(window_root->window,
                             (GdkBitmap**)&pixmaps[0]->icon_mask,
                             &style->bg[GTK_STATE_NORMAL],
                             (gchar **)question_xpm);
    pixmaps[0]->map_image =  pixmaps[0]->icon_image;
    pixmaps[0]->fog_image =  pixmaps[0]->icon_image;
    pixmaps[0]->map_mask =  pixmaps[0]->icon_mask;
    pixmaps[0]->icon_width = pixmaps[0]->icon_height = pixmaps[0]->map_width = pixmaps[0]->map_height = 32;
    pixmaps[0]->smooth_face = 0;

   	for (i=1; i<MAXPIXMAPNUM; i++)  {
        pixmaps[i] = pixmaps[0];
    }
/*
    #include "crossfiretitle.xpm"
    GdkBitmap *neotask;
    neo = gdk_pixmap_create_from_xpm_d(map_drawing_area->window, &neotask, NULL, (gchar **)crossfiretitle_xpm);
*/

    //init_common_cache_data();
}
//int create_and_rescale_image_from_data(Cache_Entry *ce, int pixmap_num, uint8 *rgba_data, int width, int height)
//{
//	return 0;
//}

static void display_mapcell(int ax, int ay, int mx, int my)
{
	gdk_draw_rectangle(map_drawing_area->window, map_drawing_area->style->black_gc, TRUE, ax*32, ay*32, 32, 32);
	#include "question.xpm"
	GdkBitmap *aboutgdkmask;
	GtkStyle *style;
	style = gtk_widget_get_style(window_root);
	  int face = 0;
	  //int face = mapdata_face(ax, ay, layer);
      draw_pixmap(
                  0, 0,
                  ax*32 + 0, ay*32 + 0,
                  ax*32+32-pixmaps[face]->map_width,
                  ay*32+32-pixmaps[face]->map_height,
                  pixmaps[face]->map_mask, pixmaps[face]->map_image,
                  pixmaps[face]->map_width>32?32:pixmaps[face]->map_width,
                  pixmaps[face]->map_height>32?32:pixmaps[face]->map_height);
		
}

void gtk_draw_map(int redraw)
{
	  int x,y;
	    printf("gtk_draw_map\n");
	  for(x = 0; x < 11; x++) {
        for(y = 0; y < 11; y++) {
  			display_mapcell(x, y, 0, 0);
  		}
  	  }
}

void draw_map(int redraw)
{
	//draw_splash();
	gtk_draw_map(redraw);
	 #include "question.xpm"
	GdkBitmap *aboutgdkmask;
	GtkStyle *style;
	style = gtk_widget_get_style(window_root);
	//pixmaps[0]->map_image = gdk_pixmap_create_from_xpm_d(map_drawing_area->window,
    //                                              &aboutgdkmask, &style->bg[GTK_STATE_NORMAL],
     //                                             (gchar **)question_xpm);
    // gdk_gc_set_clip_mask(mapgc, pixmaps[0]->map_mask);
    // gdk_gc_set_clip_origin(mapgc, 32, 32);
	//gdk_draw_pixmap(map_drawing_area->window, mapgc, pixmaps[0]->map_image, 0, 0, 0, 0, 32, 32);

	/*
#ifdef HAVE_SDL
    if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_SDL) {
        sdl_gen_map(redraw);
    } else
#endif
#ifdef HAVE_OPENGL
        if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_OPENGL) {
            opengl_gen_map(redraw);
        } else
#endif
            if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_PIXMAP) {
                if (cpl.input_state == Metaserver_Select) {
                    draw_splash();
                } else {
                    gtk_draw_map(redraw);
                }
            }
            */
}

gboolean
on_drawingarea_map_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
    draw_map(TRUE);
    printf("on_drawingarea_map_expose_event\n");
    return FALSE;
}

//Mouse button press event
gboolean
on_drawingarea_map_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printf("on_drawingarea_map_button_press_event\n");
	int dx, dy, i, x, y, xmidl, xmidh, ymidl, ymidh;
    x=(int)event->x;
    y=(int)event->y;
    switch (event->button) {
    case 1:
        //look_at(dx,dy);
        break;
    case 2:
    case 3:
	
        break;
    }
	return FALSE;
}

gboolean
on_drawingarea_map_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
	printf("on_drawingarea_map_configure_event\n");
	return FALSE;
}

void map_init(GtkWidget *window_root) {
    map_drawing_area = GTK_WIDGET(gtk_builder_get_object(window_xml,
                "drawingarea_map"));
    g_signal_connect ((gpointer) map_drawing_area, "expose_event",
                      G_CALLBACK (on_drawingarea_map_expose_event), NULL);
    g_signal_connect ((gpointer) map_drawing_area, "button_press_event",
                      G_CALLBACK (on_drawingarea_map_button_press_event), NULL);
    g_signal_connect ((gpointer) map_drawing_area, "configure_event",
                      G_CALLBACK (on_drawingarea_map_configure_event), NULL);
    //init mapgc
    mapgc = gdk_gc_new(map_drawing_area->window);
    gtk_widget_show(map_drawing_area);
    gtk_widget_add_events (map_drawing_area, GDK_BUTTON_PRESS_MASK);

}


char* server=NULL;

int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);
 	init_ui();   
    gtk_widget_show(window_root);
    map_init(window_root);
    init_image_cache_data();
    //draw_splash();
    while (1) {
    	if (server == NULL) {
            //draw_splash();
            //server = "NEOSERVER";
        }
    	event_loop();
	}

  exit(EXIT_SUCCESS);
}