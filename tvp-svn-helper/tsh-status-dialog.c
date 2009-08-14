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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <gtk/gtk.h>

#include <subversion-1/svn_client.h>

#include "tsh-common.h"
#include "tsh-tree-common.h"
#include "tsh-status-dialog.h"

static void cancel_clicked (GtkButton*, gpointer);
static void refresh_clicked (GtkButton*, gpointer);
static void move_info (GtkTreeStore*, GtkTreeIter*, GtkTreeIter*);

struct _TshStatusDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
  GtkWidget *depth;
  GtkWidget *get_all;
  GtkWidget *unversioned;
  GtkWidget *update;
	GtkWidget *no_ignore;
	GtkWidget *ignore_externals;
	GtkWidget *close;
	GtkWidget *cancel;
	GtkWidget *refresh;
};

struct _TshStatusDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshStatusDialog, tsh_status_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_REFRESH,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tsh_status_dialog_class_init (TshStatusDialogClass *klass)
{
  signals[SIGNAL_CANCEL] = g_signal_new("cancel-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);
  signals[SIGNAL_REFRESH] = g_signal_new("refresh-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);
}

enum {
	COLUMN_PATH = 0,
  COLUMN_TEXT_STAT,
  COLUMN_PROP_STAT,
  COLUMN_REPO_TEXT_STAT,
  COLUMN_REPO_PROP_STAT,
	COLUMN_COUNT
};

static void
tsh_status_dialog_init (TshStatusDialog *dialog)
{
	GtkWidget *button;
	GtkWidget *tree_view;
	GtkWidget *scroll_window;
	GtkWidget *depth;
	GtkWidget *get_all;
	GtkWidget *unversioned;
	GtkWidget *update;
	GtkWidget *no_ignore;
	GtkWidget *ignore_externals;
  GtkWidget *table;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
    GtkTreeIter iter;

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Path"),
	                                             renderer, "text",
	                                             COLUMN_PATH, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("State"),
	                                             renderer, "text",
	                                             COLUMN_TEXT_STAT, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("Prop state"),
	                                             renderer, "text",
	                                             COLUMN_PROP_STAT, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("Repo state"),
	                                             renderer, "text",
	                                             COLUMN_REPO_TEXT_STAT, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("Repo prop state"),
	                                             renderer, "text",
	                                             COLUMN_REPO_PROP_STAT, NULL);

  model = GTK_TREE_MODEL (gtk_tree_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

  table = gtk_table_new (3, 2, FALSE);

	dialog->depth = depth = gtk_combo_box_new ();

	model = GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT));

	gtk_combo_box_set_model (GTK_COMBO_BOX (depth), model);

