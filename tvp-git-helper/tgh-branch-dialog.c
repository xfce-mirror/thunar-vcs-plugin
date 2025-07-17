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

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-branch-dialog.h"

static void cancel_clicked (GtkButton*, gpointer);
static void checkout_clicked (GtkButton*, gpointer);
static void create_clicked (GtkButton*, gpointer);

struct _TghBranchDialog
{
  GtkDialog dialog;

  GtkWidget *tree_view;
  GtkWidget *close;
  GtkWidget *cancel;
  GtkWidget *checkout;
  GtkWidget *create;
};

struct _TghBranchDialogClass
{
  GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TghBranchDialog, tgh_branch_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_CHECKOUT,
  SIGNAL_CREATE,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tgh_branch_dialog_class_init (TghBranchDialogClass *klass)
{
  signals[SIGNAL_CANCEL] = g_signal_new("cancel-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);

  signals[SIGNAL_CHECKOUT] = g_signal_new("checkout-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__STRING,
    G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SIGNAL_CREATE] = g_signal_new("create-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__STRING,
    G_TYPE_NONE, 1, G_TYPE_STRING);
}

enum {
  COLUMN_BRANCH = 0,
  COLUMN_ACTIVE,
  COLUMN_COUNT
};

static void
tgh_branch_dialog_init (TghBranchDialog *dialog)
{
  GtkWidget *button;
  GtkWidget *tree_view;
  GtkWidget *scroll_window;
  GtkCellRenderer *renderer;
  GtkTreeModel *model;

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->tree_view = tree_view = gtk_tree_view_new ();

  renderer = gtk_cell_renderer_toggle_new ();
  gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               -1, _("Active"),
                                               renderer, "active",
                                               COLUMN_ACTIVE, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               -1, _("Name"),
                                               renderer, "text",
                                               COLUMN_BRANCH, NULL);

  model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_BOOLEAN));

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

  g_object_unref (model);

  gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scroll_window, TRUE, TRUE, 0);
  gtk_widget_show (tree_view);
  gtk_widget_show (scroll_window);

  dialog->checkout = button = gtk_button_new_with_mnemonic (_("_Jump to"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (checkout_clicked), dialog);
  gtk_widget_show (button);

  dialog->create = button = gtk_button_new_with_mnemonic (_("_New"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (create_clicked), dialog);
  gtk_widget_show (button);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Branch"));

  dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Close"), GTK_RESPONSE_CLOSE);
  gtk_widget_hide (button);

  dialog->cancel = button = gtk_button_new_with_mnemonic (_("_Cancel"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
  gtk_widget_show (button);

  gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tgh_branch_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
  TghBranchDialog *dialog = g_object_new (TGH_TYPE_BRANCH_DIALOG, NULL);

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

void
tgh_branch_dialog_add (TghBranchDialog *dialog, const gchar *branch, gboolean active)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  g_return_if_fail (TGH_IS_BRANCH_DIALOG (dialog));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_BRANCH, branch,
                      COLUMN_ACTIVE, active,
                      -1);
}

void
tgh_branch_dialog_done (TghBranchDialog *dialog)
{
  g_return_if_fail (TGH_IS_BRANCH_DIALOG (dialog));

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
  TghBranchDialog *dialog = TGH_BRANCH_DIALOG (user_data);

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);

  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
checkout_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;

  TghBranchDialog *dialog = TGH_BRANCH_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_BRANCH, &name, -1);

    gtk_widget_hide (dialog->close);
    gtk_widget_show (dialog->cancel);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    g_signal_emit (dialog, signals[SIGNAL_CHECKOUT], 0, name);

    g_free (name);
  }
}

static void
create_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeModel *model;
  GtkWidget *name_dialog;
  GtkWidget *label, *image, *hbox, *vbox, *name_entry;
  gchar *name;
  gint result;

  TghBranchDialog *dialog = TGH_BRANCH_DIALOG (user_data);

  name_dialog = gtk_dialog_new_with_buttons (NULL, GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, _("_Cancel"), GTK_RESPONSE_CANCEL, _("_New"), GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_resizable (GTK_WINDOW (name_dialog), FALSE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (name_dialog), TRUE);

  label = gtk_label_new (_("Branch name:"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
  gtk_label_set_yalign (GTK_LABEL (label), 0.0f);

  image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);

  name_entry = gtk_entry_new ();

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

  gtk_box_pack_start (GTK_BOX (vbox), label,
                      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), name_entry,
                      TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), image,
                      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), vbox,
                      TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (name_dialog))),
                      hbox,
                      FALSE, FALSE, 0);

  gtk_container_set_border_width (GTK_CONTAINER (name_dialog), 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (name_dialog))), 14); /* 14 + 2 * 5 = 24 */
  gtk_container_set_border_width (GTK_CONTAINER (tvp_gtk_dialog_get_action_area (GTK_DIALOG (name_dialog))), 5);
  gtk_box_set_spacing (GTK_BOX (tvp_gtk_dialog_get_action_area (GTK_DIALOG (name_dialog))), 6);

  gtk_widget_show_all (hbox);

  result = gtk_dialog_run (GTK_DIALOG (name_dialog));
  if (result != GTK_RESPONSE_ACCEPT)
  {
    gtk_widget_destroy (name_dialog);
    return;
  }

  name = g_strdup (gtk_entry_get_text (GTK_ENTRY (name_entry)));

  gtk_widget_destroy (name_dialog);

  gtk_widget_hide (dialog->close);
  gtk_widget_show (dialog->cancel);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_CREATE], 0, name);

  g_free (name);
}
