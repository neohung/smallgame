#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

#ifndef WIN32
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* WIN32 */

#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define CLIENT_TYPES_H
#define HAVE_CURL_CURL_H

#ifdef HAVE_CURL_CURL_H
#include <curl/curl.h>
#include <curl/easy.h>
#endif

static GdkPixmap *neo;

GtkWidget *window_root, *magic_map;
GtkBuilder *dialog_xml, *window_xml;
GdkGC *mapgc;

typedef unsigned long uint64;
typedef signed long sint64;
typedef unsigned int uint32;
typedef signed int sint32;
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

typedef struct Cache_Entry {
    char    *filename;
    uint32  checksum;
    uint32  ispublic:1;
    void    *image_data;
    struct Cache_Entry  *next;
} Cache_Entry;

struct Image_Cache {
    char    *image_name;
    struct Cache_Entry  *cache_entry;
} image_cache[8192];

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

typedef struct PlayerPosition {
  int x;
  int y;
} PlayerPosition;
PlayerPosition pl_pos;

int width, height;

#define CLEAR_CELLS(x, y, len_y) \
do { \
    int clear_cells_i, j; \
    memset(&the_map.cells[(x)][(y)], 0, sizeof(the_map.cells[(x)][(y)])*(len_y)); \
    for (clear_cells_i = 0; clear_cells_i < (len_y); clear_cells_i++) \
    { \
        for (j=0; j < 3; j++) { \
            the_map.cells[(x)][(y)+clear_cells_i].heads[j].size_x = 1; \
            the_map.cells[(x)][(y)+clear_cells_i].heads[j].size_y = 1; \
        } \
    } \
} while(0)

sint16 mapdata_face(int x, int y, int layer)
{
    if (width <= 0) {
        return(0);
    }

    assert(0 <= x && x < width);
    assert(0 <= y && y < height);
    assert(0 <= layer && layer < 3);

    return(the_map.cells[pl_pos.x+x][pl_pos.y+y].heads[layer].face);
}

