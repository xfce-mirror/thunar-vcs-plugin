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

#ifndef __TSH_PROPERTIES_DIALOG_H__
#define __TSH_PROPERTIES_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshPropertiesDialogClass TshPropertiesDialogClass;
typedef struct _TshPropertiesDialog      TshPropertiesDialog;

#define TSH_TYPE_PROPERTIES_DIALOG             (tsh_properties_dialog_get_type ())
#define TSH_PROPERTIES_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_PROPERTIES_DIALOG, TshPropertiesDialog))
#define TSH_PROPERTIES_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_PROPERTIES_DIALOG, TshPropertiesDialogClass))
#define TSH_IS_PROPERTIES_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_PROPERTIES_DIALOG))
#define TSH_IS_PROPERTIES_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_PROPERTIES_DIALOG))
#define TSH_PROPERTIES_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_PROPERTIES_DIALOG, TshPropertiesDialogClass))

GType       tsh_properties_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget*  tsh_properties_dialog_new      (const gchar *title,
                                            GtkWindow *parent,
                                            GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void        tsh_properties_dialog_add      (TshPropertiesDialog *dialog,
                                            const char *name,
                                            const char *value);
void        tsh_properties_dialog_done     (TshPropertiesDialog *dialog);


gchar      *tsh_properties_dialog_get_key          (TshPropertiesDialog *dialog);
gchar      *tsh_properties_dialog_get_selected_key (TshPropertiesDialog *dialog);
gchar      *tsh_properties_dialog_get_value        (TshPropertiesDialog *dialog);
svn_depth_t tsh_properties_dialog_get_depth        (TshPropertiesDialog *dialog);

G_END_DECLS;

#endif /* !__TSH_PROPERTIES_DIALOG_H__ */
