#include "test1.h"
#include <png.h>

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

void inventory_tick(void)
{
    //animate_inventory();
    //animate_look();
}

void mapdata_animation(void)
{
    int x, y, layer, face;
    struct MapCellLayer *cell;
    //
}

int do_timeout() {

    if (cpl.showmagic) {
        printf("cpl.showmagic\n");
        //magic_map_flash_pos();
    }
    if (cpl.spells_updated) {
        printf("cpl.spells_updated\n");
        //update_spell_information();
    }
    if (!tick) {
        //printf("!tick\n");
        inventory_tick();
        mapdata_animation();
    }
    
    return TRUE;
}


void do_network() {
     printf("do_network\n");
     fd_set tmp_read;
     int pollret;
     if (csocket.fd == -1) {
         printf("a) do_network: csocket.fd == -1\n");
        if (csocket_fd) {
            gdk_input_remove(csocket_fd);
            csocket_fd = 0;
            gtk_main_quit();
        }
        return;
    }
    //
    FD_ZERO(&tmp_read);
    FD_SET(csocket.fd, &tmp_read);
    timeout.tv_sec = timeout.tv_usec = 0;

    pollret = select(maxfd, &tmp_read, NULL, NULL, &timeout);
    if (pollret > 0)
    {
        if (FD_ISSET(csocket.fd, &tmp_read)) {
            DoClient(&csocket);
        }
    }
    //
    if (csocket.fd == -1) {
         printf("b) do_network: csocket.fd == -1\n");
        if (csocket_fd) {
            gdk_input_remove(csocket_fd);
            csocket_fd = 0;
            gtk_main_quit();
        }
        return;
    }
    
    //draw_map(FALSE);
    //draw_lists();
}

void event_loop() {
    int tag;
	printf("loop\n");
	gtk_timeout_add(10, (GtkFunction) do_timeout, NULL);
#ifdef WIN32
    //gtk_timeout_add(25, (GtkFunction) do_scriptout, NULL);
#endif

     csocket_fd = gdk_input_add((gint) csocket.fd,
                               GDK_INPUT_READ,
                               (GdkInputFunction) do_network, &csocket);

     tag = csocket_fd;
    gtk_main();
    gtk_timeout_remove(tag);
}

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

//#define HAVE_GETADDRINFO 1
int init_connection(char *host, int port)
{
    int fd = -1, oldbufsize, newbufsize=65535;
    socklen_t buflen=sizeof(int);
    printf("init_connection\n");
    #if !HAVE_GETADDRINFO || WIN32
        printf("!HAVE_GETADDRINFO\n");
        struct sockaddr_in insock;
        struct protoent *protox;

        protox = getprotobyname("tcp");
        fd = socket(PF_INET, SOCK_STREAM, protox->p_proto);
        if (fd==-1) {
            printf("getaddrinfo fail\n");    
            return -1;
        }
        insock.sin_family = AF_INET;
        insock.sin_port = htons((unsigned short)port);
        if (isdigit(*host)) {
            insock.sin_addr.s_addr = inet_addr(host);
        } 
        else {
            struct hostent *hostbn = gethostbyname(host);
            if (hostbn == (struct hostent *) NULL) {
                printf("unknown host\n");  
                return -1;
            }
            memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
        }
        if (connect(fd,(struct sockaddr *)&insock,sizeof(insock)) == (-1)) {
            printf("can't connect to server\n"); 
            return -1;
        }
        printf("init_connect step 1\n");
    #else
    struct addrinfo hints;
    struct addrinfo *res = NULL, *ai;
    char port_str[6];
    int fd_status, fd_flags, fd_select, fd_sockopt;
    struct timeval tv;
    fd_set fdset;
    //convert port to port_str
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset(&hints, 0, sizeof(hints));
    //fill hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        printf("getaddrinfo fail\n");
        return -1;
    }
    fd_status = 0;
    //got ai fromm res
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd == -1) {
            printf("create socket fail!!\n");
            continue;
        }else{
            printf("create socket  success!!\n");
        }
    }
    #endif
    free(csocket.servername);
    csocket.servername = malloc(sizeof(char)*(strlen(host)+1));
    strcpy(csocket.servername, host);

    #ifndef WIN32
        #include <fcntl.h>
        if (fcntl(fd, F_SETFL, O_NDELAY)==-1) {
            printf("Error on fcntl.\n");
        }
    #else
    {
        unsigned long tmp = 1;
        if (ioctlsocket(fd, FIONBIO, &tmp)<0) {
            printf("Error on ioctlsocket.\n");
        }
    }
    #endif
    //Get bufsize to oldbufsize
   if (getsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, &buflen)==-1) {
        oldbufsize=0;
    }
    //Get set new bufsize
    if (oldbufsize<newbufsize) {
        if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&newbufsize, sizeof(&newbufsize))) {
            printf("Error to set bufsize, old is %d.\n",oldbufsize);
            setsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, sizeof(&oldbufsize));
        }
    }
    printf("init_connect step 2\n");
    return fd;
}


static void metaserver_connect_to(const char *name) {
    char buf[256], *next_token = (char *)name, *hostname;
    int port = 13327;
    /* Set client status and update GUI before continuing. */
    snprintf(buf, sizeof(buf), "Connecting to '%s'...", name);
    gtk_label_set_text(GTK_LABEL(metaserver_status), buf);
    gtk_main_iteration();
    hostname = strsep(&next_token, ":");
    if (next_token != NULL) {
        port = atoi(next_token);
    }
    csocket.fd = init_connection(hostname, port);
    if (csocket.fd != -1) {
        printf("[%s:%d] connected\n",hostname,port);
        //metaserver_update_cache(name, name);
        gtk_main_quit();
        cpl.input_state = Playing;
    } else {
        snprintf(buf, 255, "Unable to connect to %s!", name);
        gtk_label_set_text(GTK_LABEL(metaserver_status), buf);
    }

}

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
        metaserver_connect_to(name);
    }
}

//Trigger when pressing ENTER in text entry line
void on_metaserver_text_entry_activate(GtkEntry *entry, gpointer user_data) {
    const gchar *entry_text;
    printf("on_metaserver_text_entry_activate\n");
    entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
    printf("Try to connect [%s]\n",entry_text);
    metaserver_connect_to(entry_text);
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
    metaserver_connect_to(name);
}


void on_button_metaserver_quit_pressed(GtkButton *button, gpointer user_data) {
    on_window_destroy_event(GTK_OBJECT(button), user_data);
}

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

void SockList_Init(SockList *sl, uint8 *buf)
{
    sl->len=0;
    sl->buf=buf + 2;    /* reserve two bytes for total length */
}
void SockList_AddChar(SockList *sl, char c)
{
    sl->buf[sl->len++]=c;
}

void SockList_AddShort(SockList *sl, uint16 data)
{
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data & 0xff;
}

void SockList_AddInt(SockList *sl, uint32 data)
{
    sl->buf[sl->len++] = (data>>24)&0xff;
    sl->buf[sl->len++] = (data>>16)&0xff;
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data & 0xff;
}


void SockList_AddString(SockList *sl, const char *str)
{
    int len = strlen(str);

    if (sl->len + len > MAX_BUF-2) {
        len = MAX_BUF-2 - sl->len;
    }
    memcpy(sl->buf + sl->len, str, len);
    sl->len += len;
}

void script_monitor_str(const char *command)
{
    int i;
    /* For each script... */
    for (i = 0; i < num_scripts; ++i) {
        /* Do we send the command? */
//        if (scripts[i].monitor) {
  //          char buf[1024];
  //          snprintf(buf, sizeof(buf), "monitor %s\n", command);
   //         write(scripts[i].out_fd, buf, strlen(buf));
   //     }
    }
}