void mapdata_init(void)
{
    int x, y;
    int i;

    if (the_map.cells == NULL) {
        the_map.cells = malloc(
                            sizeof(*the_map.cells)*512+
                            sizeof(**the_map.cells)*512*512);
        if (the_map.cells == NULL) {
            printf("mapdata_init fail\n");
           // LOG(LOG_ERROR, "mapdata_init", "%s\n", "out of memory");
            exit(1);
        }

        /* Skip past the first row of pointers to rows and assign the
         * start of the actual map data
         */
        the_map.cells[0] = (struct MapCell *)((char *)the_map.cells+(sizeof(struct MapCell *)*512));
        for (i = 0; i < 512; i++) {
            the_map.cells[i] = the_map.cells[0]+i*512;
        }
        the_map.x = 512;
        the_map.y = 512;
    }
    width = 0;
    height = 0;
    pl_pos.x = 512/2-width/2;
    pl_pos.y = 512/2-height/2;

    for (x = 0; x < 512; x++) {
        CLEAR_CELLS(x, 0, 512);
    }   
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

static uint32 image_hash_name(char *str, int tablesize) {
    uint32 hash = 0;
    char *p;
    /* use the same one-at-a-time hash function the server now uses */
    for (p = str; *p != '\0' && *p != '.'; p++) {
        hash += *p;
        hash += hash << 10;
        hash ^= hash >>  6;
    }
    hash += hash <<  3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash % tablesize;
}

static Cache_Entry *image_add_hash(char *imagename, char *filename,
                                   uint32 checksum, uint32 ispublic) {

   Cache_Entry *new_entry;
   uint32  hash = image_hash_name(imagename, 8192), newhash;
   newhash = hash;
   while (image_cache[newhash].image_name != NULL &&
           strcmp(image_cache[newhash].image_name, imagename)) {
        newhash ++;
        if (newhash == 8192) {
            newhash = 0;
        }
        if (newhash == hash) {
            //means already find (hash ~ 8192) and (0 ~ hash)
            //No any image_cache[] is NULL 
            //No any image_cache[newhash].image_name match imagename
            //so return NULL
            return NULL;
        }
   }
   //Already got proper newhash
    if (!image_cache[newhash].image_name) {
        image_cache[newhash].image_name = strdup(imagename);
    }
    new_entry = malloc(sizeof(struct Cache_Entry));
    new_entry->filename = strdup(filename);
    new_entry->checksum = checksum;
    new_entry->ispublic = ispublic;
    new_entry->image_data = NULL;
    //put origin cache_entry point in new_entry->next
    new_entry->next = image_cache[newhash].cache_entry;
    image_cache[newhash].cache_entry = new_entry;
    return new_entry;
 
}

static void image_process_line(char *line, uint32 ispublic) {
    char imagename[256], filename[256];
    uint32 checksum;

    if (line[0] == '#') {
        return;    /* Ignore comments */
    }
    //Read from line: imagename checksum filename
    if (sscanf(line, "%s %u %s", imagename, &checksum, filename) == 3) {
        image_add_hash(imagename, filename, checksum, ispublic);
    } 

}

void init_common_cache_data(void) {
    FILE *fp;
    char    bmaps[256], inbuf[256];
    memset(image_cache, 0, 8192 * sizeof(struct Image_Cache));
    snprintf(bmaps, sizeof(bmaps), "bmaps.client");
    if ((fp = fopen(bmaps, "r")) != NULL) {
        while (fgets(inbuf, 256 - 1, fp) != NULL) {
            image_process_line(inbuf, 1);
        }
        fclose(fp);
    } 
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

    init_common_cache_data();
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

#define META_SERVER "crossfire.real-time.com"
#define META_PORT   13326
#define METASERVER  FALSE
#define MAXSOCKBUF (2+65535+1)
#define MAX_FACE_SETS   20

typedef struct SockList {
#ifdef CLIENT_TYPES_H                       /* Used by the client */
    int len;
    unsigned char *buf;
#else                                       /* Used by the server */
    size_t len;
    unsigned char buf[MAXSOCKBUF]; /* 2(size)+65535(content)+1(ending NULL) */
#endif
} SockList;

typedef struct ClientSocket {
    int fd;
    SockList    inbuf;
    int cs_version, sc_version;         /**< Server versions of these
                                         */
    int command_sent, command_received; /**< These are used for the newer
                                         *   'windowing' method of commands -
                                         *   number of last command sent,
                                         *   number of received confirmation
                                         */
    int command_time;                   /**< Time (in ms) players commands
                                         *   currently take to execute
                                         */
    char* servername;
} ClientSocket;

#define CS_NUM_SKILLS 50
#define MAX_BUF 256
#define MAX_SKILL CS_NUM_SKILLS
#define FREE_AND_CLEAR(xyz) { free(xyz); xyz=NULL; }
char *skill_names[MAX_SKILL];

typedef enum Input_State {
    Playing, Reply_One, Reply_Many, Configure_Keys, Command_Mode,
    Metaserver_Select
} Input_State;

typedef enum rangetype {
  range_bottom = -1, range_none = 0, range_bow = 1, range_magic = 2,
  range_wand = 3, range_rod = 4, range_scroll = 5, range_horn = 6,
  range_steal = 7,
  range_size = 8
} rangetype;

typedef struct item_struct {
    struct item_struct *next;   /* next item in inventory */
    struct item_struct *prev;   /* previous item in inventory */
    struct item_struct *env;    /* which items inventory is this item */
    struct item_struct *inv;    /* items inventory */
    char d_name[128];  /* item's full name w/o status information */
    char s_name[128];  /* item's singular name as sent to us */
    char p_name[128];  /* item's plural name as sent to us */
    char flags[128];   /* item's status information */
    sint32 tag;         /* item identifier (0 = free) */
    uint32 nrof;        /* number of items */
    float weight;       /* how much item weights */
    sint16 face;        /* index for face array */
    uint16 animation_id;    /* Index into animation array */
    uint8 anim_speed;       /* how often to animate */
    uint8 anim_state;       /* last face in sequence drawn */
    uint16 last_anim;       /* how many ticks have passed since we last animated */
    uint16 magical:1;       /* item is magical */
    uint16 cursed:1;        /* item is cursed */
    uint16 damned:1;        /* item is damned */
    uint16 unpaid:1;        /* item is unpaid */
    uint16 locked:1;        /* item is locked */
    uint16 applied:1;       /* item is applied */
    uint16 open:1;      /* container is open */
    uint16 was_open:1;      /* container was open */
    uint16 inv_updated:1;   /* item's inventory is updated, this is set
                   when item's inventory is modified, draw
                   routines can use this to redraw things */
    uint8 apply_type;       /* how item is applied (worn/wield/etc) */
    uint32 flagsval;        /* unmodified flags value as sent from the server*/
    uint16   type;      /* Item type for ordering */
} item;

typedef struct Spell_struct {
    struct Spell_struct *next;
    char name[256];                     /**< One length byte plus data       */
    char message[10000];                /**< This is huge, the packets can't
                                         *   be much bigger than this anyway */
    uint32 tag;                         /**< Unique ID number for a spell so
                                         *   updspell etc can operate on it. */
    uint16 level;                       /**< The casting level of the spell. */
    uint16 time;                        /**< Casting time in server ticks.   */
    uint16 sp;                          /**< Mana per cast; may be zero.     */
    uint16 grace;                       /**< Grace per cast; may be zero.    */
    uint16 dam;                         /**< Damage done by spell though the
                                         *   meaning is spell dependent and
                                         *   actual damage may depend on how
                                         *   the spell works.                */
    uint8 skill_number;                 /**< The index in the skill arrays,
                                         *   plus CS_STAT_SKILLINFO. 0: no
                                         *   skill used for cast.  See also:
                                         *   request_info skill_info         */
    char *skill;                        /**< Pointer to the skill name,
                                         *   derived from the skill number.  */
    uint32 path;                        /**< The bitmask of paths this spell
                                         *   belongs to.  See request_info
                                         *   spell_paths and stats about
                                         *   attunement, repulsion, etc.     */
    sint32 face;                        /**< A face ID that may be used to
                                         *   show a graphic representation
                                         *   of the spell.                   */
    uint8 usage;                        /**< Spellmon 2 data.  Values are:
                                         *   0: No argument required.
                                         *   1: Requires other spell name.
                                         *   2: Freeform string is optional.
                                         *   3: Freeform string is required. */
    char requirements[256];             /**< Spellmon 2 data. One length byte
                                         *   plus data. If the spell requires
                                         *   items to be cast, this is a list
                                         *   of req'd items. Comma-separated,
                                         *   number of items, singular names
                                         *   (like ingredients for alchemy). */
} Spell;

typedef struct Stat_struct {
    sint8 Str;                          /**< Strength */
    sint8 Dex;                          /**< Dexterity */
    sint8 Con;                          /**< Constitution */
    sint8 Wis;                          /**< Wisdom */
    sint8 Cha;                          /**< Charisma */
    sint8 Int;                          /**< Intelligence */
    sint8 Pow;                          /**< Power */
    sint8 wc;                           /**< Weapon Class */
    sint8 ac;                           /**< Armour Class */
    sint8 level;                        /**< Experience level */
    sint16 hp;                          /**< Hit Points */
    sint16 maxhp;                       /**< Maximum hit points */
    sint16 sp;                          /**< Spell points for casting spells */
    sint16 maxsp;                       /**< Maximum spell points. */
    sint16 grace;                       /**< Spell points for using prayers. */
    sint16 maxgrace;                    /**< Maximum spell points. */
    sint64 exp;                         /**< Experience.  Killers gain 1/10. */
    sint16 food;                        /**< Quantity food in stomach.
                                         *   0 = starved.
                                         */
    sint16 dam;                         /**< How much damage this object does
                                         *   for each hit
                                         */
    sint32 speed;                       /**< Speed (is displayed as a float) */
    sint32 weapon_sp;                   /**< Weapon speed (displayed in client
                                         *   as a float)
                                         */
    uint32 attuned;                     /**< Spell paths to which the player is
                                         *   attuned
                                         */
    uint32 repelled;                    /**< Spell paths to which the player is
                                         *   repelled
                                         */
    uint32 denied;                      /**< Spell paths denied to the player*/
    uint16 flags;                       /**< Contains fire on/run on flags */
    sint16 resists[30];                 /**< Resistant values */
    uint32 resist_change:1;             /**< Resistant value change flag */
    sint16 skill_level[MAX_SKILL];      /**< Level of known skills */
    sint64 skill_exp[MAX_SKILL];        /**< Experience points for skills */
    uint32 weight_limit;                /**< Carrying weight limit */
} Stats;


typedef struct Player_Struct {
    item        *ob;                    /**< Player object */
    item        *below;                 /**< Items below the player
                                         *   (pl.below->inv) */
    item        *container;             /**< open container */
    uint16      count_left;             /**< count for commands */
    Input_State input_state;            /**< What the input state is */
    char        last_command[MAX_BUF];  /**< Last command entered */
    char        input_text[MAX_BUF];    /**< keys typed (for long commands) */
    item        *ranges[range_size];    /**< Object that is used for that */
                                        /**< range type */
    uint8       ready_spell;            /**< Index to spell that is readied */
    char        spells[255][40];        /**< List of all the spells the */
                                        /**< player knows */
    Stats       stats;                  /**< Player stats */
    Spell       *spelldata;             /**< List of spells known */
    char        title[MAX_BUF];         /**< Title of character */
    char        range[MAX_BUF];         /**< Range attack chosen */
    uint32      spells_updated;         /**< Whether or not spells updated */
    uint32      fire_on:1;              /**< True if fire key is pressed */
    uint32      run_on:1;               /**< True if run key is on */
    uint32      meta_on:1;              /**< True if fire key is pressed */
    uint32      alt_on:1;               /**< True if fire key is pressed */
    uint32      no_echo:1;              /**< If TRUE, don't echo keystrokes */
    uint32      count;                  /**< Repeat count on command */
    uint16      mmapx, mmapy;           /**< size of magic map */
    uint16      pmapx, pmapy;           /**< Where the player is on the magic
                                         *   map */
    uint8       *magicmap;              /**< Magic map data */
    uint8       showmagic;              /**< If 0, show the normal map,
                                         *   otherwise show the magic map. */
    uint16      mapxres,mapyres;        /**< Resolution to draw on the magic
                                         *   map. Only used in client-specific
                                         *   code, so it should move there. */
    char        *name;                  /**< Name of PC, set and freed in account.c
                                         *   play_character() (using data returned
                                         *   from server to AccountPlayersCmd, via
                                         *   character_choose window,
                                         *   OR in
                                         *   send_create_player_to_server() when
                                         *   new character created. */
} Client_Player;

typedef struct FaceSets_struct {
    uint8   setnum;                     /**<  */
    uint8   fallback;                   /**<  */
    char    *prefix;                    /**<  */
    char    *fullname;                  /**<  */
    char    *size;                      /**<  */
    char    *extension;                 /**<  */
    char    *comment;                   /**<  */
} FaceSets;

typedef struct Face_Information_struct {
    uint8   faceset;
    char    *want_faceset;
    sint16  num_images;
    uint32  bmaps_checksum, old_bmaps_checksum;
    /**
     * Just for debugging/logging purposes.  This is cleared on each new
     * server connection.  This may not be 100% precise (as we increment
     * cache_hits when we find a suitable image to load - if the data is bad,
     * that would count as both a hit and miss.
     */
    sint16  cache_hits, cache_misses;
    uint8   have_faceset_info;          /**< Simple value to know if there is
                                         *   data in facesets[].
                                         */
    FaceSets    facesets[20];
} Face_Information;

struct RC_Choice {
    char *choice_name;                  /* name to respond, eg, race_choice_1 */
    char *choice_desc;                  /* Longer description of choice */
    int num_values;                     /* How many values we have */
    char **value_arch;    /* Array arch names */
    char **value_desc;    /* Array of description */
};

typedef struct Race_Class_Info {
    char    *arch_name;     /* Name of the archetype this correponds to */
    char    *public_name;   /* Public (human readadable) name */
    char    *description;   /* Description of the race/class */
    sint8   stat_adj[7];   /* Adjustment values */
    int     num_rc_choice;                  /* Size of following array */
    struct RC_Choice    *rc_choice;         /* array of choices */
} Race_Class_Info;

typedef struct Starting_Map_Info {
    char    *arch_name;     /* Name of archetype for this map */
    char    *public_name;   /* Name of the human readable name */
    char    *description;   /* Description of this map */
} Starting_Map_Info;

Client_Player cpl;
ClientSocket csocket;
Face_Information face_info;
char *meta_server=META_SERVER;
int meta_port=META_PORT;
int wantloginmethod;
int maxfd;
int serverloginmethod;

char *news=NULL, *motd=NULL, *rules=NULL;
int spellmon_level = 0;                 /**< Keeps track of what spellmon
                                         *   command is supported by the
                                         *   server. */
int num_races = 0;    /* Number of different races server has */
int used_races = 0;   /* How many races we have filled in */
int num_classes = 0;  /* Same as race data above, but for classes */
int used_classes = 0;
int stat_points = 0;    /* Number of stat points for new characters */
int stat_min = 0;       /* Minimum stat for new characters */
int stat_maximum = 0;   /* Maximum stat for new characters */
int starting_map_number = 0;   /* Number of starting maps */
Race_Class_Info *races=NULL, *classes=NULL;
Starting_Map_Info *starting_map_info = NULL;
/*
int meta_port=META_PORT, want_skill_exp=0,
    replyinfo_status=0, requestinfo_sent=0, replyinfo_last_face=0,
    maxfd,metaserver_on=METASERVER, metaserver2_on=METASERVER2,
          wantloginmethod=0, serverloginmethod=0;
*/
static void sigpipe_handler(int sig) {
    /* ignore that signal for now */
}

static void init_sockets() {
    /* Use the 'new' login method. */
    wantloginmethod = 2;
    csocket.inbuf.buf = malloc(MAXSOCKBUF);

#ifdef WIN32
    maxfd = 0; /* This is ignored on win32 platforms */

    /* This is required for sockets to be used under win32 */
    WORD Version = 0x0202;
    WSADATA wsaData;

    if (WSAStartup(Version, &wsaData) != 0) {
        //LOG(LOG_CRITICAL, "main.c::main", "Could not load winsock!");
        exit(1);
    }
#else /* def WIN32 */
    //For linux
    signal(SIGPIPE, sigpipe_handler);
    #ifdef HAVE_SYSCONF
        maxfd = sysconf(_SC_OPEN_MAX);
    #else
        maxfd = getdtablesize();
    #endif
#endif /* def WIN32 */
}

void free_all_race_class_info(Race_Class_Info *data, int num_entries)
{
    int i;
    /* Because we are going free the array storage itself, there is no reason
     * to clear the data[i].. values.
     */
    for (i=0; i<num_entries; i++) {
        int j;

        if (data[i].arch_name) {
            free(data[i].arch_name);
        }
        if (data[i].public_name) {
            free(data[i].public_name);
        }
        if (data[i].description) {
            free(data[i].description);
        }

        for (j=0; j<data[i].num_rc_choice; j++) {
            int k;

            for (k=0; k<data[i].rc_choice[j].num_values; k++) {
                free(data[i].rc_choice[j].value_arch[k]);
                free(data[i].rc_choice[j].value_desc[k]);
            }
            free(data[i].rc_choice[j].value_arch);
            free(data[i].rc_choice[j].value_desc);
            free(data[i].rc_choice[j].choice_name);
            free(data[i].rc_choice[j].choice_desc);
        }
    }

    free(data);
    data=NULL;
}


void reset_client_vars() {
    int i;

    cpl.count_left = 0;
    cpl.container = NULL;
    memset(&cpl.stats, 0, sizeof(Stats));
    cpl.stats.maxsp = 1;    /* avoid div by 0 errors */
    cpl.stats.maxhp = 1;    /* ditto */
    cpl.stats.maxgrace = 1; /* ditto */

    /* ditto - displayed weapon speed is weapon speed/speed */
    cpl.stats.speed = 1;
    cpl.input_text[0] = '\0';
    cpl.title[0] = '\0';
    cpl.range[0] = '\0';
    cpl.last_command[0] = '\0';

    for (i = 0; i < range_size; i++) {
        cpl.ranges[i] = NULL;
    }

    cpl.magicmap = NULL;
    cpl.showmagic = 0;

    csocket.command_sent = 0;
    csocket.command_received = 0;
    csocket.command_time = 0;

    face_info.faceset = 0;
    face_info.num_images = 0;
    /* Preserve the old one - this can be used to see if the next
     * server has the same name -> number mapping so that we don't
     * need to rebuild all the images.
     */
    face_info.old_bmaps_checksum = face_info.bmaps_checksum;
    face_info.bmaps_checksum = 0;
    face_info.cache_hits = 0;
    face_info.cache_misses = 0;
    face_info.have_faceset_info = 0;
    for (i = 0; i < MAX_FACE_SETS; i++) {
        FREE_AND_CLEAR(face_info.facesets[i].prefix);
        FREE_AND_CLEAR(face_info.facesets[i].fullname);
        face_info.facesets[i].fallback = 0;
        FREE_AND_CLEAR(face_info.facesets[i].size);
        FREE_AND_CLEAR(face_info.facesets[i].extension);
        FREE_AND_CLEAR(face_info.facesets[i].comment);
    }
    //
    //reset_player_data();
    for (i = 0; i < MAX_SKILL; i++) {
        cpl.stats.skill_exp[i] = 0;
        cpl.stats.skill_level[i] = 0;
    }
    //
    for (i = 0; i < MAX_SKILL; i++) {
        FREE_AND_CLEAR(skill_names[i]);
    }
    if (motd) {
        FREE_AND_CLEAR(motd);
    }
    if (news) {
        FREE_AND_CLEAR(news);
    }
    if (rules) {
        FREE_AND_CLEAR(rules);
    }
    if (races) {
        free_all_race_class_info(races, num_races);
        num_races = 0;
        used_races = 0;
        races = NULL;
    }
    if (classes) {
        free_all_race_class_info(classes, num_classes);
        num_classes = 0;
        used_classes = 0;
        classes = NULL;
    }
    stat_points = 0;
    stat_min = 0;
    stat_maximum = 0;

    serverloginmethod = 0;

}

#define MAX_METASERVER 100

#define MS_SMALL_BUF    60
#define MS_LARGE_BUF    512

typedef struct Meta_Info {
    char    ip_addr[MS_SMALL_BUF];  /* MS1 */
    char    hostname[MS_LARGE_BUF]; /* MS1 & MS2 */
    int     port;           /* MS2 - port server is on */
    char    html_comment[MS_LARGE_BUF]; /* MS2 */
    char    text_comment[MS_LARGE_BUF]; /* MS1 & MS2 - for MS1, presumed */
                    /* all comments are text */
    char    archbase[MS_SMALL_BUF]; /* MS2 */
    char    mapbase[MS_SMALL_BUF];  /* MS2 */
    char    codebase[MS_SMALL_BUF]; /* MS2 */
    char    flags[MS_SMALL_BUF];    /* MS2 */
    int     num_players;        /* MS1 & MS2 */
    uint32  in_bytes;           /* MS2 */
    uint32  out_bytes;          /* MS2 */
    int     idle_time;          /* MS1 - for MS2, calculated from */
                    /* last_update value */
    int     uptime;         /* MS2 */
    char    version[MS_SMALL_BUF];  /* MS1 & MS2 */
    int     sc_version;         /* MS2 */
    int     cs_version;         /* MS2 */
} Meta_Info;

char* server=NULL;
int need_mapping_update=1;
int meta_numservers = 0;
int metaserver_on=METASERVER; //FALSE
Meta_Info *meta_servers = NULL;
int ms2_is_running;
pthread_mutex_t ms2_info_mutex;

static char *metaservers[] = {"http://crossfire.real-time.com/metaserver2/meta_client.php"};

//Fill meta_serversp[]
size_t metaserver2_writer(void *ptr, size_t size, size_t nmemb, void *data)
{
#ifdef HAVE_CURL_CURL_H
    size_t realsize = size * nmemb;
    char    *cp, *newline, *eq, inbuf[CURL_MAX_WRITE_SIZE*2+1], *leftover;

    leftover = (char*) data;

 //   printf("metaserver2_writer: ptr --> \n");
 //   printf("[%s]\n",(char*)ptr);
    if (realsize > CURL_MAX_WRITE_SIZE) {
        //LOG(LOG_CRITICAL, "common::metaserver2_writer", "Function called with more data than allowed!");
    }

    /* This memcpy here is to just give us a null terminated character
     * array - easier to do with than having to check lengths as well as other
     * values.  Also, it makes it easier to deal with unprocessed data from
     * the last call.
     */
    memcpy(inbuf, leftover, strlen(leftover));
    memcpy(inbuf+strlen(leftover), ptr, realsize);
    inbuf[strlen(leftover)+realsize] = 0;
    leftover[0] =0;

    /* Processing this block of data shouldn't take very long, even on
     * slow machines, so putting the lock here, instead of each time
     * we update a variable is cleaner
     */
    pthread_mutex_lock(&ms2_info_mutex);
    printf("metaserver2_writer: inbuf --> \n");
    printf("\n[%s]\n",(char*)inbuf);
    for (cp = inbuf; cp != NULL && *cp!=0; cp=newline) {
        newline=strchr(cp, '\n');
        if (newline) {
            *newline = 0;
            newline++;
        } else {
            /* If we didn't get a newline, then this is the
             * end of the block of data for this call - store
             * away the extra for the next call.
             */
            strncpy(leftover, cp, CURL_MAX_WRITE_SIZE-1);
            leftover[CURL_MAX_WRITE_SIZE-1] = 0;
            break;
        }

        eq = strchr(cp,'=');
        if (eq) {
            *eq = 0;
            eq++;
        }

        if (!strcmp(cp, "START_SERVER_DATA")) {
            /* Clear out all data - MS2 doesn't necessarily use all the
             * fields, so blank out any that we are not using.
             */
            memset(&meta_servers[meta_numservers], 0, sizeof(Meta_Info));
        } else if (!strcmp(cp, "END_SERVER_DATA")) {
            int i;

            /* we can get data from both metaserver1 & 2 - no reason to keep
             * both.  So check for duplicates, and consider metaserver2
             * data 'better'.
             */
            for (i=0; i<meta_numservers; i++) {
                if (!strcasecmp(meta_servers[i].hostname, meta_servers[meta_numservers].hostname)) {
                    memcpy(&meta_servers[i], &meta_servers[meta_numservers], sizeof(Meta_Info));
                    break;
                }
            }
            if (i>=meta_numservers) {
                meta_numservers++;
            }
        } else {
            /* If we get here, these should be variable=value pairs.
             * if we don't have a value, can't do anything, and
             * report an error.  This would normally be incorrect
             * data from the server.
             */
            if (!eq) {
                //LOG(LOG_ERROR, "common::metaserver2_writer", "Unknown line: %s",cp);
                continue;
            }
            if (!strcmp(cp,"hostname")) {
                strncpy(meta_servers[meta_numservers].hostname, eq, sizeof(meta_servers[meta_numservers].hostname));
            } else if (!strcmp(cp,"port")) {
                meta_servers[meta_numservers].port = atoi(eq);
            } else if (!strcmp(cp,"html_comment")) {
                strncpy(meta_servers[meta_numservers].html_comment, eq, sizeof(meta_servers[meta_numservers].html_comment));
            } else if (!strcmp(cp,"text_comment")) {
                strncpy(meta_servers[meta_numservers].text_comment, eq, sizeof(meta_servers[meta_numservers].text_comment));
            } else if (!strcmp(cp,"archbase")) {
                strncpy(meta_servers[meta_numservers].archbase, eq, sizeof(meta_servers[meta_numservers].archbase));
            } else if (!strcmp(cp,"mapbase")) {
                strncpy(meta_servers[meta_numservers].mapbase, eq, sizeof(meta_servers[meta_numservers].mapbase));
            } else if (!strcmp(cp,"codebase")) {
                strncpy(meta_servers[meta_numservers].codebase, eq, sizeof(meta_servers[meta_numservers].codebase));
            } else if (!strcmp(cp,"flags")) {
                strncpy(meta_servers[meta_numservers].flags, eq, sizeof(meta_servers[meta_numservers].flags));
            } else if (!strcmp(cp,"version")) {
                strncpy(meta_servers[meta_numservers].version, eq, sizeof(meta_servers[meta_numservers].version));
            } else if (!strcmp(cp,"num_players")) {
                meta_servers[meta_numservers].num_players = atoi(eq);
            } else if (!strcmp(cp,"in_bytes")) {
                meta_servers[meta_numservers].in_bytes = atoi(eq);
            } else if (!strcmp(cp,"out_bytes")) {
                meta_servers[meta_numservers].out_bytes = atoi(eq);
            } else if (!strcmp(cp,"uptime")) {
                meta_servers[meta_numservers].uptime = atoi(eq);
            } else if (!strcmp(cp,"sc_version")) {
                meta_servers[meta_numservers].sc_version = atoi(eq);
            } else if (!strcmp(cp,"cs_version")) {
                meta_servers[meta_numservers].cs_version = atoi(eq);
            } else if (!strcmp(cp,"last_update")) {
                /* MS2 reports update time as when it last got an update,
                 * where as we want actual elapsed time since last update.
                 * So do the conversion.  Second check is because of clock
                 * skew - my clock may be fast, and we don't want negative times.
                 */
                meta_servers[meta_numservers].idle_time = time(NULL) - atoi(eq);
                if (meta_servers[meta_numservers].idle_time < 0) {
                    meta_servers[meta_numservers].idle_time = 0;
                }
            } else {
                //LOG(LOG_ERROR, "common::metaserver2_writer", "Unknown line: %s=%s",cp,eq);
            }
        }
    } 
    printf("Get meta_numservers: %d\n",meta_numservers);
    pthread_mutex_unlock(&ms2_info_mutex);
    return realsize;
#else
    return 0;
#endif
}


static int get_metaserver2_data(char *metaserver2)
{
#ifdef HAVE_CURL_CURL_H
    CURL *curl;
    CURLcode res;
    char    leftover[CURL_MAX_WRITE_SIZE];

    curl = curl_easy_init();
    if (!curl) {
        return 0;
    }
    leftover[0] =0;
    curl_easy_setopt(curl, CURLOPT_URL, metaserver2);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, metaserver2_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, leftover);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res) {
        return 0;
    } else {
        return 1;
    }
#else
    return 1;
#endif
}

