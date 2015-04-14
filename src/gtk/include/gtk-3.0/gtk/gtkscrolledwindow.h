/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_SCROLLED_WINDOW_H__
#define __GTK_SCROLLED_WINDOW_H__

#include <gtk/gtkbin.h>

G_BEGIN_DECLS


#define GTK_TYPE_SCROLLED_WINDOW            (gtk_scrolled_window_get_type ())
#define GTK_SCROLLED_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SCROLLED_WINDOW, GtkScrolledWindow))
#define GTK_SCROLLED_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCROLLED_WINDOW, GtkScrolledWindowClass))
#define GTK_IS_SCROLLED_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SCROLLED_WINDOW))
#define GTK_IS_SCROLLED_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCROLLED_WINDOW))
#define GTK_SCROLLED_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SCROLLED_WINDOW, GtkScrolledWindowClass))


typedef struct _GtkScrolledWindow              GtkScrolledWindow;
typedef struct _GtkScrolledWindowPrivate       GtkScrolledWindowPrivate;
typedef struct _GtkScrolledWindowClass         GtkScrolledWindowClass;

struct _GtkScrolledWindow
{
  GtkBin container;

  GtkScrolledWindowPrivate *priv;
};

struct _GtkScrolledWindowClass
{
  GtkBinClass parent_class;

  gint scrollbar_spacing;

  /* Action signals for keybindings. Do not connect to these signals
   */

  /* Unfortunately, GtkScrollType is deficient in that there is
   * no horizontal/vertical variants for GTK_SCROLL_START/END,
   * so we have to add an additional boolean flag.
   */
  gboolean (*scroll_child) (GtkScrolledWindow *scrolled_window,
	  		    GtkScrollType      scroll,
			    gboolean           horizontal);

  void (* move_focus_out) (GtkScrolledWindow *scrolled_window,
			   GtkDirectionType   direction);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GType          gtk_scrolled_window_get_type          (void) G_GNUC_CONST;
GtkWidget*     gtk_scrolled_window_new               (GtkAdjustment     *hadjustment,
						      GtkAdjustment     *vadjustment);
void           gtk_scrolled_window_set_hadjustment   (GtkScrolledWindow *scrolled_window,
						      GtkAdjustment     *hadjustment);
void           gtk_scrolled_window_set_vadjustment   (GtkScrolledWindow *scrolled_window,
						      GtkAdjustment     *vadjustment);
GtkAdjustment* gtk_scrolled_window_get_hadjustment   (GtkScrolledWindow *scrolled_window);
GtkAdjustment* gtk_scrolled_window_get_vadjustment   (GtkScrolledWindow *scrolled_window);
GtkWidget*     gtk_scrolled_window_get_hscrollbar    (GtkScrolledWindow *scrolled_window);
GtkWidget*     gtk_scrolled_window_get_vscrollbar    (GtkScrolledWindow *scrolled_window);
void           gtk_scrolled_window_set_policy        (GtkScrolledWindow *scrolled_window,
						      GtkPolicyType      hscrollbar_policy,
						      GtkPolicyType      vscrollbar_policy);
void           gtk_scrolled_window_get_policy        (GtkScrolledWindow *scrolled_window,
						      GtkPolicyType     *hscrollbar_policy,
						      GtkPolicyType     *vscrollbar_policy);
void           gtk_scrolled_window_set_placement     (GtkScrolledWindow *scrolled_window,
						      GtkCornerType      window_placement);
void           gtk_scrolled_window_unset_placement   (GtkScrolledWindow *scrolled_window);

GtkCornerType  gtk_scrolled_window_get_placement     (GtkScrolledWindow *scrolled_window);
void           gtk_scrolled_window_set_shadow_type   (GtkScrolledWindow *scrolled_window,
						      GtkShadowType      type);
GtkShadowType  gtk_scrolled_window_get_shadow_type   (GtkScrolledWindow *scrolled_window);
void	       gtk_scrolled_window_add_with_viewport (GtkScrolledWindow *scrolled_window,
						      GtkWidget		*child);

gint           gtk_scrolled_window_get_min_content_width  (GtkScrolledWindow *scrolled_window);
void           gtk_scrolled_window_set_min_content_width  (GtkScrolledWindow *scrolled_window,
                                                           gint               width);
gint           gtk_scrolled_window_get_min_content_height (GtkScrolledWindow *scrolled_window);
void           gtk_scrolled_window_set_min_content_height (GtkScrolledWindow *scrolled_window,
                                                           gint               height);
GDK_AVAILABLE_IN_3_4
void           gtk_scrolled_window_set_kinetic_scrolling  (GtkScrolledWindow        *scrolled_window,
                                                           gboolean                  kinetic_scrolling);
GDK_AVAILABLE_IN_3_4
gboolean       gtk_scrolled_window_get_kinetic_scrolling  (GtkScrolledWindow        *scrolled_window);

GDK_AVAILABLE_IN_3_4
void           gtk_scrolled_window_set_capture_button_press (GtkScrolledWindow      *scrolled_window,
                                                             gboolean                capture_button_press);
GDK_AVAILABLE_IN_3_4
gboolean       gtk_scrolled_window_get_capture_button_press (GtkScrolledWindow      *scrolled_window);

gint _gtk_scrolled_window_get_scrollbar_spacing (GtkScrolledWindow *scrolled_window);


G_END_DECLS


#endif /* __GTK_SCROLLED_WINDOW_H__ */
