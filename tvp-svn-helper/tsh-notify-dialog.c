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

#include "tsh-common.h"
#include "tsh-notify-dialog.h"

static void cancel_clicked (GtkButton*, gpointer);

struct _TshNotifyDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
	GtkWidget *close;
	GtkWidget *cancel;
};

struct _TshNotifyDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshNotifyDialog, tsh_notify_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tsh_notify_dialog_class_init (TshNotifyDialogClass *klass)
{
  signals[SIGNAL_CANCEL] = g_signal_new("cancel-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);
}

enum {
	COLUMN_ACTION = 0,
	COLUMN_PATH,
	COLUMN_MIME,
	COLUMN_COUNT
};

static void
tsh_notify_dialog_init (TshNotifyDialog *dialog)
{
	GtkWidget *button;
	GtkWidget *tree_view;
	GtkWidget *scroll_window;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Action"),
	                                             renderer, "text",
	                                             COLUMN_ACTION, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Path"),
	                                             renderer, "text",
	                                             COLUMN_PATH, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("MIME type"),
	                                             renderer, "text",
	                                             COLUMN_MIME, NULL);

	model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Notification"));

	dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Close"), GTK_RESPONSE_CLOSE);
	gtk_widget_hide (button);

	dialog->cancel = button = gtk_button_new_with_mnemonic (_("_Cancel"));
	gtk_box_pack_end (GTK_BOX (tvp_gtk_dialog_get_action_area (GTK_DIALOG (dialog))), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
	gtk_widget_show (button);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tsh_notify_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshNotifyDialog *dialog = g_object_new (TSH_TYPE_NOTIFY_DIALOG, NULL);

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
tsh_notify_dialog_add (TshNotifyDialog *dialog, const char *action, const char *file, const char *mime_type)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;

  g_return_if_fail (TSH_IS_NOTIFY_DIALOG (dialog));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_ACTION, action,
	                    COLUMN_PATH, file,
	                    COLUMN_MIME, mime_type,
	                    -1);

	path = gtk_tree_model_get_path (model, &iter);
	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (dialog->tree_view), path, NULL, FALSE, 0, 0);

	gtk_tree_path_free (path);
}

void
tsh_notify_dialog_done (TshNotifyDialog *dialog)
{
  g_return_if_fail (TSH_IS_NOTIFY_DIALOG (dialog));

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->close);
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
	TshNotifyDialog *dialog = TSH_NOTIFY_DIALOG (user_data);

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->close);

  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}
