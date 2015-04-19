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
    //icon_image should be (GdkPixbuf*)
    void        *icon_mask, *icon_image;
    uint16      icon_width, icon_height;
    //map_image should be (GdkPixmap*)
    //map_mask should be (GdkBitmap*)
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

ClientSocket csocket;
Face_Information face_info;
Client_Player cpl;

uint32 tick=0;

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

GtkWidget *map_drawing_area, *map_notebook;
static GdkBitmap *dark1, *dark2, *dark3;
static GdkPixmap *dark;

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
int replyinfo_status=0, requestinfo_sent=0, replyinfo_last_face=0;
/*
int meta_port=META_PORT, want_skill_exp=0,
    replyinfo_status=0, requestinfo_sent=0, replyinfo_last_face=0,
    maxfd,metaserver_on=METASERVER, metaserver2_on=METASERVER2,
          wantloginmethod=0, serverloginmethod=0;
*/

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

static GtkWidget *metaserver_window, *treeview_metaserver, *metaserver_button,
       *metaserver_status, *metaserver_entry;
static GtkListStore *store_metaserver;
static GtkTreeSelection *metaserver_selection;

enum {
    LIST_HOSTNAME, LIST_IPADDR, LIST_IDLETIME, LIST_PLAYERS, LIST_VERSION, LIST_COMMENT
};

struct script_state {
    //lua_State* state;
    const char* filename;
};

static struct script_state* scripts = NULL;
int num_scripts;

#define VERSION_CS 1023
#define VERSION_SC 1029
char VERSION_INFO[MAX_BUF];

enum CmdFormat {
   ASCII,
   SHORT_ARRAY,
   INT_ARRAY,
   SHORT_INT, /* one short, one int */
   MIXED, /* weird ones like magic map */
   STATS,
   NODATA
};

struct CmdMapping {
    const char *cmdname;
    void (*cmdproc)(unsigned char *, int );
    enum CmdFormat cmdformat;
};

#define NCOMMANDS ((int)(sizeof(commands)/sizeof(struct CmdMapping)))

typedef void (*CmdProc)(unsigned char *, int len);

#define TEXTVIEW_MOTD           0
#define TEXTVIEW_NEWS           1
#define TEXTVIEW_RULES_ACCOUNT  2
#define TEXTVIEW_RULES_CHAR     3

static GtkWidget *add_character_window, *choose_char_window,
       *create_account_window, *login_window, *account_password_window;

/* These are in the login_window */
static GtkWidget *button_login, *button_create_account,
       *button_go_metaserver, *button_exit_client,
       *entry_account_name,
       *entry_account_password, *label_account_login_status;

static GtkWidget *button_new_create_account, *button_new_cancel,
       *entry_new_account_name,
       *entry_new_account_password, *entry_new_confirm_password,
       *label_create_account_status;


GtkTextBuffer *textbuf_motd, *textbuf_news, *textbuf_rules_account,
              *textbuf_rules_char;

#define NUM_COLORS 13
#define FONT_NORMAL     0
#define FONT_ARCANE     1
#define FONT_STRANGE    2
#define FONT_FIXED      3
#define FONT_HAND       4
#define NUM_FONTS       5
#define MSG_TYPE_LAST                 21
typedef struct Info_Pane
{
    GtkWidget       *textview;
    GtkWidget       *scrolled_window;
    GtkTextBuffer   *textbuffer;
    GtkTextMark     *textmark;
    GtkAdjustment   *adjustment;
    GtkTextTag      *color_tags[NUM_COLORS];
    GtkTextTag      *font_tags[NUM_FONTS];
    GtkTextTag      *bold_tag, *italic_tag, *underline_tag, *default_tag;
    GtkTextTag      **msg_type_tags[MSG_TYPE_LAST];
} Info_Pane;

Info_Pane login_pane[4];
char account_password[256];

int start_login_has_init = 0;

void SetupCmd(char *buf, int len);
void FailureCmd(char *buf, int len);
void VersionCmd(char *data, int len);
void TickCmd(uint8 *data, int len);
void ReplyInfoCmd(uint8 *buf, int len);
void AccountPlayersCmd(char *buf, int len);
void Image2Cmd(uint8 *data,  int len);
void PlayerCmd(unsigned char *data, int len);
void Item2Cmd(unsigned char *data, int len);
void NewmapCmd(unsigned char *data, int len);
void StatsCmd(unsigned char *data, int len);
void Map2Cmd(unsigned char *data, int len);
struct CmdMapping commands[] = {
    { "version",         (CmdProc)VersionCmd, ASCII },
    { "replyinfo",       ReplyInfoCmd, ASCII},
    { "setup",           (CmdProc)SetupCmd, ASCII},
    { "failure",         (CmdProc)FailureCmd, ASCII},
    { "accountplayers",  (CmdProc)AccountPlayersCmd, ASCII},
     { "image2",          Image2Cmd, MIXED       },
     { "player",          PlayerCmd, MIXED       },
      { "item2",           Item2Cmd, MIXED },
       { "newmap",          NewmapCmd, NODATA },
        {
        "stats",           StatsCmd, STATS      //update player stats 
    },
    { "map2",            Map2Cmd, SHORT_ARRAY },
    /*
    { "map2",            Map2Cmd, SHORT_ARRAY },
    { "map_scroll",      (CmdProc)map_scrollCmd, ASCII },
    { "magicmap",        MagicMapCmd, MIXED   },
    { "newmap",          NewmapCmd, NODATA },
    { "mapextended",     MapExtendedCmd, MIXED  },

    { "item2",           Item2Cmd, MIXED },
    { "upditem",         UpdateItemCmd, MIXED },
    { "delitem",         DeleteItem, INT_ARRAY },
    { "delinv",          DeleteInventory, ASCII },

    { "addspell",        AddspellCmd, MIXED },
    { "updspell",        UpdspellCmd, MIXED },
    { "delspell",        DeleteSpell, INT_ARRAY },

    { "drawinfo",        (CmdProc)DrawInfoCmd, ASCII },
    { "drawextinfo",     (CmdProc)DrawExtInfoCmd, ASCII},
    {
        "stats",           StatsCmd, STATS       
    },
    { "image2",          Image2Cmd, MIXED       },
    {
        "face2",           Face2Cmd, MIXED       
    },
    */