int meta_sort(Meta_Info *m1, Meta_Info *m2)
{
    return strcasecmp(m1->hostname, m2->hostname);
}

void *metaserver2_thread(void *junk)
{
    int metaserver_choice, tries=0;
    do {
        metaserver_choice = random() % (sizeof(metaservers) / sizeof(char*));
        tries++;
        if (tries>5) {
            break;
        }
        printf("metaserver2_thread: tries=%d\n",tries);
    } while (!get_metaserver2_data(metaservers[metaserver_choice]));

    pthread_mutex_lock(&ms2_info_mutex);
    qsort(meta_servers, meta_numservers, sizeof(Meta_Info), (int (*)(const void *, const void *))meta_sort);
    ms2_is_running=0;
    pthread_mutex_unlock(&ms2_info_mutex);
    pthread_exit(NULL);
    // never reached, just to make the compiler happy.
    return NULL;
}


int metaserver2_get_info(void)
{
    pthread_t   thread_id;
    int     ret;

    //metaserver_load_cache();

    pthread_mutex_lock(&ms2_info_mutex);
    if (!meta_servers) {
        meta_servers = calloc(MAX_METASERVER, sizeof(Meta_Info));
    }

    ms2_is_running=1;
    pthread_mutex_unlock(&ms2_info_mutex);
    printf("metaserver2_get_info and call metaserver2_thread\n");
    ret=pthread_create(&thread_id, NULL, metaserver2_thread, NULL);
    if (ret) {
        //LOG(LOG_ERROR, "common::metaserver2_get_info", "Thread creation failed.");
        pthread_mutex_lock(&ms2_info_mutex);
        ms2_is_running=0;
        pthread_mutex_unlock(&ms2_info_mutex);
    }

    return 0;
}

