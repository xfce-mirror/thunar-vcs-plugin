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
#include <gtk/gtk.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-stash-dialog.h"

static void selection_changed (GtkTreeView*, gpointer);
static void cancel_clicked (GtkButton*, gpointer);
static void save_clicked (GtkButton*, gpointer);
static void apply_clicked (GtkButton*, gpointer);
static void pop_clicked (GtkButton*, gpointer);
static void drop_clicked (GtkButton*, gpointer);
static void clear_clicked (GtkButton*, gpointer);

struct _TghStashDialog
{
  GtkDialog dialog;

  GtkWidget *tree_view;
  GtkWidget *file_view;
  GtkWidget *save;
  GtkWidget *apply;
  GtkWidget *pop;
  GtkWidget *drop;
  GtkWidget *close;
  GtkWidget *clear;
  GtkWidget *cancel;
};

struct _TghStashDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TghStashDialog, tgh_stash_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_SAVE,
  SIGNAL_APPLY,
  SIGNAL_POP,
  SIGNAL_DROP,
  SIGNAL_CLEAR,
  SIGNAL_SHOW,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tgh_stash_dialog_class_init (TghStashDialogClass *klass)
{
  signals[SIGNAL_CANCEL] = g_signal_new("cancel-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0);

  signals[SIGNAL_SAVE] = g_signal_new("save-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SIGNAL_APPLY] = g_signal_new("apply-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SIGNAL_POP] = g_signal_new("pop-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SIGNAL_DROP] = g_signal_new("drop-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SIGNAL_CLEAR] = g_signal_new("clear-clicked",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0);

  signals[SIGNAL_SHOW] = g_signal_new("selection-changed",
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__STRING,
      G_TYPE_NONE, 1, G_TYPE_STRING);
}

enum {
  COLUMN_NAME = 0,
  COLUMN_BRANCH,
  COLUMN_DESCRIPTION,
  COLUMN_COUNT
};

enum {
  FILE_COLUMN_FILE = 0,
  FILE_COLUMN_PERCENTAGE,
  FILE_COLUMN_CHANGES,
  FILE_COLUMN_COUNT
};

static void
tgh_stash_dialog_init (TghStashDialog *dialog)
{
  GtkWidget *button;
  GtkWidget *tree_view;
  GtkWidget *file_view;
  GtkWidget *scroll_window;
  GtkWidget *vpane;
  GtkCellRenderer *renderer;
  GtkTreeModel *model;

  vpane = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->tree_view = tree_view = gtk_tree_view_new ();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
      -1, _("Name"), renderer,
      "text", COLUMN_NAME,
      NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
      -1, _("Branch"), renderer,
      "text", COLUMN_BRANCH,
      NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
      -1, _("Description"), renderer,
      "text", COLUMN_DESCRIPTION,
      NULL);

  model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

  g_object_unref (model);

  g_signal_connect (G_OBJECT (tree_view), "cursor-changed", G_CALLBACK (selection_changed), dialog);

  gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
  gtk_paned_pack1 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
  gtk_widget_show (tree_view);
  gtk_widget_show (scroll_window);

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  dialog->file_view = file_view = gtk_tree_view_new ();

  renderer = gtk_cell_renderer_progress_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (file_view),
      -1, _("Changes"), renderer,
      "value", FILE_COLUMN_PERCENTAGE,
      "text", FILE_COLUMN_CHANGES,
      NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (file_view),
      -1, _("File"),
      renderer, "text",
      FILE_COLUMN_FILE, NULL);

  model = GTK_TREE_MODEL (gtk_list_store_new (FILE_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING));

  gtk_tree_view_set_model (GTK_TREE_VIEW (file_view), model);

  g_object_unref (model);

  gtk_container_add (GTK_CONTAINER (scroll_window), file_view);
  gtk_paned_pack2 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
  gtk_widget_show (file_view);
  gtk_widget_show (scroll_window);

  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), vpane, TRUE, TRUE, 0);
  gtk_widget_show (vpane);

  dialog->save = button = gtk_button_new_with_mnemonic (_("_Save"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (save_clicked), dialog);
  gtk_widget_show (button);

  dialog->apply = button = gtk_button_new_with_mnemonic (_("_Apply"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (apply_clicked), dialog);
  gtk_widget_show (button);

  dialog->pop = button = gtk_button_new_with_mnemonic (_("_OK"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (pop_clicked), dialog);
  gtk_widget_show (button);

  dialog->drop = button = gtk_button_new_with_mnemonic (_("_Delete"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (drop_clicked), dialog);
  gtk_widget_show (button);

  dialog->clear = button = gtk_button_new_with_mnemonic (_("C_lear"));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (clear_clicked), dialog);
  gtk_widget_show (button);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Stash"));

  dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Close"), GTK_RESPONSE_CLOSE);
  gtk_widget_hide (button);

  dialog->cancel = button = gtk_button_new_with_mnemonic (_("_Cancel"));
  gtk_box_pack_end (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dialog))), button, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
  gtk_widget_show (button);

  tgh_make_homogeneous (dialog->save, dialog->apply, dialog->pop, dialog->drop, dialog->close, dialog->cancel, NULL);

  gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tgh_stash_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
  TghStashDialog *dialog = g_object_new (TGH_TYPE_STASH_DIALOG, NULL);

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
tgh_stash_dialog_add (TghStashDialog *dialog, const gchar *name, const gchar *branch, const gchar *description)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  g_return_if_fail (TGH_IS_STASH_DIALOG (dialog));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
      COLUMN_NAME, name,
      COLUMN_BRANCH, branch,
      COLUMN_DESCRIPTION, description,
      -1);
}