static int write_socket(int fd, const unsigned char *buf, int len)
{
    int amt=0;
    const unsigned char *pos=buf;
    /* If we manage to write more than we wanted, take it as a bonus */
    while (len>0) {
        do {
#ifndef WIN32
            amt=write(fd, pos, len);
        } while ((amt<0) && ((errno==EINTR) || (errno=EAGAIN)));
#else
            amt=send(fd, pos, len, 0);
        }
        while ((amt<0) && (WSAGetLastError()==EINTR));
#endif
        if (amt < 0) { /* We got an error */
            printf("New socket (fd=%d) write failed: %s.\n",fd, strerror(errno));
            return -1;
        }
        if (amt==0) {
            printf("Write_To_Socket: No data written out.");
        }
        len -= amt;
        pos += amt;
    }
    return 0;
}

int SockList_Send(SockList *sl, int fd)
{
    sl->buf[-2] = sl->len / 256;
    sl->buf[-1] = sl->len % 256;

    return write_socket(fd, sl->buf-2, sl->len+2);
}


int cs_print_string(int fd, const char *str, ...)
{
    va_list args;
    SockList sl;
    uint8 buf[MAX_BUF];

    SockList_Init(&sl, buf);
    va_start(args, str);
    sl.len += vsprintf((char*)sl.buf + sl.len, str, args);
    va_end(args);
    //[version 1023 1029]
    printf("cs_print_string:[%s]\n",sl.buf);
    //script_monitor_str((char*)sl.buf);
    //return 0;
   return SockList_Send(&sl, fd);
}

void SendVersion(ClientSocket csock) {
    cs_print_string(csock.fd, "version %d %d %s",
            VERSION_CS, VERSION_SC, VERSION_INFO);
}

void close_server_connection()
{
#ifdef WIN32
    closesocket(csocket.fd);
#else
    close(csocket.fd);
#endif
    csocket.fd = -1;
}

void TickCmd(uint8 *data, int len)
{

   // tick = GetInt_String(data);

    /* Up to the specific client to decide what to do */
   // client_tick(tick);
}

void VersionCmd(char *data, int len) {
    char *cp;
    printf("VersionCmd:[%s\n]",data);
    csocket.cs_version = atoi(data);
    /* set sc_version in case it is an old server supplying only one version */
    csocket.sc_version = csocket.cs_version;
    if (csocket.cs_version != VERSION_CS) {
        printf("Differing C->S version numbers (%d,%d)", VERSION_CS, csocket.cs_version);
        /*  exit(1);*/
    }
    cp = strchr(data, ' ');
    if (!cp) {
        return;
    }
    csocket.sc_version = atoi(cp);
    if (csocket.sc_version != VERSION_SC) {
        printf("Differing S->C version numbers (%d,%d)", VERSION_SC, csocket.sc_version);
    }
    cp = strchr(cp + 1, ' ');
    if (cp) {
        printf("Playing on server type %s", cp);
    }
}

void ReplyInfoCmd(uint8 *buf, int len)
{
    uint8 *cp;
    int i;
    if (!buf) {
        return;
    }
    for (i = 0; i < len; i++) {
        /* Either a space or newline represents a break */
        if (*(buf+i) == ' ' || *(buf+i) == '\n') {
            break;
        }
    }
    cp = buf+i;
    *cp++ = '\0';
    printf("ReplyInfoCmd_cmd: [%s]\n",buf);
    printf("ReplyInfoCmd_data: [%s]\n",cp);
    if (!strcmp((char*)buf, "image_info")) {
        //get_image_info(cp, len-i-1);
    }
}

void DoClient(ClientSocket *csocket)
{
    int i, len;
    unsigned char *data;

    while (1) {
        i = SockList_ReadPacket(csocket->fd, &csocket->inbuf, MAXSOCKBUF - 1);
        /*
         * If a socket error occurred while reading the packet, drop the
         * server connection.  Is there a better way to handle this?
         */
        if (i == -1) {
            close_server_connection();
            return;
        }
        /*
         * Drop incomplete packets without attempting to process the contents.
         */
        if (i == 0) {
            return;
        }
        // printf("csocket->inbuf.buf: [%s]\n",csocket->inbuf.buf);

        /*
         * Null-terminate the buffer, and set the data pointer so it points
         * to the first character of the data (following the packet length).
         */
        csocket->inbuf.buf[csocket->inbuf.len] = '\0';
        data = csocket->inbuf.buf + 2;
        /*
         * Commands that provide data are always followed by a space.  Find
         * the space and convert it to a null character.  If no spaces are
         * found, the packet contains a command with no associatd data.
         */
        while ((*data != ' ') && (*data != '\0')) {
            ++data;
        }
        if (*data == ' ') {
            *data = '\0';
            data++;
            len = csocket->inbuf.len - (data - csocket->inbuf.buf);
        } else {
            len = 0;
        }
        /*
         * Search for the command in the list of supported server commands.
         * If the server command is supported by the client, let the script
         * watcher know what command was received, then process it and quit
         * searching the command list.
         */
         //Do command calling
         printf("csocket->inbuf.buf+2: [%s]\n",csocket->inbuf.buf+2);
         
        for(i = 0; i < NCOMMANDS; i++) {
            if (strcmp((char*)csocket->inbuf.buf+2,commands[i].cmdname)==0) {
                //script_watch((char*)csocket->inbuf.buf+2,data,len,commands[i].cmdformat);
                commands[i].cmdproc(data,len);
                break;
            }
        }
        
        /*
         * After processing the command, mark the socket input buffer empty.
         */
        csocket->inbuf.len=0;
        /*
         * Complain about unsupported commands to facilitate troubleshooting.
         * The client and server should negotiate a connection such that the
         * server does not send commands the client does not support.
         */
        if (i == NCOMMANDS) {
            printf("Unrecognized command from server (%s)\n",
                   csocket->inbuf.buf+2);
        }
    }
}

static void do_account_login(const char *name, const char *password) {
    SockList sl;
    uint8 buf[MAX_BUF];

    if (!name || !password || *name == 0 || *password == 0) {
        printf("You must enter both a name and password!\n");
        gtk_label_set_text(GTK_LABEL(label_account_login_status),
                           "You must enter both a name and password!");
    } else {
        gtk_label_set_text(GTK_LABEL(label_account_login_status), "");

        SockList_Init(&sl, buf);
        SockList_AddString(&sl, "accountlogin ");
        SockList_AddChar(&sl, strlen(name));
        SockList_AddString(&sl, name);
        SockList_AddChar(&sl, strlen(password));
        SockList_AddString(&sl, password);
        SockList_Send(&sl, csocket.fd);
        printf("SockList_Send: [%s]\n", sl.buf);
 /*
        SendVersion(csocket);
        printf("do_account_login: send login\n");
        while (1) {
            DoClient(&csocket);
        usleep(10*1000);  
        }
        */
        /* Store password away for new character creation */
        snprintf(account_password, sizeof(account_password), "%s", password);
    }
}

void
on_entry_account_name_activate(GtkEntry *entry, gpointer user_data) {
    const char *password;

    password = gtk_entry_get_text(GTK_ENTRY(entry_account_password));

    if (!password || *password == 0) {
        gtk_widget_grab_focus(entry_account_password);
    } else {
        do_account_login(gtk_entry_get_text(GTK_ENTRY(entry_account_name)), password);
    }
}

void
on_entry_account_password_activate(GtkEntry *entry, gpointer user_data) {
    const char *name;

    name = gtk_entry_get_text(GTK_ENTRY(entry_account_name));

    if (!name || *name == 0) {
        gtk_widget_grab_focus(entry_account_name);
    } else {
        do_account_login(name, gtk_entry_get_text(GTK_ENTRY(entry_account_password)));
    }
}


