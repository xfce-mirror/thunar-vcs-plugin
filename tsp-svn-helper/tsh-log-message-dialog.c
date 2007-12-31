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

#include "tsh-log-message-dialog.h"

struct _TshLogMessageDialog
{
	GtkDialog dialog;

  GtkWidget *vpane;
	GtkWidget *text_view;
	GtkWidget *tree_view;
};

struct _TshLogMessageDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshLogMessageDialog, tsh_log_message_dialog, GTK_TYPE_DIALOG)

static void
tsh_log_message_dialog_class_init (TshLogMessageDialogClass *klass)
{
}

enum {
	COLUMN_STATE = 0,
	COLUMN_PATH,
	COLUMN_COUNT
};

static void
tsh_log_message_dialog_init (TshLogMessageDialog *dialog)
{
	GtkWidget *text_view;
	GtkWidget *tree_view;
	GtkWidget *scroll_window;
  GtkWidget *vpane;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;

  dialog->vpane = vpane = gtk_vpaned_new ();

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  dialog->text_view = text_view = gtk_text_view_new ();

	gtk_container_add (GTK_CONTAINER (scroll_window), text_view);
  gtk_paned_pack1 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
	gtk_widget_show (text_view);
	gtk_widget_show (scroll_window);

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("State"),
	                                             renderer, "text",
	                                             COLUMN_STATE, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Path"),
	                                             renderer, "text",
	                                             COLUMN_PATH, NULL);

	model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
  gtk_paned_pack2 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vpane, TRUE, TRUE, 0);
	gtk_widget_show (vpane);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Log Message"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                          NULL);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
}

GtkWidget*
tsh_log_message_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshLogMessageDialog *dialog = g_object_new (TSH_TYPE_LOG_MESSAGE_DIALOG, NULL);

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
tsh_log_message_dialog_add (TshLogMessageDialog *dialog, const char *state, const char *file)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;

  g_return_if_fail (TSH_IS_LOG_MESSAGE_DIALOG (dialog));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_STATE, state,
	                    COLUMN_PATH, file,
	                    -1);

	path = gtk_tree_model_get_path (model, &iter);
	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (dialog->tree_view), path, NULL, FALSE, 0, 0);

	gtk_tree_path_free (path);
}

gchar *
tsh_log_message_dialog_get_message (TshLogMessageDialog *dialog)
{
  GtkTextIter start, end;

  g_return_val_if_fail (TSH_IS_LOG_MESSAGE_DIALOG (dialog), NULL);

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