void
tgh_stash_dialog_add_file (TghStashDialog *dialog, guint insertions, guint deletions, const gchar *file)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *changes;
  guint sum;

  g_return_if_fail (TGH_IS_STASH_DIALOG (dialog));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));

  changes = g_strdup_printf ("+%u -%u", insertions, deletions);
  sum = insertions + deletions;
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
      FILE_COLUMN_FILE, file,
      FILE_COLUMN_PERCENTAGE, sum?insertions * 100 / sum:0,
      FILE_COLUMN_CHANGES, changes,
      -1);
  g_free (changes);
}

void
tgh_stash_dialog_done (TghStashDialog *dialog)
{
  g_return_if_fail (TGH_IS_STASH_DIALOG (dialog));

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);
}

static void
selection_changed (GtkTreeView *tree_view, gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;

  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (tree_view);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    gtk_widget_hide (dialog->cancel);
    gtk_widget_show (dialog->close);

    g_signal_emit (dialog, signals[SIGNAL_SHOW], 0, name);

    g_free (name);
  }
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);

  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
save_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeModel *model;
  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);
  GtkWidget *name_dialog;
  GtkWidget *label, *image, *hbox, *vbox, *desc_entry;
  gchar *name;
  gint result;

  name_dialog = gtk_dialog_new_with_buttons (NULL, GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_ACCEPT, NULL);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
  gtk_window_set_resizable (GTK_WINDOW (name_dialog), FALSE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (name_dialog), TRUE);

  label = gtk_label_new (_("Stash description:"));
  image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.0);

  gtk_misc_set_alignment   (GTK_MISC  (label), 0.0, 0.0);

  desc_entry = gtk_entry_new ();

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

  gtk_box_pack_start (GTK_BOX (vbox), label,
                      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), desc_entry,
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
  gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (name_dialog))), 5);
  gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (name_dialog))), 6);

  gtk_widget_show_all (hbox);

  result = gtk_dialog_run (GTK_DIALOG (name_dialog));
  if (result != GTK_RESPONSE_ACCEPT)
  {
    gtk_widget_destroy (name_dialog);
    return;
  }

  name = g_strdup (gtk_entry_get_text (GTK_ENTRY (desc_entry)));

  gtk_widget_destroy (name_dialog);

  gtk_widget_hide (dialog->close);
  gtk_widget_show (dialog->cancel);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_SAVE], 0, name);

  g_free (name);
}

static void
apply_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;

  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

    gtk_widget_hide (dialog->close);
    gtk_widget_show (dialog->cancel);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    g_signal_emit (dialog, signals[SIGNAL_APPLY], 0, name);

    g_free (name);
  }
}

static void
pop_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;

  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

    gtk_widget_hide (dialog->close);
    gtk_widget_show (dialog->cancel);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    g_signal_emit (dialog, signals[SIGNAL_POP], 0, name);

    g_free (name);
  }
}

static void
drop_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;
  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);
  GtkWidget *sure_dialog;
  gint result;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

    sure_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Are you sure you want to drop %s?"), name);
    result = gtk_dialog_run (GTK_DIALOG (sure_dialog));
    gtk_widget_destroy (sure_dialog);
    if (result != GTK_RESPONSE_YES)
      return;

    gtk_widget_hide (dialog->close);
    gtk_widget_show (dialog->cancel);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    g_signal_emit (dialog, signals[SIGNAL_DROP], 0, name);

    g_free (name);
  }
}

static void
clear_clicked (GtkButton *button, gpointer user_data)
{
  GtkTreeModel *model;
  TghStashDialog *dialog = TGH_STASH_DIALOG (user_data);
  GtkWidget *sure_dialog;
  gint result;

  sure_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Are you sure you want to clear all stash?"));
  result = gtk_dialog_run (GTK_DIALOG (sure_dialog));
  gtk_widget_destroy (sure_dialog);
  if (result != GTK_RESPONSE_YES)
    return;

  gtk_widget_hide (dialog->close);
  gtk_widget_show (dialog->cancel);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->file_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_CLEAR], 0);
}
