#include <stdio.h>
#include <gtk/gtk.h>
#include <glib.h>

GtkWidget *mainWindow;
GtkWidget *debugWindow;
GtkWidget *debugimage;
GdkPixbuf *pixBuff;
GdkPixbuf *debug_pixBuff;
GtkWidget *image;

GThread **tids;
GMutex mutex;


cairo_surface_t *badvisor;
/*
static gpointer draw_camera(gpointer data)
//static void realize_cb(GtkWidget *widget, gpointer data)
{
	for(;;)
	{
		g_mutex_lock(&mutex);
		//g_usleep(1000);
		//.....
		g_mutex_unlock(&mutex);
		g_thread_yield();
		if (quit_signal){
			break;
		}
	}
}

void check1_button_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkToggleButton *check = GTK_TOGGLE_BUTTON(widget);
	if (gtk_toggle_button_get_active(check))
	{
		cv::namedWindow("Debug window", cv::WINDOW_AUTOSIZE );
		tids[1] = g_thread_new("debugvideo", draw_debug, NULL);
	}else{
		
		 cv::destroyWindow("Debug window");
	}
}

void radio1_button_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkToggleButton *radio = GTK_TOGGLE_BUTTON(widget);
	if (gtk_toggle_button_get_active(radio))
	{
		printf("radio1 active\n");
	}
}

void radio2_button_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkToggleButton *radio = GTK_TOGGLE_BUTTON(widget);
	if (gtk_toggle_button_get_active(radio))
	{
		printf("radio2 active\n");
	}
}

void radio3_button_callback(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkToggleButton *radio = GTK_TOGGLE_BUTTON(widget);
	if (gtk_toggle_button_get_active(radio))
	{
		printf("radio3 active\n");
	}
}


void toggle1_button_callback(GtkWidget *widget, gpointer callback_data)
//(GtkWidget *widget, GdkEventButton *event, gpointer callback_data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		 enable_face_detect = true;
	}else{
		 enable_face_detect = false;
	}
}

*/

static gpointer draw_board(gpointer data)
//static void realize_cb(GtkWidget *widget, gpointer data)
{
		g_mutex_lock(&mutex);
		
		
		//g_usleep(1000);
		//.....
		g_mutex_unlock(&mutex);
}

static void do_drawing(cairo_t *cr)
{
/*
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_line_width(cr, 0.5);
  //draw...
  cairo_stroke(cr); 
  */
    cairo_set_source_surface(cr, badvisor, 100, 100);
	cairo_paint(cr); 
}

static gboolean draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  do_drawing(cr);

  return FALSE;
}

/*
static gboolean draw_event(GtkWidget *widget, cairo_t *cr)	
{
	tids[0] = g_thread_new("draw_board", draw_board, image);
	return 0;
}
*/

void quit_button_click_callback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data)
{
	//quit_signal = true;
	//g_thread_join(tids[0]);
	//g_thread_join(tids[1]);
	g_mutex_clear(&mutex);
    gtk_main_quit();
}