static void do_account_create(const char *name, const char *p1,
                              const char *p2) {
    SockList sl;
    uint8 buf[MAX_BUF];

    if (strcmp(p1, p2)) {
        gtk_label_set_text(GTK_LABEL(label_create_account_status),
                           "The passwords you entered do not match!");
        return;
    } else {
        gtk_label_set_text(GTK_LABEL(label_create_account_status), "");
        SockList_Init(&sl, buf);
        SockList_AddString(&sl, "accountnew ");
        SockList_AddChar(&sl, strlen(name));
        SockList_AddString(&sl, name);
        SockList_AddChar(&sl, strlen(p1));
        SockList_AddString(&sl, p1);
        printf("do_account_create: [%s]\n",sl.buf);
 //       SockList_Send(&sl, csocket.fd);
        /* Store password away for new character creation */
        snprintf(account_password, sizeof(account_password), "%s", p1);
    }
}

void
on_button_new_create_account_clicked(GtkButton *button, gpointer user_data) {
    const char *password1, *password2, *name;

    password1 = gtk_entry_get_text(GTK_ENTRY(entry_new_account_password));
    password2 = gtk_entry_get_text(GTK_ENTRY(entry_new_confirm_password));
    name = gtk_entry_get_text(GTK_ENTRY(entry_new_account_name));

    if (name && name[0] && password1 && password1[0] && password2 && password2[0]) {
        do_account_create(name, password1, password2);
    } else {
        gtk_label_set_text(GTK_LABEL(label_create_account_status),
                           "You must fill in all three entries!");
    }
}


static void init_create_account_window() {
    GtkTextIter end;

    create_account_window =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "create_account_window"));

      button_new_create_account =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_new_create_account"));
    button_new_cancel =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_new_cancel"));
    login_pane[TEXTVIEW_RULES_ACCOUNT].textview =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "textview_rules_account"));

    textbuf_rules_account =
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(login_pane[TEXTVIEW_RULES_ACCOUNT].textview));

    entry_new_account_name =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "entry_new_account_name"));
    entry_new_account_password =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "entry_new_account_password"));
    entry_new_confirm_password =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "entry_new_confirm_password"));
    label_create_account_status =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "label_create_account_status"));

    g_signal_connect((gpointer) button_new_create_account, "clicked",
                     G_CALLBACK(on_button_new_create_account_clicked), NULL);
 
    //gtk_widget_show(create_account_window);
}

void
on_button_create_account_clicked(GtkButton *button, gpointer user_data) {
    gtk_widget_hide(login_window);
    gtk_label_set_text(GTK_LABEL(label_create_account_status), "");
    gtk_entry_set_text(GTK_ENTRY(entry_new_account_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_new_account_password), "");
    gtk_entry_set_text(GTK_ENTRY(entry_new_confirm_password), "");
    gtk_widget_show(create_account_window);
}


void
on_button_login_clicked(GtkButton *button, gpointer user_data) {
    printf("on_button_login_clicked\n");
    do_account_login(gtk_entry_get_text(GTK_ENTRY(entry_account_name)),
                     gtk_entry_get_text(GTK_ENTRY(entry_account_password)));
}


static void init_login_window() {
    GtkTextIter end;

    login_window = GTK_WIDGET(gtk_builder_get_object(dialog_xml, "login_window"));
    gtk_window_set_transient_for(
        GTK_WINDOW(login_window), GTK_WINDOW(window_root));

    button_login =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_login"));
    button_create_account =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_create_account"));
    button_go_metaserver =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_go_metaserver"));
    button_exit_client =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_exit_client"));
    label_account_login_status =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "label_account_login_status"));


    login_pane[TEXTVIEW_MOTD].textview =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "textview_motd"));
    textbuf_motd =
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(login_pane[TEXTVIEW_MOTD].textview));

    login_pane[TEXTVIEW_NEWS].textview =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "textview_news"));
    textbuf_news =
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(login_pane[TEXTVIEW_NEWS].textview));

    entry_account_name =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "entry_account_name"));
    entry_account_password =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "entry_account_password"));

   // g_signal_connect((gpointer) login_window, "delete_event",
   //                  G_CALLBACK(on_window_delete_event), NULL);
   g_signal_connect((gpointer) entry_account_name, "activate",
                     G_CALLBACK(on_entry_account_name_activate), NULL);
   g_signal_connect((gpointer) entry_account_password, "activate",
                     G_CALLBACK(on_entry_account_password_activate), NULL);
   g_signal_connect((gpointer) button_login, "clicked",
                    G_CALLBACK(on_button_login_clicked), NULL);
    g_signal_connect((gpointer) button_create_account, "clicked",
                     G_CALLBACK(on_button_create_account_clicked), NULL);
    /*
    g_signal_connect((gpointer) button_go_metaserver, "clicked",
                     G_CALLBACK(on_button_go_metaserver_clicked), NULL);
    g_signal_connect((gpointer) button_exit_client, "clicked",
                     G_CALLBACK(on_button_exit_client_clicked), NULL);
*/
 }

void create_character_window_show()
{
    printf("create_character_window_show\n");
    //cs_print_string(csocket.fd, "requestinfo race_list");
    //cs_print_string(csocket.fd, "requestinfo class_list");
    //cs_print_string(csocket.fd, "requestinfo newcharinfo");
     gtk_widget_show(create_character_window);
}

void
on_button_create_character_clicked(GtkButton *button, gpointer user_data) {
    gtk_widget_hide(choose_char_window);
    printf("on_button_create_character_clicked: serverloginmethod=%d \n",serverloginmethod);
    if (serverloginmethod >= 2) {
        create_character_window_show();
    } else {
        gtk_widget_show(new_character_window);
        gtk_entry_set_text(GTK_ENTRY(entry_new_character_name), "");
    }
}


static void create_new_character() {
    const char *name;
    uint8 buf[MAX_BUF];
    SockList sl;

    SockList_Init(&sl, buf);

    name =  gtk_entry_get_text(GTK_ENTRY(entry_new_character_name));

   // if (!name || *name == 0) {
   //     gtk_label_set_text(GTK_LABEL(label_new_char_status),
    //                       "You must enter a character name.");
    //    return;
    //} else {
       //strcpy(name,"aaa");
        gtk_label_set_text(GTK_LABEL(label_new_char_status), "");

        SockList_AddString(&sl, "createplayer ");
        SockList_AddChar(&sl, strlen(name));
        SockList_AddString(&sl, name);
        SockList_AddChar(&sl, strlen(account_password));
        SockList_AddString(&sl, account_password);
        SockList_Send(&sl, csocket.fd);
   // }
}


static void send_create_player_to_server()
{
    const gchar *char_name;
    int i, on_choice, tmp;
    SockList sl;
    char buf[MAX_BUF];
    uint8 sockbuf[MAX_BUF];

    SockList_Init(&sl, sockbuf);
    SockList_AddString(&sl, "createplayer ");
    SockList_AddChar(&sl, strlen("neo"));
    SockList_AddString(&sl, "neo");
    SockList_AddChar(&sl, strlen(account_password));

    SockList_AddString(&sl, account_password);

    snprintf(buf, MAX_BUF, "race %s", "dwarf_player");
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);
 
    snprintf(buf, MAX_BUF, "class %s", "alchemist_class");
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "starting_map %s", "map_beginners_house");
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "str",15);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "con",10);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "dex",16);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "int",11);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "wis",12);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "pow",11);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    snprintf(buf, MAX_BUF, "%s %d", "cha",10);
    SockList_AddChar(&sl, strlen(buf)+1);
    SockList_AddString(&sl, buf);
    SockList_AddChar(&sl, 0);

    SockList_Send(&sl, csocket.fd);

    //keybindings_init(char_name);
}



void
on_button_cc_done(GtkButton *button, gpointer user_data)
{
    printf("on_button_cc_done\n");
    send_create_player_to_server();
    /*
    if (character_data_ok()) {
         gtk_label_set_text(GTK_LABEL(label_cc_status_update),
                           "Sending new character information to server");
        show_window(WINDOW_CREATE_CHARACTER);
        send_create_player_to_server();
    }
    */
}



