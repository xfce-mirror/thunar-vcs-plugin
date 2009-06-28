/*-
 * Copyright (c) 2006 Peter de Ridder <peter@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __TGH_STATUS_DIALOG_H__
#define __TGH_STATUS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TghStatusDialogClass TghStatusDialogClass;
typedef struct _TghStatusDialog      TghStatusDialog;

#define TGH_TYPE_STATUS_DIALOG             (tgh_status_dialog_get_type ())
#define TGH_STATUS_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TGH_TYPE_STATUS_DIALOG, TghStatusDialog))
#define TGH_STATUS_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TGH_TYPE_STATUS_DIALOG, TghStatusDialogClass))
#define TGH_IS_STATUS_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TGH_TYPE_STATUS_DIALOG))
#define TGH_IS_STATUS_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TGH_TYPE_STATUS_DIALOG))
#define TGH_STATUS_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TGH_TYPE_STATUS_DIALOG, TghStatusDialogClass))

GType      tgh_status_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tgh_status_dialog_new      (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tgh_status_dialog_add      (TghStatusDialog *dialog,
                                       const gchar *path,
                                       const gchar *state,
                                       gboolean commit);
void       tgh_status_dialog_done     (TghStatusDialog *dialog);

G_END_DECLS;

#endif /* !__TGH_STATUS_DIALOG_H__ */