static GtkWidget *metaserver_window, *treeview_metaserver, *metaserver_button,
       *metaserver_status, *metaserver_entry;
static GtkListStore *store_metaserver;
static GtkTreeSelection *metaserver_selection;

//Mouse click the row twice
void on_treeview_metaserver_row_activated(GtkTreeView *treeview,
    GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    printf("on_treeview_metaserver_row_activated\n");
    GtkTreeIter iter;
    GtkTreeModel *model;
    char *name, *ip;

    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &name, 1, &ip, -1);
        printf("Try to connect [%s]\n",name);
        //metaserver_connect_to(name);
    }
}

//Trigger when pressing ENTER in text entry line
void on_metaserver_text_entry_activate(GtkEntry *entry, gpointer user_data) {
    const gchar *entry_text;
    printf("on_metaserver_text_entry_activate\n");
    entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
    printf("Try to connect [%s]\n",entry_text);
    //metaserver_connect_to(entry_text);
}

//Trigger when typing in text entry line
gboolean on_metaserver_text_entry_key_press_event(GtkWidget *widget,
    GdkEventKey *event, gpointer user_data) {
    printf("on_metaserver_text_entry_key_press_event\n");
    gtk_widget_set_sensitive(metaserver_button, TRUE);
    gtk_tree_selection_unselect_all(metaserver_selection);
    return FALSE;
}