void init_create_character_window() {
    char tmpbuf[80];
    int i;
    GtkTextIter iter;
    GtkCellRenderer *renderer;

    create_character_window = GTK_WIDGET(gtk_builder_get_object(dialog_xml, "create_character_window"));
    gtk_window_set_transient_for(GTK_WINDOW(create_character_window), GTK_WINDOW(window_root));


    button_cc_done = GTK_WIDGET(gtk_builder_get_object(dialog_xml,"button_cc_done"));
    button_cc_cancel = GTK_WIDGET(gtk_builder_get_object(dialog_xml,"button_cc_cancel"));
    button_choose_starting_map = GTK_WIDGET(gtk_builder_get_object(dialog_xml,"button_choose_starting_map"));
    label_cc_status_update = GTK_WIDGET(gtk_builder_get_object(dialog_xml,"label_cc_status_update"));
    g_signal_connect ((gpointer) button_cc_done, "clicked",
                      G_CALLBACK (on_button_cc_done), NULL);
  
   
}

/*
void
on_button_create_new_char_clicked(GtkButton *button, gpointer user_data) {
    printf("on_button_create_new_char_clicked\n");   
    create_new_character();
}

void
on_button_new_char_cancel_clicked(GtkButton *button, gpointer user_data) {
    gtk_widget_hide(new_character_window);
    gtk_widget_show(choose_char_window);
}
*/
/*
static void init_new_character_window() {
    new_character_window =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "create_character_window"));
    
     gtk_window_set_transient_for(
        GTK_WINDOW(new_character_window), GTK_WINDOW(window_root));


    button_create_new_char =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_create_character"));
          button_new_char_cancel =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_cc_cancel"));
 
  entry_new_character_name =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "cc_entry_new_character_name"));

    g_signal_connect((gpointer) button_create_new_char, "clicked",
                     G_CALLBACK(on_button_create_new_char_clicked), NULL);
       g_signal_connect((gpointer) button_new_char_cancel, "clicked",
                     G_CALLBACK(on_button_new_char_cancel_clicked), NULL);
 
 
}
*/
 static void init_choose_char_window() {
    GtkTextIter end;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    choose_char_window =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "choose_character_window"));

    button_create_character =
        GTK_WIDGET(gtk_builder_get_object(dialog_xml, "button_create_character"));
    g_signal_connect((gpointer) button_create_character, "clicked",
                     G_CALLBACK(on_button_create_character_clicked), NULL);
  
}

void start_login(int method) {
    serverloginmethod = method;
    if (!start_login_has_init) {
        init_login_window();

        init_create_account_window();

        init_choose_char_window();

        init_create_character_window();
  //      init_new_character_window();
/*
        init_add_character_window();

        init_choose_char_window();

        init_create_account_window();

        init_new_character_window();

        init_account_password_window();

        start_login_has_init = 1;
        update_login_info(INFO_NEWS);
        update_login_info(INFO_RULES);
        update_login_info(INFO_MOTD);
        */
    }
/*
    gtk_entry_set_text(GTK_ENTRY(entry_account_name), "");
    gtk_entry_set_text(GTK_ENTRY(entry_account_password), "");
    gtk_widget_grab_focus(entry_account_name);
    gtk_widget_show(login_window);
*/
    gtk_widget_show(login_window);
}


void choose_character_init() {
    gtk_widget_hide(login_window);
    //gtk_widget_hide(add_character_window);
    //gtk_widget_hide(create_account_window);
    gtk_widget_show(choose_char_window);

    gtk_list_store_clear(character_store);
}


void AccountPlayersCmd(char *buf, int len)
{
    choose_character_init();
}

void SetupCmd(char *buf, int len)
{
    int s;
    char *cmd, *param;
    printf("SetupCmd: %s\n", buf);
    for (s = 0; ; ) {
        if (s >= len) { /* Ugly, but for secure...*/
            break;
        }

        cmd = &buf[s];

        /* Find the next space, and put a null there */
        for (; buf[s] && buf[s] != ' '; s++)
            ;
        buf[s++] = 0;
        while (buf[s] == ' ') {
            s++;
        }
        if (s >= len) {
            break;
        }

        param = &buf[s];

        for (; buf[s] && buf[s] != ' '; s++)
            ;
        buf[s++] = 0;
        while (s < len && buf[s] == ' ') {
            s++;
        }
        printf("SETUP_CMD: [%s]\n",cmd);
        printf("SETUP_PARAM: [%s]\n",param);
        if (!strcmp(cmd, "loginmethod")) {
            int method = atoi(param);
            if (method) {
                start_login(method);
            }
        }
           
    }
}

void FailureCmd(char *buf, int len)
{
    char *cp;
    cp = strchr(buf,' ');
    if (!cp) {
        return;
    }
    *cp = 0;
    cp++;

    if (!strcmp(buf,"accountlogin")) {
         printf("FailureCmd> %s:%s",buf, cp);
        //account_login_failure(cp);
    } else if (!strcmp(buf,"accountnew")) {
         printf("FailureCmd> %s:%s",buf, cp);
        //account_creation_failure(cp);
    } else if (!strcmp(buf,"accountaddplayer")) {
         printf("FailureCmd> %s:%s",buf, cp);
        //account_add_character_failure(cp);
    } else if (!strcmp(buf,"createplayer")) {
        //create_new_character_failure(cp);
        printf("FailureCmd> %s:%s",buf, cp);
    } else if (!strcmp(buf, "accountpw")) {
        printf("FailureCmd> %s:%s",buf, cp);
        //account_change_password_failure(cp);
    } else
        printf("FailureCmd> Got a failure response we can not handle: %s:%s",buf, cp);
}

int GetInt_String(const unsigned char *data)
{
    return ((data[0]<<24) + (data[1]<<16) + (data[2]<<8) + data[3]);
}

short GetShort_String(const unsigned char *data)
{
    return ((data[0]<<8)+data[1]);
}

sint64 GetInt64_String(const unsigned char *data)
{
#ifdef WIN32
    return (((sint64)data[0]<<56) + ((sint64)data[1]<<48) +
            ((sint64)data[2]<<40) + ((sint64)data[3]<<32) +
            ((sint64)data[4]<<24) + ((sint64)data[5]<<16) + ((sint64)data[6]<<8) + (sint64)data[7]);
#else
    return (((uint64)data[0]<<56) + ((uint64)data[1]<<48) +
            ((uint64)data[2]<<40) + ((uint64)data[3]<<32) +
            ((uint64)data[4]<<24) + (data[5]<<16) + (data[6]<<8) + data[7]);
#endif
}


void new_player (long tag, char *name, long weight, long face)
{
    Spell *spell, *spnext;

    cpl.ob->tag    = tag;
    cpl.ob->nrof   = 1;
   // copy_name(cpl.ob->d_name, name);

    if (strlen(name) != 0) {
        //keybindings_init(name);
    }

    cpl.ob->weight = (float) weight / 1000;
    cpl.ob->face   = face;

    if (cpl.spelldata) {
        for (spell = cpl.spelldata; spell; spell = spnext) {
            spnext = spell->next;
            free(spell);
        }
        cpl.spelldata = NULL;
    }

}


static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    //
    memcpy(data, data_cp + data_start, length);
    data_start += length;
}

