/*-
 * Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
 * Copyright (C) 2012 Stefan Sperling <stsp@stsp.name>
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

#ifndef __TSH_DIFF_DIALOG_H__
#define __TSH_DIFF_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshDiffDialogClass TshDiffDialogClass;
typedef struct _TshDiffDialog      TshDiffDialog;

#define TSH_TYPE_DIFF_DIALOG            (tsh_diff_dialog_get_type ())
#define TSH_DIFF_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_DIFF_DIALOG, TshDiffDialog))
#define TSH_DIFF_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_DIFF_DIALOG, TshDiffDialogClass))
#define TSH_IS_DIFF_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_DIFF_DIALOG))
#define TSH_IS_DIFF_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_DIFF_DIALOG))
#define TSH_DIFF_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_DIFF_DIALOG, TshDiffDialogClass))

GType      tsh_diff_dialog_get_type  (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_diff_dialog_new       (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tsh_diff_dialog_add       (TshDiffDialog *dialog,
                                       const char *line,
                                       gint len);
void       tsh_diff_dialog_done      (TshDiffDialog *dialog);

svn_depth_t tsh_diff_dialog_get_depth (TshDiffDialog *dialog);
gboolean   tsh_diff_dialog_get_show_copies_as_adds (TshDiffDialog *dialog);

G_END_DECLS;

#endif /* !__TSH_DIFF_DIALOG_H__ */
