/* GTK - The GIMP Toolkit
 * Copyright (C) 2007 Red Hat, Inc.
 *
 * Authors:
 * - Bastien Nocera <bnocera@redhat.com>
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
 * Modified by the GTK+ Team and others 2007.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_VOLUME_BUTTON_H__
#define __GTK_VOLUME_BUTTON_H__

#include <gtk/gtkscalebutton.h>

G_BEGIN_DECLS

#define GTK_TYPE_VOLUME_BUTTON                 (gtk_volume_button_get_type ())
#define GTK_VOLUME_BUTTON(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_VOLUME_BUTTON, GtkVolumeButton))
#define GTK_VOLUME_BUTTON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_VOLUME_BUTTON, GtkVolumeButtonClass))
#define GTK_IS_VOLUME_BUTTON(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_VOLUME_BUTTON))
#define GTK_IS_VOLUME_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VOLUME_BUTTON))
#define GTK_VOLUME_BUTTON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_VOLUME_BUTTON, GtkVolumeButtonClass))

typedef struct _GtkVolumeButton       GtkVolumeButton;
typedef struct _GtkVolumeButtonClass  GtkVolumeButtonClass;

struct _GtkVolumeButton
{
  GtkScaleButton  parent;
};

struct _GtkVolumeButtonClass
{
  GtkScaleButtonClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GType		gtk_volume_button_get_type	(void) G_GNUC_CONST;
GtkWidget*	gtk_volume_button_new		(void);

G_END_DECLS

#endif /* __GTK_VOLUME_BUTTON_H__ */