//Convert png data to RGB888 data and output width,height
uint8 *png_to_data(uint8 *data, int len, uint32 *width, uint32 *height)
{
    //data and len input, get width and height
    uint8 *pixels=NULL;
    static png_bytepp rows=NULL;
    static int rows_byte=0;

    png_structp png_ptr;
    png_infop   info_ptr;
    int bit_depth, color_type, interlace_type, y;

    data_len=len;
    data_cp = data;
    data_start=0;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }
    png_set_read_fn(png_ptr, NULL, user_read_data);
    png_read_info(png_ptr, info_ptr);
    //
    *width = png_get_image_width(png_ptr, info_ptr);
    *height = png_get_image_height(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    interlace_type = png_get_interlace_type(png_ptr, info_ptr);
    //
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) {
        //indexed images convert to RGB
        png_set_expand(png_ptr);
    }else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        // grayscale images convert to RGB
        png_set_expand (png_ptr);
    } else if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        // If we have transparency header, convert it to alpha channel 
        png_set_expand(png_ptr);
    } else if (bit_depth < 8) {
        // If we have < 8 scale it up to 8 
        png_set_expand(png_ptr);
    }
    //
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        //convert gray to RGB888
        png_set_gray_to_rgb(png_ptr);
    }
    // If interlaced, handle that 
    if (interlace_type != PNG_INTERLACE_NONE) {
        png_set_interlace_handling(png_ptr);
    }
    // pad it to 4 bytes to make processing easier 
    if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
        png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);
    }
    // Update the info the reflect our transformations
    png_read_update_info(png_ptr, info_ptr);

    *width = png_get_image_width(png_ptr, info_ptr);
    *height = png_get_image_height(png_ptr, info_ptr);
    // malloc pixels and fill it then return 
    pixels = (uint8*)malloc(*width **height * 4);
    if (!pixels) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        exit(1);
    }
    // calculate png_bytepp struct rows, the size of a line
    if (rows_byte == 0) {
        rows = (png_bytepp)malloc(sizeof(png_byte*)*(*height));
        rows_byte = *height;
    } else if (*height > rows_byte) {
        rows = (png_bytepp)realloc(rows, sizeof(png_byte *)*(*height));
        if (rows == NULL) {
            exit(EXIT_FAILURE);
        }
        rows_byte = *height;
    }
    if (!rows) {
        png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
        free(pixels);
        return NULL;
    }
    for (y=0; y<*height; y++) {
        rows[y] = pixels + y **width * 4;
    }
    //png_read_image need png_ptr and rows
    png_read_image(png_ptr, rows);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return pixels;
}

static void free_pixmap(PixmapInfo *pi)
{
    if (pi->icon_image) {
        g_object_unref(pi->icon_image);
    }
    if (pi->icon_mask) {
        g_object_unref(pi->icon_mask);
    }
    if (pi->map_mask) {
        gdk_pixmap_unref(pi->map_mask);
    }
    if (pi->map_image) {
            gdk_pixmap_unref(pi->map_image);
    }
}

//see Png.c
int rgba_to_gdkpixbuf(uint8 *data,int width, int height,GdkPixbuf **pix)
{
    int rowstride;
    guchar  *pixels, *p;
    int x,y;

    *pix  = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    rowstride =  gdk_pixbuf_get_rowstride(*pix);
    pixels = gdk_pixbuf_get_pixels(*pix);
    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            p = pixels + y * rowstride + x * 4;
            p[0] = data[4*(x + y * width)];
            p[1] = data[4*(x + y * width) + 1 ];
            p[2] = data[4*(x + y * width) + 2 ];
            p[3] = data[4*(x + y * width) + 3 ];
        }
    }
    return 0;
}

//Give rgb data and convert to GdkPixbuf* data , then save to pi->icon_image
static void create_icon_image(uint8 *data, PixmapInfo *pi, int pixmap_num)
{
    pi->icon_mask = NULL;
    if (rgba_to_gdkpixbuf(data, pi->icon_width, pi->icon_height, (GdkPixbuf**)&pi->icon_image)) {
        printf("Unable to create scaled image, dest num = %d\n", pixmap_num);
    }
}

int rgba_to_gdkpixmap(GdkWindow *window, uint8 *data,int width, int height,
                      GdkPixmap **pix, GdkBitmap **mask, GdkColormap *colormap)
{
    GdkGC       *gc, *gc_alpha;
    int         has_alpha=0, alpha;
    GdkColor  scolor;
    int x,y;
    //malloc GdkPixmap *pix by window and width, height
    *pix = gdk_pixmap_new(window, width, height, -1);

    gc=gdk_gc_new(*pix);
    gdk_gc_set_function(gc, GDK_COPY);

    *mask=gdk_pixmap_new(window, width, height,1);
    gc_alpha=gdk_gc_new(*mask);


    //draw rectangle for GdkBitmap *mask first
    scolor.pixel=1;
    gdk_gc_set_foreground(gc_alpha, &scolor);
    gdk_draw_rectangle(*mask, gc_alpha, 1, 0, 0, width, height);

    scolor.pixel=0;
    gdk_gc_set_foreground(gc_alpha, &scolor);

    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            alpha = data[(y * width + x) * 4 +3];
            // Transparent bit 
            if (alpha==0) {
                //alpha=0, use gdk_draw_point() with scolor.pixel=0 to Transparent it
                gdk_draw_point(*mask, gc_alpha, x, y);
                has_alpha=1;
            }
        }
    }
    //give RGB888 data, width, height , then draw to GdkPixmap *pix
    gdk_draw_rgb_32_image(*pix, gc,  0, 0, width, height, GDK_RGB_DITHER_NONE, data, width*4);
    
    if (!has_alpha) {
        //if !has_alpha=1 means no Transparent to use
        gdk_pixmap_unref(*mask);
        *mask = NULL;
    }
    gdk_gc_destroy(gc_alpha);
    gdk_gc_destroy(gc);
    return 0;
}

static void create_map_image(uint8 *data, PixmapInfo *pi)
{
    pi->map_image = NULL;
    pi->map_mask = NULL;
    rgba_to_gdkpixmap(window_root->window, data, pi->map_width, pi->map_height,
                          (GdkPixmap**)&pi->map_image, (GdkBitmap**)&pi->map_mask,
                          gtk_widget_get_colormap(window_root));
}


