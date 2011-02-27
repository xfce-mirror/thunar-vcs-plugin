/*-
 * Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TGH_NOTIFY_DIALOG_H__
#define __TGH_NOTIFY_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TghNotifyDialogClass TghNotifyDialogClass;
typedef struct _TghNotifyDialog      TghNotifyDialog;

#define TGH_TYPE_NOTIFY_DIALOG             (tgh_notify_dialog_get_type ())
#define TGH_NOTIFY_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TGH_TYPE_NOTIFY_DIALOG, TghNotifyDialog))
#define TGH_NOTIFY_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TGH_TYPE_NOTIFY_DIALOG, TghNotifyDialogClass))
#define TGH_IS_NOTIFY_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TGH_TYPE_NOTIFY_DIALOG))
#define TGH_IS_NOTIFY_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TGH_TYPE_NOTIFY_DIALOG))
#define TGH_NOTIFY_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TGH_TYPE_NOTIFY_DIALOG, TghNotifyDialogClass))

GType      tgh_notify_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tgh_notify_dialog_new      (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tgh_notify_dialog_add      (TghNotifyDialog *dialog,
                                       const gchar *action,
                                       const gchar *path);
void       tgh_notify_dialog_done     (TghNotifyDialog *dialog);

G_END_DECLS;

#endif /* !__TGH_NOTIFY_DIALOG_H__ */
