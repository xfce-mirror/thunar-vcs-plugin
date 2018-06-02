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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include "tgh-clean-dialog.h"

struct _TghCleanDialog
{
  GtkDialog dialog;

  GtkWidget *directories;
  GtkWidget *ignore;
  GtkWidget *force;
};

struct _TghCleanDialogClass
{
  GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TghCleanDialog, tgh_clean_dialog, GTK_TYPE_DIALOG)

static void
tgh_clean_dialog_class_init (TghCleanDialogClass *klass)
{
}

static void
tgh_clean_dialog_init (TghCleanDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkCellRenderer *renderer;

  dialog->directories = gtk_check_button_new_with_label (_("Remove directories."));
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), dialog->directories, FALSE, TRUE, 0);
  gtk_widget_show(dialog->directories);

  model = GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT));

  dialog->ignore = gtk_combo_box_new_with_model (model);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), dialog->ignore, FALSE, TRUE, 0);
  gtk_widget_show(dialog->ignore);

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
      0, _("Exclude ignored files"),
      1, TGH_CLEAN_IGNORE_EXCLUDE,
      -1);

  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (dialog->ignore), &iter);

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
      0, _("Include ignored files"),
      1, TGH_CLEAN_IGNORE_INCLUDE,
      -1);

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
      0, _("Only ignored files"),
      1, TGH_CLEAN_IGNORE_ONLY,
      -1);

  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (dialog->ignore), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (dialog->ignore), renderer, "text", 0);

  dialog->force = gtk_check_button_new_with_label (_("Force clean."));
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), dialog->force, FALSE, TRUE, 0);
  gtk_widget_show(dialog->force);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Clean"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
      _("_Cancel"), GTK_RESPONSE_CANCEL,
      _("_OK"), GTK_RESPONSE_OK,
      NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tgh_clean_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
  TghCleanDialog *dialog = g_object_new (TSH_TYPE_TRUST_DIALOG, NULL);

  if(title)
    gtk_window_set_title (GTK_WINDOW(dialog), title);

  if(parent)
    gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

  if(flags & GTK_DIALOG_MODAL)
    gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

  if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
    gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

  return GTK_WIDGET(dialog);
}

gboolean
tgh_clean_dialog_get_diretories (TghCleanDialog *dialog)
{
  g_return_val_if_fail (TGH_IS_CLEAN_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->directories));
}

TghCleanIgnore
tgh_clean_dialog_get_ignore (TghCleanDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  TghCleanIgnore ignore;
  GValue value;

  memset(&value, 0, sizeof(GValue));

  g_return_val_if_fail (TGH_IS_CLEAN_DIALOG (dialog), TGH_CLEAN_IGNORE_EXCLUDE);

  g_return_val_if_fail (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (dialog->ignore), &iter), TGH_CLEAN_IGNORE_EXCLUDE);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->ignore));
  gtk_tree_model_get_value (model, &iter, 1, &value);

  ignore = g_value_get_int (&value);

  g_value_unset(&value);

  return ignore;
}

gboolean
tgh_clean_dialog_get_force (TghCleanDialog *dialog)
{
  g_return_val_if_fail (TGH_IS_CLEAN_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->force));
}