    /*
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    0, _("Unknown"),
	                    1, svn_depth_unknown,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    0, _("Exclude"),
	                    1, svn_depth_exclude,
	                    -1);
    */

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Self means only this file/direcotry is shown
                         */
	                    0, _("Self"),
	                    1, svn_depth_empty,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Immediate files means this file/direcotry and the files it contains are shown
                         */
	                    0, _("Immediate files"),
	                    1, svn_depth_files,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Immediates means this file/direcotry and the subdirectories are shown
                         */
	                    0, _("Immediates"),
	                    1, svn_depth_immediates,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Recursive means the list is full recursive
                         */
	                    0, _("Recursive"),
	                    1, svn_depth_infinity,
	                    -1);

    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (depth), &iter);

	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (depth), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (depth), renderer, "text", 0);

  gtk_table_attach (GTK_TABLE (table), depth, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (depth);

	dialog->get_all = get_all = gtk_check_button_new_with_label (_("Show Unmodified Files"));
  gtk_table_attach (GTK_TABLE (table), get_all, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (get_all);

	dialog->unversioned = unversioned = gtk_check_button_new_with_label (_("Show Unversioned Files"));
  gtk_table_attach (GTK_TABLE (table), unversioned, 0, 1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (unversioned);

	dialog->no_ignore = no_ignore = gtk_check_button_new_with_label (_("Show Ignored Files"));
  gtk_table_attach (GTK_TABLE (table), no_ignore, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (no_ignore);

	dialog->ignore_externals = ignore_externals = gtk_check_button_new_with_label (_("Hide Externals"));
  gtk_table_attach (GTK_TABLE (table), ignore_externals, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (ignore_externals);

	dialog->update = update = gtk_check_button_new_with_label (_("Check Repository"));
  gtk_table_attach (GTK_TABLE (table), update, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (update);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Status"));

  gtk_button_box_set_layout(GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area), GTK_BUTTONBOX_EDGE);

	dialog->cancel = button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
	gtk_widget_show (button);

	dialog->refresh = button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (refresh_clicked), dialog);
	gtk_widget_hide (button);

	dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_widget_show (button);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tsh_status_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshStatusDialog *dialog = g_object_new (TSH_TYPE_STATUS_DIALOG, NULL);

	if(title)
		gtk_window_set_title (GTK_WINDOW(dialog), title);

	if(parent)
		gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	if(flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_NO_SEPARATOR)
		gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);

	return GTK_WIDGET(dialog);
}

void       
tsh_status_dialog_add (TshStatusDialog *dialog, const char *file, const char *text, const char *prop, const char *repo_text, const char *repo_prop)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

  g_return_if_fail (TSH_IS_STATUS_DIALOG (dialog));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  tsh_tree_get_iter_for_path (GTK_TREE_STORE (model), file, &iter, COLUMN_PATH, move_info);
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                      //COLUMN_PATH, file,
                      COLUMN_TEXT_STAT, text,
                      COLUMN_PROP_STAT, prop,
                      COLUMN_REPO_TEXT_STAT, repo_text,
                      COLUMN_REPO_PROP_STAT, repo_prop,
                      -1);
}

void
tsh_status_dialog_done (TshStatusDialog *dialog)
{
  g_return_if_fail (TSH_IS_STATUS_DIALOG (dialog));

  gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tree_view));

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->refresh);
}

svn_depth_t
tsh_status_dialog_get_depth (TshStatusDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  svn_depth_t depth;
  GValue value;

  memset(&value, 0, sizeof(GValue));

  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), svn_depth_unknown);

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (dialog->depth), &iter))
    return svn_depth_unknown;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->depth));
  gtk_tree_model_get_value (model, &iter, 1, &value);

  depth = g_value_get_int (&value);

  g_value_unset(&value);

  return depth;
}

gboolean
tsh_status_dialog_get_show_unmodified (TshStatusDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->get_all));
}

gboolean
tsh_status_dialog_get_show_unversioned (TshStatusDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->unversioned));
}

gboolean
tsh_status_dialog_get_check_reposetory (TshStatusDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->update));
}

gboolean
tsh_status_dialog_get_show_ignore (TshStatusDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->no_ignore));
}

gboolean
tsh_status_dialog_get_hide_externals (TshStatusDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_STATUS_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ignore_externals));
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
	TshStatusDialog *dialog = TSH_STATUS_DIALOG (user_data);

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->refresh);
	
  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
refresh_clicked (GtkButton *button, gpointer user_data)
{
	GtkTreeModel *model;
	TshStatusDialog *dialog = TSH_STATUS_DIALOG (user_data);

	gtk_widget_hide (dialog->refresh);
	gtk_widget_show (dialog->cancel);

  g_signal_emit (dialog, signals[SIGNAL_REFRESH], 0);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_tree_store_clear (GTK_TREE_STORE (model));
}

static void
move_info (GtkTreeStore *store, GtkTreeIter *dest, GtkTreeIter *src)
{
  gchar *text, *prop, *repo_text, *repo_prop;

  gtk_tree_model_get (GTK_TREE_MODEL (store), src,
                      COLUMN_TEXT_STAT, &text,
                      COLUMN_PROP_STAT, &prop,
                      COLUMN_REPO_TEXT_STAT, &repo_text,
                      COLUMN_REPO_PROP_STAT, &repo_prop,
                      -1);

  gtk_tree_store_set (store, dest,
                      COLUMN_TEXT_STAT, text,
                      COLUMN_PROP_STAT, prop,
                      COLUMN_REPO_TEXT_STAT, repo_text,
                      COLUMN_REPO_PROP_STAT, repo_prop,
                      -1);

  g_free (text);
  g_free (prop);
  g_free (repo_text);
  g_free (repo_prop);
}

