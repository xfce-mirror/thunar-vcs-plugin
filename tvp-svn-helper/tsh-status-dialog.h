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

#ifndef __TSH_STATUS_DIALOG_H__
#define __TSH_STATUS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshStatusDialogClass TshStatusDialogClass;
typedef struct _TshStatusDialog      TshStatusDialog;

#define TSH_TYPE_STATUS_DIALOG             (tsh_status_dialog_get_type ())
#define TSH_STATUS_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_STATUS_DIALOG, TshStatusDialog))
#define TSH_STATUS_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_STATUS_DIALOG, TshStatusDialogClass))
#define TSH_IS_STATUS_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_STATUS_DIALOG))
#define TSH_IS_STATUS_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_STATUS_DIALOG))
#define TSH_STATUS_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_STATUS_DIALOG, TshStatusDialogClass))

GType      tsh_status_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_status_dialog_new      (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tsh_status_dialog_add      (TshStatusDialog *dialog,
                                       const char *path,
                                       const char *text,
                                       const char *prop,
                                       const char *repo_text,
                                       const char *repo_prop);
void       tsh_status_dialog_done     (TshStatusDialog *dialog);

svn_depth_t tsh_status_dialog_get_depth            (TshStatusDialog *dialog);
gboolean    tsh_status_dialog_get_show_unmodified  (TshStatusDialog *dialog);
gboolean    tsh_status_dialog_get_show_unversioned (TshStatusDialog *dialog);
gboolean    tsh_status_dialog_get_check_reposetory (TshStatusDialog *dialog);
gboolean    tsh_status_dialog_get_show_ignore      (TshStatusDialog *dialog);
gboolean    tsh_status_dialog_get_hide_externals   (TshStatusDialog *dialog);

G_END_DECLS;

#endif /* !__TSH_STATUS_DIALOG_H__ */