//Clicked Connect Button
void on_metaserver_select_clicked(GtkButton *button, gpointer user_data) {
    printf("on_metaserver_select_clicked\n");
 
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *name = NULL, *ip = NULL, *metaserver_txt;

    metaserver_txt = (char *)gtk_entry_get_text(GTK_ENTRY(metaserver_entry));
    if (gtk_tree_selection_get_selected(metaserver_selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &name, 1, &ip, -1);

    } else if (*metaserver_txt == '\0') {

        /* This can happen if user blanks out server name text field then hits
         * the connect button.
         */
        gtk_label_set_text(GTK_LABEL(metaserver_status), "Error - nothing selected!\n");
        gtk_widget_set_sensitive(metaserver_button, FALSE);
        return;
    } else {
        /* This shouldn't happen because the button should not be pressable
         * until the user selects something
         */
        gtk_label_set_text(GTK_LABEL(metaserver_status), "Error - nothing selected!\n");
    }
    if (!name) {
        name = metaserver_txt;
    }
    printf("Try to connect [%s]\n",name);
    //metaserver_connect_to(name);
}


void on_button_metaserver_quit_pressed(GtkButton *button, gpointer user_data) {
    on_window_destroy_event(GTK_OBJECT(button), user_data);
}

enum {
    LIST_HOSTNAME, LIST_IPADDR, LIST_IDLETIME, LIST_PLAYERS, LIST_VERSION, LIST_COMMENT
};

