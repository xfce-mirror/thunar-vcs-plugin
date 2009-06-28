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

#ifndef __TGH_BRANCH_DIALOG_H__
#define __TGH_BRANCH_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TghBranchDialogClass TghBranchDialogClass;
typedef struct _TghBranchDialog      TghBranchDialog;

#define TGH_TYPE_BRANCH_DIALOG             (tgh_branch_dialog_get_type ())
#define TGH_BRANCH_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TGH_TYPE_BRANCH_DIALOG, TghBranchDialog))
#define TGH_BRANCH_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TGH_TYPE_BRANCH_DIALOG, TghBranchDialogClass))
#define TGH_IS_BRANCH_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TGH_TYPE_BRANCH_DIALOG))
#define TGH_IS_BRANCH_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TGH_TYPE_BRANCH_DIALOG))
#define TGH_BRANCH_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TGH_TYPE_BRANCH_DIALOG, TghBranchDialogClass))

GType      tgh_branch_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tgh_branch_dialog_new      (const gchar *title,
                                       GtkWindow *parent,
                                       GtkDialogFlags flags) G_GNUC_MALLOC G_GNUC_INTERNAL;

void       tgh_branch_dialog_add      (TghBranchDialog *dialog,
                                       const gchar *branch,
                                       gboolean active);
void       tgh_branch_dialog_done     (TghBranchDialog *dialog);

G_END_DECLS;

#endif /* !__TGH_BRANCH_DIALOG_H__ */