uint8 *rescale_rgba_data(uint8 *data, int *width, int *height, int scale)
{
    static int xrow[BPP * MAX_IMAGE_WIDTH], yrow[BPP*MAX_IMAGE_HEIGHT];
    static uint8 *nrows[MAX_IMAGE_HEIGHT];

    /* Figure out new height/width */
    int new_width = *width  * scale / RATIO, new_height = *height * scale / RATIO;

    int sourcerow=0, ytoleft, ytofill, xtoleft, xtofill, dest_column=0, source_column=0, needcol,
        destrow=0;
    int x,y;
    uint8 *ndata;
    uint8 r,g,b,a;

    if (*width > MAX_IMAGE_WIDTH || new_width > MAX_IMAGE_WIDTH
            || *height > MAX_IMAGE_HEIGHT || new_height > MAX_IMAGE_HEIGHT) {
        exit(0);
    }

    /* clear old values these may have */
    memset(yrow, 0, sizeof(int) **height * BPP);

    ndata = (uint8*)malloc(new_width * new_height * BPP);

    for (y=0; y<new_height; y++) {
        nrows[y] = (png_bytep) (ndata + y * new_width * BPP);
    }

    ytoleft = scale;
    ytofill = RATIO;

    for (y=0,sourcerow=0; y < new_height; y++) {
        memset(xrow, 0, sizeof(int) **width * BPP);
        while (ytoleft < ytofill) {
            for (x=0; x< *width; ++x) {
                /* Only want to copy the data if this is not a transperent pixel.
                 * If it is transparent, the color information is has is probably
                 * bogus, and blending that makes the results look worse.
                 */
                if (data[(sourcerow **width + x)*BPP+3] > 0 ) {
                    yrow[x*BPP] += ytoleft * data[(sourcerow **width + x)*BPP]/RATIO;
                    yrow[x*BPP+1] += ytoleft * data[(sourcerow **width + x)*BPP+1]/RATIO;
                    yrow[x*BPP+2] += ytoleft * data[(sourcerow **width + x)*BPP+2]/RATIO;
                }
                /* Alpha is a bit special - we don't want to blend it -
                 * we want to take whatever is the more opaque value.
                 */
                if (data[(sourcerow **width + x)*BPP+3] > yrow[x*BPP+3]) {
                    yrow[x*BPP+3] = data[(sourcerow **width + x)*BPP+3];
                }
            }
            ytofill -= ytoleft;
            ytoleft = scale;
            if (sourcerow < *height) {
                sourcerow++;
            }
        }

        for (x=0; x < *width; ++x) {
            if (data[(sourcerow **width + x)*BPP+3] > 0 ) {
                xrow[x*BPP] = yrow[x*BPP] + ytofill * data[(sourcerow **width + x)*BPP] / RATIO;
                xrow[x*BPP+1] = yrow[x*BPP+1] + ytofill * data[(sourcerow **width + x)*BPP+1] / RATIO;
                xrow[x*BPP+2] = yrow[x*BPP+2] + ytofill * data[(sourcerow **width + x)*BPP+2] / RATIO;
            }
            if (data[(sourcerow **width + x)*BPP+3] > xrow[x*BPP+3]) {
                xrow[x*BPP+3] = data[(sourcerow **width + x)*BPP+3];
            }
            yrow[x*BPP]=0;
            yrow[x*BPP+1]=0;
            yrow[x*BPP+2]=0;
            yrow[x*BPP+3]=0;
        }

        ytoleft -= ytofill;
        if (ytoleft <= 0) {
            ytoleft = scale;
            if (sourcerow < *height) {
                sourcerow++;
            }
        }

        ytofill = RATIO;
        xtofill = RATIO;
        dest_column = 0;
        source_column=0;
        needcol=0;
        r=0;
        g=0;
        b=0;
        a=0;

        for (x=0; x< *width; x++) {
            xtoleft = scale;

            while (xtoleft >= xtofill) {
                if (needcol) {
                    dest_column++;
                    r=0;
                    g=0;
                    b=0;
                    a=0;
                }

                if (xrow[source_column*BPP+3] > 0) {
                    r += xtofill * xrow[source_column*BPP] / RATIO;
                    g += xtofill * xrow[1+source_column*BPP] / RATIO;
                    b += xtofill * xrow[2+source_column*BPP] / RATIO;
                }
                if (xrow[3+source_column*BPP] > a) {
                    a = xrow[3+source_column*BPP];
                }

                nrows[destrow][dest_column * BPP] = r;
                nrows[destrow][1+dest_column * BPP] = g;
                nrows[destrow][2+dest_column * BPP] = b;
                nrows[destrow][3+dest_column * BPP] = a;
                xtoleft -= xtofill;
                xtofill = RATIO;
                needcol=1;
            }

            if (xtoleft > 0 ) {
                if (needcol) {
                    dest_column++;
                    r=0;
                    g=0;
                    b=0;
                    a=0;
                    needcol=0;
                }

                if (xrow[3+source_column*BPP] > 0) {
                    r += xtoleft * xrow[source_column*BPP] / RATIO;
                    g += xtoleft * xrow[1+source_column*BPP] / RATIO;
                    b += xtoleft * xrow[2+source_column*BPP] / RATIO;
                }
                if (xrow[3+source_column*BPP] > a) {
                    a = xrow[3+source_column*BPP];
                }

                xtofill -= xtoleft;
            }
            source_column++;
        }

        if (xtofill > 0 ) {
            source_column--;
            if (xrow[3+source_column*BPP] > 0) {
                r += xtofill * xrow[source_column*BPP] / RATIO;
                g += xtofill * xrow[1+source_column*BPP] / RATIO;
                b += xtofill * xrow[2+source_column*BPP] / RATIO;
            }
            if (xrow[3+source_column*BPP] > a) {
                a = xrow[3+source_column*BPP];
            }
        }

        /* Not positve, but without the bound checking for dest_column,
         * we were overrunning the buffer.  My guess is this only really
         * showed up if when the images are being scaled - there is probably
         * something like half a pixel left over.
         */
        if (!needcol && (dest_column < new_width)) {
            nrows[destrow][dest_column * BPP] = r;
            nrows[destrow][1+dest_column * BPP] = g;
            nrows[destrow][2+dest_column * BPP] = b;
            nrows[destrow][3+dest_column * BPP] = a;
        }
        destrow++;
    }
    *width = new_width;
    *height = new_height;
    return ndata;
}

//give pixmap_num and rgb data with width,height and output Cache_Entry* ce
int create_and_rescale_image_from_data(Cache_Entry *ce, int pixmap_num, uint8 *rgba_data, int width, int height)
{
    int nx, ny, iscale, factor;
    uint8 *png_tmp;
    PixmapInfo  *pi;

    if (pixmap_num <= 0 || pixmap_num >= MAXPIXMAPNUM) {
        return 1;
    }

    if (pixmaps[pixmap_num] != pixmaps[0]) {
            free_pixmap(pixmaps[pixmap_num]);
            free(pixmaps[pixmap_num]);        
        pixmaps[pixmap_num] = pixmaps[0];
    }

    pi = calloc(1, sizeof(PixmapInfo));
    iscale = 100;
  
    if (width > DEFAULT_IMAGE_SIZE || height>DEFAULT_IMAGE_SIZE) {
        int ts = 100;

        factor = width / DEFAULT_IMAGE_SIZE;
        if (factor >= MAX_ICON_SPACES) {
            factor = MAX_ICON_SPACES - 1;
        }
        if (icon_rescale_factor[factor] < ts) {
            ts = icon_rescale_factor[factor];
        }

        factor = height / DEFAULT_IMAGE_SIZE;
        if (factor >= MAX_ICON_SPACES) {
            factor = MAX_ICON_SPACES - 1;
        }
        if (icon_rescale_factor[factor] < ts) {
            ts = icon_rescale_factor[factor];
        }
        printf("factor=%d\n",factor);
        printf("iscale=%d\n",iscale);
        iscale = ts * 100 / 100;
    }

    /* In all cases, the icon images are in native form. */
    if (iscale != 100) {
        nx=width;
        ny=height;
        png_tmp = rescale_rgba_data(rgba_data, &nx, &ny, iscale);
        pi->icon_width = nx;
        pi->icon_height = ny;
        create_icon_image(png_tmp, pi, pixmap_num);
        free(png_tmp);
    } else {
        pi->icon_width = width;
        pi->icon_height = height;
        create_icon_image(rgba_data, pi, pixmap_num);
    }

        pi->map_width = width;
        pi->map_height = height;
        png_tmp = rgba_data;
        create_map_image(png_tmp, pi);

    /*
     * Not ideal, but if it is missing the map or icon image, presume something
     * failed.  However, opengl doesn't set the map_image, so if using that
     * display mode, don't make this check.
     */
    if (!pi->icon_image || (!pi->map_image)) {
        free_pixmap(pi);
        free(pi);
        return 1;
    }
    if (ce) {
        ce->image_data = pi;
    }
    pixmaps[pixmap_num] = pi;

    return 0;
}
//
void display_newpng(int face, uint8 *buf, int buflen, int setnum) {
    uint8   *pngtmp;
    uint32 width, height;
    Cache_Entry *ce = NULL;

    pngtmp = png_to_data(buf, buflen, &width, &height);
    if (create_and_rescale_image_from_data(ce, face, pngtmp, width, height)) {
        printf("create_and_rescale_image_from_data() error\n");
    }
    free(pngtmp);
}

// fill pixmaps[pnum]
void Image2Cmd(uint8 *data,  int len) 
{
    //sizeof(uint8) = 1, sizeof(int) = 4
    //pnum means face, which is image number
    int pnum, plen;
    uint8 setnum;
    //pnum=data[0]*256^3 + data[0]*256^2 + data[1]*256+data[3]
    pnum = GetInt_String(data);
    setnum = data[4];
    plen = GetInt_String(data + 5);
    //pnum:0~3, setnum:4, plen:5~8
    display_newpng(pnum, data + 9, plen, setnum);
    printf("Fill pixmaps[%d] and len: %d\n", pnum, plen);
}