    { "tick",            TickCmd, INT_ARRAY    },
/*
    { "music",           (CmdProc)MusicCmd, ASCII },
    {
        "sound2",          Sound2Cmd, MIXED     
    },
    { "anim",            AnimCmd, SHORT_ARRAY},
    { "smooth",          SmoothCmd, SHORT_ARRAY},

    { "player",          PlayerCmd, MIXED       },
    { "comc",            CompleteCmd, SHORT_INT },

    { "addme_failed",    (CmdProc)AddMeFail, NODATA },
    { "addme_success",   (CmdProc)AddMeSuccess, NODATA },
    { "version",         (CmdProc)VersionCmd, ASCII },
    { "goodbye",         (CmdProc)GoodbyeCmd, NODATA },
    { "setup",           (CmdProc)SetupCmd, ASCII},
    { "failure",         (CmdProc)FailureCmd, ASCII},
    { "accountplayers",  (CmdProc)AccountPlayersCmd, ASCII},

    { "query",           (CmdProc)handle_query, ASCII},
    { "replyinfo",       ReplyInfoCmd, ASCII},
    { "ExtendedTextSet", (CmdProc)SinkCmd, NODATA},
    { "ExtendedInfoSet", (CmdProc)SinkCmd, NODATA},

    { "pickup",          PickupCmd, INT_ARRAY  },
*/
};

#define CFG_LT_TILE         1
#define FOG_MAP_SIZE 512

#define RI_IMAGE_INFO 0x1
#define RI_IMAGE_SUMS 0x2

gint csocket_fd;

struct timeval timeout;

void DoClient(ClientSocket *csocket);

GtkListStore    *character_store;


static GtkWidget *button_play_character, *button_create_character,
       *button_add_character, *button_return_login, *button_account_password,
       *treeview_choose_character;

static GtkWidget *entry_new_character_name, *new_character_window,
       *label_new_char_status, *button_create_new_char,
       *button_new_char_cancel;

#define NUM_NEW_CHAR_STATS  7
#define NUM_OPT_FIELDS  6
static GtkWidget *spinbutton_cc[NUM_NEW_CHAR_STATS], *label_rs[NUM_NEW_CHAR_STATS],
       *label_cs[NUM_NEW_CHAR_STATS], *label_tot[NUM_NEW_CHAR_STATS],
       *label_cc_unspent, *textview_rs_desc, *label_cc_desc, *label_cc_status_update,
       *button_cc_cancel, *button_cc_done, *create_character_window, *combobox_rs,
       *combobox_cs, *textview_cs_desc, *entry_new_character_name, *button_choose_starting_map,
       *opt_label[NUM_OPT_FIELDS], *opt_combobox[NUM_OPT_FIELDS];

#define PNGX_NOFILE     1
#define PNGX_OUTOFMEM   2
#define PNGX_DATA       3

static uint8 *data_cp;
static int data_len, data_start;

#define MAX_ICON_SPACES     10
static const int icon_rescale_factor[MAX_ICON_SPACES] = {
    100, 100,           80 /* 2 = 160 */,   60 /* 3 = 180 */,
    50 /* 4 = 200 */,   45 /* 5 = 225 */,   40 /* 6 = 240 */,
    35 /* 7 = 259 */,   35 /* 8 = 280 */,   33 /* 9 = 300 */
};


#define DEFAULT_IMAGE_SIZE      32
#define RATIO   100

#define MAX_IMAGE_WIDTH         1024
#define MAX_IMAGE_HEIGHT        1024
#define BPP 4
#define NAME_LEN    128
#define copy_name(t,f) strncpy(t, f, NAME_LEN-1); t[NAME_LEN-1]=0;
#define CS_STAT_RESIST_START      100 /**< Start of resistances (inclusive) */
#define CS_STAT_RESIST_END        117 /**< End of resistances (inclusive)   */
#define CS_STAT_SKILLINFO         140
#define CS_NUM_SKILLS              50
void Map2Cmd(unsigned char *data, int len);
#define MAX_VIEW 64
#define MAP2_COORD_OFFSET   15

#define MAP2_TYPE_CLEAR         0x0
#define MAP2_TYPE_DARKNESS      0x1

#define MAP2_LAYER_START        0x10
#define MAXLAYERS 10
#define MAP1_LAYERS 3

#define FACE_IS_ANIM    1<<15
#define ANIM_RANDOM     1<<13
#define ANIM_SYNC       2<<13

#define ANIM_FLAGS_MASK 0x6000 

#define ANIM_MASK       0x1fff

int mapupdatesent = 0;
