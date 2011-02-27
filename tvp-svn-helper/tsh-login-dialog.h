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

#ifndef __TSH_LOGIN_DIALOG_H__
#define __TSH_LOGIN_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshLoginDialogClass TshLoginDialogClass;
typedef struct _TshLoginDialog      TshLoginDialog;

#define TSH_TYPE_LOGIN_DIALOG             (tsh_login_dialog_get_type ())
#define TSH_LOGIN_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_LOGIN_DIALOG, TshLoginDialog))
#define TSH_LOGIN_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_LOGIN_DIALOG, TshLoginDialogClass))
#define TSH_IS_LOGIN_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_LOGIN_DIALOG))
#define TSH_IS_LOGIN_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_LOGIN_DIALOG))
#define TSH_LOGIN_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_LOGIN_DIALOG, TshLoginDialogClass))

GType      tsh_login_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_login_dialog_new      (const gchar *title,
                                      GtkWindow *parent,
                                      GtkDialogFlags flags,
                                      const gchar *username,
                                      gboolean password,
                                      gboolean may_save) G_GNUC_MALLOC G_GNUC_INTERNAL;

gchar*       tsh_login_dialog_get_username (TshLoginDialog*);
gchar*       tsh_login_dialog_get_password (TshLoginDialog*);
gboolean     tsh_login_dialog_get_may_save (TshLoginDialog*);

G_END_DECLS;

#endif /* !__TSH_LOGIN_DIALOG_H__ */