//Trigger when selecting item by mouse
gboolean metaserver_selection_func(GtkTreeSelection *selection,
        GtkTreeModel *model, GtkTreePath *path,
        gboolean path_currently_selected, gpointer userdata) {
    printf("metaserver_selection_func\n");
    gtk_widget_set_sensitive(metaserver_button, TRUE);
    gtk_entry_set_text(GTK_ENTRY(metaserver_entry), "");
    return TRUE;
}

int metaserver_check_status(void)
{
    int status;

    pthread_mutex_lock(&ms2_info_mutex);
    status = ms2_is_running ;
    pthread_mutex_unlock(&ms2_info_mutex);

    return status;
}

void get_metaserver() {
    GtkTreeIter iter;
    GtkWidget *widget;
    char file_cache[MAX_BUF];
    const gchar *metaserver_txt;
    int i, j;
    static int has_init = 0;

    //hide_all_login_windows();
    //snprintf(file_cache, MAX_BUF, "servers.cache");
    //CONVERT_FILESPEC_TO_OS_FORMAT(file_cache);
    //cached_server_file = file_cache;
     if (!has_init) {
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        metaserver_window = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                                       "metaserver_window"));
        gtk_window_set_transient_for(GTK_WINDOW(metaserver_window),
                                     GTK_WINDOW(window_root));
        treeview_metaserver = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                                         "treeview_metaserver"));
        metaserver_button = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                                       "metaserver_select"));
        metaserver_status = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                                       "metaserver_status"));
        metaserver_entry = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                                      "metaserver_text_entry"));

        g_signal_connect((gpointer) metaserver_window, "destroy",
                         G_CALLBACK(on_window_destroy_event), NULL);

        g_signal_connect((gpointer) treeview_metaserver, "row_activated",
                         G_CALLBACK(on_treeview_metaserver_row_activated), NULL);
        
        g_signal_connect((gpointer) metaserver_entry, "activate",
                         G_CALLBACK(on_metaserver_text_entry_activate), NULL);
        
        g_signal_connect((gpointer) metaserver_entry, "key_press_event",
                         G_CALLBACK(on_metaserver_text_entry_key_press_event), NULL);
        g_signal_connect((gpointer) metaserver_button, "clicked",
                         G_CALLBACK(on_metaserver_select_clicked), NULL);

        widget = GTK_WIDGET(gtk_builder_get_object(dialog_xml,
                            "button_metaserver_quit"));
        g_signal_connect((gpointer) widget, "pressed",
                         G_CALLBACK(on_button_metaserver_quit_pressed), NULL);
        g_signal_connect((gpointer) widget, "activate",
                         G_CALLBACK(on_button_metaserver_quit_pressed), NULL);

        store_metaserver = gtk_list_store_new(6,
                                              G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
                                              G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_metaserver),
                                GTK_TREE_MODEL(store_metaserver));

        renderer = gtk_cell_renderer_text_new();

        column = gtk_tree_view_column_new_with_attributes("Hostname", renderer,
                 "text", LIST_HOSTNAME, NULL);

        gtk_tree_view_column_set_sort_column_id(column, LIST_HOSTNAME);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_metaserver), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Updated (Sec)", renderer,
                 "text", LIST_IDLETIME, NULL);
        gtk_tree_view_column_set_sort_column_id(column, LIST_IDLETIME);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_metaserver), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Players", renderer,
                 "text", LIST_PLAYERS, NULL);
        gtk_tree_view_column_set_sort_column_id(column, LIST_PLAYERS);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_metaserver), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Version", renderer,
                 "text", LIST_VERSION, NULL);
        gtk_tree_view_column_set_sort_column_id(column, LIST_VERSION);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_metaserver), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Server Comment", renderer,
                 "text", LIST_COMMENT, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_metaserver), column);

        gtk_widget_realize(metaserver_window);

        metaserver_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
                                   treeview_metaserver));
        gtk_tree_selection_set_mode(metaserver_selection, GTK_SELECTION_BROWSE);
        gtk_tree_selection_set_select_function(metaserver_selection,
                                               metaserver_selection_func, NULL, NULL);
        has_init = 1;
    }
    gtk_widget_show(metaserver_window);
    //
    gtk_label_set_text(GTK_LABEL(metaserver_status),
                       "Waiting for data from metaserver");
    metaserver_txt = gtk_entry_get_text(GTK_ENTRY(metaserver_entry));
    if (*metaserver_txt == '\0') {
        gtk_widget_set_sensitive(metaserver_button, FALSE);
    } else {
        gtk_widget_set_sensitive(metaserver_button, TRUE);
    }

    gtk_list_store_clear(store_metaserver);
    
    while (metaserver_check_status()) {
        usleep(100);
        gtk_main_iteration_do(FALSE);
    }
    printf("exit metaserver_check_status()\n");
    pthread_mutex_lock(&ms2_info_mutex);
    qsort(meta_servers, meta_numservers, sizeof(Meta_Info), (int (*)(const void *,
            const void *))meta_sort);
    for (i = 0; i < meta_numservers; i++) {
        //if (check_server_version(i)) {
            gtk_list_store_append(store_metaserver, &iter);
            gtk_list_store_set(store_metaserver, &iter,
                               LIST_HOSTNAME, meta_servers[i].hostname,
                               LIST_IPADDR, meta_servers[i].hostname,
                               LIST_IDLETIME,  meta_servers[i].idle_time,
                               LIST_PLAYERS, meta_servers[i].num_players,
                               LIST_VERSION, meta_servers[i].version,
                               LIST_COMMENT, meta_servers[i].text_comment,
                               -1);
       // }
    }
    pthread_mutex_unlock(&ms2_info_mutex);
 
    //
    cpl.input_state = Metaserver_Select;
    gtk_label_set_text(GTK_LABEL(metaserver_status), "Waiting for user selection");
    gtk_main();

    gtk_widget_hide(metaserver_window);
}

int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);
 	init_ui();   
    mapdata_init();
    gtk_widget_show(window_root);
    map_init(window_root);
    init_sockets();
    init_image_cache_data();
    while (1) {
        reset_client_vars();
        csocket.inbuf.len = 0;
        csocket.cs_version = 0;
    	if (server == NULL) {
            //draw_splash();
            printf("server = NULL\n");
            metaserver2_get_info();
            get_metaserver();
        }
    	event_loop();
	}

  exit(EXIT_SUCCESS);
}