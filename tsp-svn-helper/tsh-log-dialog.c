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
#include "tsh-log-dialog.h"

static void selection_changed (GtkTreeView*, gpointer);
static void cancel_clicked (GtkButton*, gpointer);
static void refresh_clicked (GtkButton*, gpointer);

struct _TshLogDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
  GtkWidget *text_view;
  GtkWidget *file_view;
	GtkWidget *close;
	GtkWidget *cancel;
	GtkWidget *refresh;
};

struct _TshLogDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshLogDialog, tsh_log_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_REFRESH,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tsh_log_dialog_class_init (TshLogDialogClass *klass)
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
	COLUMN_REVISION = 0,
  COLUMN_AUTHOR,
  COLUMN_DATE,
  COLUMN_MESSAGE,
  COLUMN_FULL_MESSAGE,
	COLUMN_COUNT
};

static void
tsh_log_dialog_init (TshLogDialog *dialog)
{
	GtkWidget *button;
	GtkWidget *tree_view;
	GtkWidget *text_view;
	GtkWidget *file_view;
	GtkWidget *scroll_window;
  GtkWidget *pane;
  GtkWidget *vpane;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;

  pane = gtk_vpaned_new ();

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Revision"),
	                                             renderer, "text",
	                                             COLUMN_REVISION, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("Author"),
	                                             renderer, "text",
	                                             COLUMN_AUTHOR, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Date"),
	                                             renderer, "text",
	                                             COLUMN_DATE, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Message"),
	                                             renderer, "text",
	                                             COLUMN_MESSAGE, NULL);

	model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_LONG, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

  g_signal_connect (G_OBJECT (tree_view), "cursor-changed", G_CALLBACK (selection_changed), dialog); 

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
  gtk_paned_pack1 (GTK_PANED(pane), scroll_window, TRUE, FALSE);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  vpane = gtk_vpaned_new ();

  dialog->text_view = text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);

	gtk_container_add (GTK_CONTAINER (scroll_window), text_view);
  gtk_paned_pack1 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
	gtk_widget_show (text_view);
	gtk_widget_show (scroll_window);

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->file_view = file_view = gtk_tree_view_new ();

	//model = GTK_TREE_MODEL (gtk_list_store_new (0));

	//gtk_tree_view_set_model (GTK_TREE_VIEW (file_view), model);

	//g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), file_view);
  gtk_paned_pack2 (GTK_PANED(vpane), scroll_window, TRUE, FALSE);
	//gtk_widget_show (file_view);
	//gtk_widget_show (scroll_window);

  gtk_paned_pack2 (GTK_PANED(pane), vpane, TRUE, FALSE);
  gtk_widget_show (vpane);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), pane, TRUE, TRUE, 0);
  gtk_widget_show (pane);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Log"));

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
tsh_log_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshLogDialog *dialog = g_object_new (TSH_TYPE_LOG_DIALOG, NULL);

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
tsh_log_dialog_add (TshLogDialog *dialog, GSList *files, glong revision, const char *author, const char *date, const char *message)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
  gchar **lines;
  gchar **first_line;

  g_return_if_fail (TSH_IS_LOG_DIALOG (dialog));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  lines = g_strsplit_set (message, "\r\n", -1);
  first_line = lines;
  while (*first_line)
  {
    if (g_strstrip (*first_line)[0])
      break;
    first_line++;
  }
  if (!first_line)
    first_line = lines;

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_REVISION, revision,
	                    COLUMN_AUTHOR, author,
	                    COLUMN_DATE, date,
	                    COLUMN_MESSAGE, *first_line,
	                    COLUMN_FULL_MESSAGE, message,
	                    -1);

  g_strfreev (lines);
}

void
tsh_log_dialog_done (TshLogDialog *dialog)
{
  g_return_if_fail (TSH_IS_LOG_DIALOG (dialog));

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->refresh);
}

static void
selection_changed (GtkTreeView *tree_view, gpointer user_data)
{
	GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *message = NULL;

	TshLogDialog *dialog = TSH_LOG_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (tree_view);
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_FULL_MESSAGE, &message, -1);
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view)), message, -1);
    g_free (message);
  }
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
	TshLogDialog *dialog = TSH_LOG_DIALOG (user_data);

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->refresh);
	
  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
refresh_clicked (GtkButton *button, gpointer user_data)
{
	GtkTreeModel *model;
	TshLogDialog *dialog = TSH_LOG_DIALOG (user_data);

	gtk_widget_hide (dialog->refresh);
	gtk_widget_show (dialog->cancel);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
	
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_REFRESH], 0);
}