void reset_player_data() {
    int i;

    for (i = 0; i < MAX_SKILL; i++) {
        cpl.stats.skill_exp[i] = 0;
        cpl.stats.skill_level[i] = 0;
    }
}

void PlayerCmd(unsigned char *data, int len)
{
    char name[MAX_BUF];
    int tag, weight, face, i = 0, nlen;

    reset_player_data();
    tag = GetInt_String(data);
    i += 4;
    weight = GetInt_String(data+i);
    i += 4;
    face = GetInt_String(data+i);
    i += 4;
    nlen = data[i++];
    memcpy(name, (const char*)data+i, nlen);
    name[nlen] = '\0';
    i += nlen;

    if (i != len) {
        //LOG(LOG_WARNING, "common::PlayerCmd", "lengths do not match (%d!=%d)", len, i);
    }
    printf("PlayerCmd: t[%d],w[%d],f[%d],n[%s]\n", tag,weight,face,name);
    //new_player(tag, name, weight, face);
}

static void common_item_command(uint8 *data, int len)
{

    int weight, loc, tag, face, flags, pos = 0, nlen, anim, nrof, type;
    uint8 animspeed;
    char name[MAX_BUF];

    loc = GetInt_String(data);
    pos += 4;
    printf("Item2Cmd: loc[%d],",loc);
    if (pos == len) {
        return;
    } else if (loc < 0) { 
        return;
    } else {
        while (pos < len) {
            tag = GetInt_String(data+pos);
            pos += 4;
            flags = GetInt_String(data+pos);
            pos += 4;
            weight = GetInt_String(data+pos);
            pos += 4;
            face = GetInt_String(data+pos);
            pos += 4;
            nlen = data[pos++];
            memcpy(name, (char*)data+pos, nlen);
            pos += nlen;
            name[nlen] = '\0';
            anim = GetShort_String(data+pos);
            pos += 2;
            animspeed = data[pos++];
            nrof = GetInt_String(data+pos);
            pos += 4;
            type = GetShort_String(data+pos);
            pos += 2;
            printf("tag[%d],flags[%d],weight[%d],face[%d],nlen[%d],name[%s],anim[%d],animspeed[%d],nrof[%d],type[%d]\n",
                tag,flags,weight,face,nlen,name,anim,animspeed,nrof,type
                );
            //update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, type);
            //item_actions(locate_item(tag));
        }
        if (pos > len) {
            
        }
    }
}

void Item2Cmd(unsigned char *data, int len)
{
    common_item_command(data, len);
}

void reset_map(void)
{
}

void display_map_newmap(void)
{
    reset_map();
}

void mapdata_newmap(void)
{
    int x, y;

    /* Clear the_map.cells[]. */
    for (x = 0; x < FOG_MAP_SIZE; x++) {
        CLEAR_CELLS(x, 0, FOG_MAP_SIZE);
        for (y = 0; y < FOG_MAP_SIZE; y++) {
            the_map.cells[x][y].need_update = 1;
        }
    }

    /* Clear bigfaces[]. */
  //  while (bigfaces_head != NULL) {
  //      expand_clear_bigface_from_layer(bigfaces_head->x, bigfaces_head->y, bigfaces_head->layer, 0);
  //  }

    display_map_newmap();
}


void NewmapCmd(unsigned char *data, int len)
{
    (void)data; /* __UNUSED__ */
    (void)len; /* __UNUSED__ */

    mapdata_newmap();
}



void StatsCmd(unsigned char *data, int len)
{
    printf("StatsCmd: \n");
    int i = 0, c, redraw = 0;
    sint64 last_exp;
     while (i < len) {
        c = data[i++];
        if (c >= CS_STAT_RESIST_START && c <= CS_STAT_RESIST_END) {
            //c>100 and c<117
            cpl.stats.resists[c-CS_STAT_RESIST_START] = GetShort_String(data+i);
            printf("update resist: c[%d]<--%d\n",c,GetShort_String(data+i));
            i += 2;
            cpl.stats.resist_change = 1;
        } else if (c >= CS_STAT_SKILLINFO && c < (CS_STAT_SKILLINFO+CS_NUM_SKILLS)) {
            ////c >=140 and c < 140+
            cpl.stats.skill_level[c-CS_STAT_SKILLINFO] = data[i++];
            last_exp = cpl.stats.skill_exp[c-CS_STAT_SKILLINFO];
            cpl.stats.skill_exp[c-CS_STAT_SKILLINFO] = GetInt64_String(data+i);
             printf("update skill_exp: c[%d]<--%ld\n",c,GetInt64_String(data+i));
            //
            //use_skill(c-CS_STAT_SKILLINFO);
            //
            if (last_exp == 0 && cpl.stats.skill_exp[c-CS_STAT_SKILLINFO]) {
                redraw = 1;
            }
            i += 8;
        } else {
            switch (c) {
            case 1:
                cpl.stats.hp = GetShort_String(data+i);
                printf("update stat: c[%d]<--%d\n",c,GetShort_String(data+i));
                i += 2;
                break;
            default:
                printf("update stat: c[%d]<--%d\n",c,GetShort_String(data+i));
                 i += 2;
                break;
            }
        }

    }
    // draw_stats(redraw);
    //draw_message_window(0);
}


void Map2Cmd(unsigned char *data, int len)
{
    printf("Map2Cmd: ");
    int mask, x, y, pos = 0, space_len, value;
    uint8 type;
    //0xFFFF  0xFF
    // mask   type
    // mask got x,y
    // 
    //display_map_startupdate();
    /* Not really using map1 protocol, but some draw logic differs from the
     * original draw logic, and map2 is closest.
     */
    while (pos < len) {
        // short = 2 bytes
        // int = 4 bytes
        mask = GetShort_String(data+pos);
        printf("Mask[0x%X]",mask);
        pos += 2;
        // X/512 & (63) - 15
        // X/8 & (63) - 15
        // mask:
        // xxxxxxxxxxxxxxx
        // |  X |  Y  |  |
        //
        // (type:     [value]   [opt]   [opt]) , (), () ... 
        // xxx1xxxx xxxxxxxx xxxxxxxx xxxxxxxx
        // | |||_____layer:0~0xf, next 0xFF value
        // ---|___1:set layer
        //  |_space_len, 1: only type
        //               2: type value
        //               3: type value[bit15:1->Anim,0->smooth] opt[Anim or smooth]
        //               4: type value                          opt[Anim] opt[smooth]
        // xxx0xxxx
        //     |__|
        //      |___0x00: CLEAR
        //          0x01: DARKNESS, next 0xFF:value
        //
        x = ((mask>>10)&0x3f)-MAP2_COORD_OFFSET;
        y = ((mask>>4)&0x3f)-MAP2_COORD_OFFSET;
        printf("->(%d,%d)",x,y);
        /* This is a scroll then.  Go back and fetch another coordinate */
        if (mask&0x1) {
            //mapdata_scroll(x, y);
            printf("\n");
            continue;
        }

        if (x<0) {
            //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got negative x!");
            x = 0;
        } else if (x >= MAX_VIEW) {
            //X>=64
            //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got x >= MAX_VIEW!");
            x = MAX_VIEW - 1;
        }

        if (y<0) {
            //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got negative y!");
            y = 0;
        } else if (y >= MAX_VIEW) {
            //Y>=64
            //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got y >= MAX_VIEW!");
            y = MAX_VIEW - 1;
        }
        assert(0 <= x && x < MAX_VIEW);
        assert(0 <= y && y < MAX_VIEW);
        //mapdata_clear_old(x, y);
        while (pos < len) {
            type = data[pos++];
            printf(" type[%d]",type);
            /* type == 255 means nothing more for this space */
            if (type == 255) {
                //mapdata_set_check_space(x, y);
                printf("\n");
                break;
            }
            space_len = type>>5;
            printf("->s_len[%d]",space_len);
            type &= 0x1f;
            if (type == MAP2_TYPE_CLEAR) {
                //mapdata_clear_space(x, y);
                printf("\n");
                continue;
            } else if (type == MAP2_TYPE_DARKNESS) {
                value = data[pos++];
                printf(" (DARKNESS)v[%d]",value);
                //mapdata_set_darkness(x, y, value);
                printf("\n");
                continue;
            } else if (type >= MAP2_LAYER_START && type < MAP2_LAYER_START+MAXLAYERS) {
                int layer, opt;
                layer = type&0xf;
                printf("->(LAYER)layer[%d]",layer);
                if (layer < 0) {
                    //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got negative layer!");
                    layer = 0;
                } else if (layer >= MAXLAYERS) {
                    //LOG(LOG_WARNING, "commands.c::Map2Cmd", "got layer >= MAXLAYERS!");
                    layer = MAXLAYERS - 1;
                }
                assert(0 <= layer && layer < MAXLAYERS);
                value = GetShort_String(data+pos);
                printf(" v[%d]",value);
                pos += 2;
                if (!(value&FACE_IS_ANIM)) {
                    //mapdata_set_face_layer(x, y, value, layer);
                }
                if (space_len > 2) {
                    opt = data[pos++];
                    printf(" o1[%d]",opt);
                    if (value&FACE_IS_ANIM) {
                        /* Animation speed */
                        //mapdata_set_anim_layer(x, y, value, opt, layer);
                    } else {
                        /* Smooth info */
                        //mapdata_set_smooth(x, y, opt, layer);
                    }
                }
                /* Currently, if 4 bytes, must be a smooth byte */
                if (space_len > 3) {
                    opt = data[pos++];
                    printf(" o2[%d]",opt);
                    //mapdata_set_smooth(x, y, opt, layer);
                }
                printf("\n");
                continue;

            }


        }
    }
     mapupdatesent = 0;
    //display_map_doneupdate(FALSE, FALSE);
}

