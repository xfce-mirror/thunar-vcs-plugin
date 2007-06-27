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

#ifndef __TSH_UPDATE_DIALOG_H__
#define __TSH_UPDATE_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshUpdateDialogClass TshUpdateDialogClass;
typedef struct _TshUpdateDialog      TshUpdateDialog;

#define TSH_TYPE_UPDATE_DIALOG             (tsh_update_dialog_get_type ())
#define TSH_UPDATE_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_UPDATE_DIALOG, TshUpdateDialog))
#define TSH_UPDATE_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_UPDATE_DIALOG, TshUpdateDialogClass))
#define TSH_IS_UPDATE_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_UPDATE_DIALOG))
#define TSH_IS_UPDATE_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_UPDATE_DIALOG))
#define TSH_UPDATE_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_UPDATE_DIALOG, TshUpdateDialogClass))

GType      tsh_update_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_update_dialog_new      (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tsh_update_dialog_add      (TshUpdateDialog *dialog,
                                       const char *action,
                                       const char *path,
                                       const char *mime_type);
void       tsh_update_dialog_done     (TshUpdateDialog *dialog);

G_END_DECLS;

#endif /* !__TSH_UPDATE_DIALOG_H__ */