int main(int argc,char *argv[]){

	//建立tids[]矩陣
	tids = g_new(GThread*,2);
	//
	badvisor = cairo_image_surface_create_from_png("res/iXiangQi_55x55/badvisor.png");
	//Do gtk
	GtkWidget *drawing_area;
	gtk_init(&argc, &argv);
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainWindow), "小程式");
	gtk_container_set_border_width(GTK_CONTAINER(mainWindow), 1);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_set_size_request(mainWindow, 640, 480);
	//使用grid
	GtkWidget *grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(mainWindow),grid);
    gtk_widget_show(grid);
	//標籤
	GtkWidget *labellist=gtk_label_new("列表");
    gtk_grid_attach((GtkGrid*)grid,labellist,0,0,1,1);
	gtk_widget_show(labellist);
	//quit按鈕
	GtkWidget *quit_button = gtk_button_new_with_label("Quit");
	gtk_grid_attach((GtkGrid*)grid,quit_button,0,9,1,1);
	gtk_widget_show(quit_button);
	g_signal_connect(G_OBJECT(quit_button), "button_press_event", G_CALLBACK(quit_button_click_callback), NULL);
	
	//建立放影像的box
	GtkWidget * box = gtk_event_box_new();
	gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 10);
	gtk_widget_show(box);	
	//建立影像
	image = gtk_image_new_from_file ("res/iXiangQi_55x55/chessboard.png");
	//gtk_widget_set_size_request(image, frame.cols, frame.rows);
	gtk_container_add (GTK_CONTAINER(box), image);
	gtk_widget_show(image);
	g_signal_connect (G_OBJECT(image), "draw", G_CALLBACK(draw_event), image);
	
	/*
	//toggle1按鈕
	GtkWidget *toggle1 = gtk_toggle_button_new_with_label ("Face Detect");
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(toggle1), false);
	enable_face_detect = true;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle1), true);
	gtk_grid_attach((GtkGrid*)grid,toggle1,0,1,1,1);
	gtk_widget_show(toggle1);
	g_signal_connect (toggle1, "toggled", G_CALLBACK (toggle1_button_callback), NULL);
	
	GtkWidget *radio1 = gtk_radio_button_new_with_label(NULL, "haarcascade");
//	gtk_widget_show(radio1);
	GtkWidget *radio2 = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)) , "radio2");
//	gtk_widget_show(radio2);
	GtkWidget *radio3 = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)) , "radio3");
//	gtk_widget_show(radio3);
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), radio1, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), radio2, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), radio3, TRUE, TRUE, 5);
	GtkWidget *radio_frame = gtk_frame_new("Methods");
    gtk_container_add(GTK_CONTAINER(radio_frame), vbox);
	gtk_grid_attach((GtkGrid*)grid,radio_frame,0,2,1,1);
	g_signal_connect (radio1, "toggled", G_CALLBACK (radio1_button_callback), NULL);
	g_signal_connect (radio2, "toggled", G_CALLBACK (radio2_button_callback), NULL);
	g_signal_connect (radio3, "toggled", G_CALLBACK (radio3_button_callback), NULL);
	//設定預設為radio2
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio2), true);
	gtk_widget_show(radio_frame);
	gtk_widget_show(vbox);
	gtk_widget_show(radio1);
	gtk_widget_show(radio2);
	gtk_widget_show(radio3);
	//
	GtkWidget *check1 = gtk_check_button_new_with_label("check1");
	gtk_grid_attach((GtkGrid*)grid,check1,0,3,1,1);
	g_signal_connect (check1, "toggled", G_CALLBACK (check1_button_callback), NULL);
	gtk_widget_show(check1);
	//
	//建立放影像的box
	GtkWidget * box = gtk_event_box_new();
	gtk_grid_attach(GTK_GRID(grid), box, 1, 0, 1, 10);
	gtk_widget_show(box);	
	//建立影像
    cv::Mat *im = &rgb_frame;
	//pixBuff=gdk_pixbuf_new_from_data((guchar*)im->datastart,GDK_COLORSPACE_RGB,FALSE,8,im->size().width,im->size().height,(im->channels()*im->cols), NULL,NULL);
    //image = gtk_image_new_from_pixbuf(pixBuff);
	image = gtk_image_new();
	//gtk_widget_set_size_request(image, frame.cols, frame.rows);
	gtk_container_add (GTK_CONTAINER(box), image);
	gtk_widget_show(image);
	//
	//GtkWidget * debugbox = gtk_event_box_new();
	//gtk_grid_attach(GTK_GRID(grid), debugbox, 1, 10, 1, 1);
	//gtk_widget_show(debugbox);	
	//debugimage = gtk_image_new_from_pixbuf(pixBuff);
	//gtk_container_add (GTK_CONTAINER(debugbox), debugimage);
	//設定繪圖call back function
	//g_signal_connect (G_OBJECT(image), "realize", G_CALLBACK(realize_cb), image);
	//g_signal_connect (G_OBJECT(image), "draw", G_CALLBACK(draw_event), image);
	//
	
	 g_mutex_init(&mutex); 
	//g_mutex_lock(&mutex);
	//tids[1] = g_thread_new("video", draw_background, image);
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE );
	tids[0] = g_thread_new("video", draw_camera, image);
	//
	*/
	gtk_widget_show(mainWindow); 
	gtk_main();
	g_free(tids);
	printf("Finish\n");
	//Finish Gtk
	return 0;
}