int SockList_ReadPacket(int fd, SockList *sl, int len)
{
    int stat, toread;

    /* We already have a partial packet */
    if (sl->len<2) {
#ifndef WIN32
        do {
            stat=read(fd, sl->buf + sl->len, 2-sl->len);
        } while ((stat==-1) && (errno==EINTR));
#else
        do {
            stat=recv(fd, sl->buf + sl->len, 2-sl->len, 0);
        } while ((stat==-1) && (WSAGetLastError()==EINTR));
#endif

        if (stat<0) {
            /* In non blocking mode, EAGAIN is set when there is no data
             * available.
             */
#ifndef WIN32
            if (errno!=EAGAIN && errno!=EWOULDBLOCK)
#else
            if (WSAGetLastError()!=EAGAIN && WSAGetLastError()!=WSAEWOULDBLOCK)
#endif
            {
                printf("ReadPacket got error %d, returning -1",errno);
                return -1;
            }
            return 0;   /*Error */
        }
        if (stat==0) {
            return -1;
        }

        sl->len += stat;
#ifdef CS_LOGSTATS
        cst_tot.ibytes += stat;
        cst_lst.ibytes += stat;
#endif
        if (stat<2) {
            return 0;    /* Still don't have a full packet */
        }
    }

    /* Figure out how much more data we need to read.  Add 2 from the
     * end of this - size header information is not included.
     */
    toread = 2+(sl->buf[0] << 8) + sl->buf[1] - sl->len;
    if ((toread + sl->len) > len) {
        printf("Want to read more bytes than will fit in buffer.\n");
        /* return error so the socket is closed */
        return -1;
    }
    do {
#ifndef WIN32
        do {
            stat = read(fd, sl->buf+ sl->len, toread);
        } while ((stat<0) && (errno==EINTR));
#else
        do {
            stat = recv(fd, sl->buf+ sl->len, toread, 0);
        } while ((stat<0) && (WSAGetLastError()==EINTR));
#endif
        if (stat<0) {

#ifndef WIN32
            if (errno!=EAGAIN && errno!=EWOULDBLOCK)
#else
            if (WSAGetLastError()!=EAGAIN && WSAGetLastError()!=WSAEWOULDBLOCK)
#endif
            {
                printf("ReadPacket got error %d, returning 0\n",errno);
            }
            return 0;       /*Error */
        }
        if (stat==0) {
            return -1;
        }
        sl->len += stat;

#ifdef CS_LOGSTATS
        cst_tot.ibytes += stat;
        cst_lst.ibytes += stat;
#endif
        toread -= stat;
        if (toread==0) {
            return 1;
        }

        if (toread < 0) {
           printf("SockList_ReadPacket: Read more bytes than desired.\n");
            return 1;
        }
    } while (toread>0);
    return 0;
}

void mapdata_set_size(int viewx, int viewy)
{
    mapdata_init();

    width = viewx;
    height = viewy;
    pl_pos.x = FOG_MAP_SIZE/2-width/2;
    pl_pos.y = FOG_MAP_SIZE/2-height/2;
}

void negotiate_connection(int sound)
{
    printf("call negotiate_connection\n");
    int tries;
    SendVersion(csocket);
    tries=0;
    while (csocket.cs_version==0) {
        DoClient(&csocket);
        if (csocket.fd == -1) {
            printf("csocket.fd = -1 , error\n");
            return;
        }
        usleep(10*1000);    /* 10 milliseconds */
        tries++;
        //printf("tries %d\n",tries);
        /* If we have't got a response in 10 seconds, bail out */
        if (tries > 1000) {
            printf("tries > 1000 close_server_connection()\n");
            close_server_connection();
            return;
        }
    }
    printf("csocket.cs_version: %d\n",csocket.cs_version);
     printf("csocket.sc_version: %d\n",csocket.sc_version);

   cs_print_string(csocket.fd,
                    "setup map2cmd 1 tick 1 sound2 %d darkness %d spellmon 1 spellmon 2 "
                    "faceset %d facecache %d want_pickup 1 loginmethod %d newmapcmd 1",
                    (0 >= 0) ? 3 : 0, CFG_LT_TILE ? 1 : 0,
                    1, FALSE, wantloginmethod);

    cs_print_string(csocket.fd, "requestinfo skill_info");
    cs_print_string(csocket.fd,"requestinfo exp_table");
    cs_print_string(csocket.fd,"requestinfo motd");
    cs_print_string(csocket.fd,"requestinfo news");
    cs_print_string(csocket.fd,"requestinfo rules");

    mapdata_set_size(11, 11);

    if (csocket.sc_version >= 1027) {
        int last_end=0, last_start=-99;
        cs_print_string(csocket.fd,"requestinfo image_info");
        requestinfo_sent = RI_IMAGE_INFO;
        replyinfo_status = 0;
        replyinfo_last_face = 0;
        //Main process receive package from server
        int count;
        do {
            DoClient(&csocket);
            if (csocket.fd == -1) {
                printf("csocket.fd=-1\n");
                return;
            }
            gtk_main_iteration();
            usleep(10*1000); 
            count++;
            //printf("count:%2d\n",count);
            if (count > 100) count = 0;
        } while (replyinfo_status != requestinfo_sent);
        printf("replyinfo_status != requestinfo_sent :Exit \n");
    }

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
        }else{
             printf("SERVER=%s\n",server);
             csocket.fd = init_connection(server, 13327);
             server = NULL;
             if (csocket.fd == -1) {
                printf("csocket.fd=-1\n");
                continue;
             }
        }
        negotiate_connection(0);
        printf("call event_loop()\n");
    	event_loop();
	}

  exit(EXIT_SUCCESS);
}