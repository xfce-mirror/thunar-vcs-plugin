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

#include <thunar-vfs/thunar-vfs.h>
#include <gtk/gtk.h>

#include <subversion-1/svn_client.h>

#include "tsh-common.h"
#include "tsh-status-dialog.h"

static void cancel_clicked (GtkButton*, gpointer);
static void refresh_clicked (GtkButton*, gpointer);

struct _TshStatusDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
  GtkWidget *get_all;
  GtkWidget *unversioned;
  GtkWidget *update;
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
	GtkWidget *get_all;
	GtkWidget *unversioned;
	GtkWidget *update;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;

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

	model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

	dialog->get_all = get_all = gtk_check_button_new_with_label (_("Show Unmodified Files"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), get_all, FALSE, FALSE, 0);
	gtk_widget_show (get_all);

	dialog->unversioned = unversioned = gtk_check_button_new_with_label (_("Show Unversioned Files"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), unversioned, FALSE, FALSE, 0);
	gtk_widget_show (unversioned);

	dialog->update = update = gtk_check_button_new_with_label (_("Check Repository"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), update, FALSE, FALSE, 0);
	gtk_widget_show (update);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Status"));

	dialog->cancel = button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
	gtk_widget_show (button);

	dialog->refresh = button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
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

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_PATH, file,
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

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->refresh);
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

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
	
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_REFRESH], 0);
}

