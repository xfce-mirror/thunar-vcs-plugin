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
#include <subversion-1/svn_props.h>

#include "tsh-common.h"
#include "tsh-properties-dialog.h"

static void selection_changed (GtkTreeView*, gpointer);
static void cancel_clicked (GtkButton*, gpointer);
static void set_clicked (GtkButton*, gpointer);
static void delete_clicked (GtkButton*, gpointer);
static void tsh_make_homogeneous (GtkWidget *, ...) G_GNUC_NULL_TERMINATED;

struct _TshPropertiesDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
  GtkWidget *combo_box;
  GtkWidget *text_view;
  GtkWidget *depth;
	GtkWidget *set;
	GtkWidget *delete;
	GtkWidget *close;
  GtkWidget *cancel;
};

struct _TshPropertiesDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshPropertiesDialog, tsh_properties_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_SET,
  SIGNAL_DELETE,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tsh_properties_dialog_class_init (TshPropertiesDialogClass *klass)
{
  signals[SIGNAL_CANCEL] = g_signal_new("cancel-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);

  signals[SIGNAL_SET] = g_signal_new("set-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);

  signals[SIGNAL_DELETE] = g_signal_new("delete-clicked",
    G_OBJECT_CLASS_TYPE (klass),
    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
    0, NULL, NULL,
    g_cclosure_marshal_VOID__VOID,
    G_TYPE_NONE, 0);
}

enum {
	COLUMN_NAME = 0,
  COLUMN_VALUE,
  COLUMN_FULL_VALUE,
	COLUMN_COUNT
};

static void
tsh_properties_dialog_init (TshPropertiesDialog *dialog)
{
	GtkWidget *button;
	GtkWidget *tree_view;
	GtkWidget *combo_box;
	GtkWidget *text_view;
  GtkWidget *depth;
	GtkWidget *scroll_window;
  GtkWidget *vpane;
  GtkWidget *box;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
    GtkTreeIter iter;
  GtkEntryCompletion *completion;

  vpane = gtk_vpaned_new ();

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Name"),
	                                             renderer, "text",
	                                             COLUMN_NAME, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, _("Value"),
	                                             renderer, "text",
	                                             COLUMN_VALUE, NULL);

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

  box = gtk_vbox_new (FALSE, 0);

  dialog->combo_box = combo_box = gtk_combo_box_entry_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_EOL_STYLE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_EXECUTABLE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_EXTERNALS);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_IGNORE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_KEYWORDS);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_MERGEINFO);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_MIME_TYPE);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_NEEDS_LOCK);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), SVN_PROP_SPECIAL);

  completion = gtk_entry_completion_new();
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
  gtk_entry_completion_set_model(completion, model);
  gtk_entry_completion_set_text_column(completion, gtk_combo_box_entry_get_text_column (GTK_COMBO_BOX_ENTRY (combo_box)));
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY (gtk_bin_get_child (GTK_BIN (combo_box))), completion);
  g_object_unref(completion);

  gtk_box_pack_start (GTK_BOX(box), combo_box, FALSE, TRUE, 0);
	gtk_widget_show (combo_box);

  dialog->text_view = text_view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD_CHAR);

	gtk_container_add (GTK_CONTAINER (scroll_window), text_view);
  gtk_box_pack_start (GTK_BOX(box), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (text_view);
	gtk_widget_show (scroll_window);

	model = GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT));

	dialog->depth = depth = gtk_combo_box_new_with_model (model);

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
                         * Self means only this file/direcotry is updated
                         */
	                    0, _("Self"),
	                    1, svn_depth_empty,
	                    -1);

    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (depth), &iter);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Immediate files means this file/direcotry and the files it contains are updated
                         */
	                    0, _("Immediate files"),
	                    1, svn_depth_files,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Immediates means this file/direcotry and the subdirectories are updated
                         */
	                    0, _("Immediates"),
	                    1, svn_depth_immediates,
	                    -1);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        /* Translators: svn recursion selection
                         * Recursive means the update is full recursive
                         */
	                    0, _("Recursive"),
	                    1, svn_depth_infinity,
	                    -1);

	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (depth), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (depth), renderer, "text", 0);

  gtk_box_pack_start (GTK_BOX(box), depth, FALSE, TRUE, 0);
	gtk_widget_show (depth);

  gtk_paned_pack2 (GTK_PANED(vpane), box, TRUE, FALSE);
  gtk_widget_show (box);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vpane, TRUE, TRUE, 0);
  gtk_widget_show (vpane);

  //gtk_button_box_set_layout(GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area), GTK_BUTTONBOX_EDGE);
  gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), GTK_DIALOG (dialog)->action_area);

  GTK_DIALOG (dialog)->action_area = box = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->vbox), box,
                    FALSE, TRUE, 0);
  gtk_widget_show (box);

  gtk_box_reorder_child (GTK_BOX (GTK_DIALOG (dialog)->vbox), box, 0);

  //box = gtk_hbox_new (FALSE, 12);

	dialog->set = button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_box_pack_start (GTK_BOX (box), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (set_clicked), dialog);
	gtk_widget_show (button);

	dialog->delete = button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_box_pack_start (GTK_BOX (box), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (delete_clicked), dialog);
	gtk_widget_show (button);

  //gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), box, FALSE, TRUE, 0);
	//gtk_widget_show (box);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Properties"));

	dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_widget_hide (button);

	dialog->cancel = button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
	gtk_widget_show (button);

  tsh_make_homogeneous (dialog->set, dialog->delete, dialog->close, dialog->cancel, NULL);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tsh_properties_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshPropertiesDialog *dialog = g_object_new (TSH_TYPE_PROPERTIES_DIALOG, NULL);

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

static void
tsh_make_homogeneous (GtkWidget *first, ...)
{
  GtkWidget *iter;
  GtkRequisition request;
  gint max_width = 0;
  gint max_height = 0;
  va_list ap;

  va_start (ap, first);
  iter = first;
  while (iter)
  {
    gtk_widget_size_request(iter, &request);
    if (request.width > max_width)
      max_width = request.width;
    if (request.height > max_height)
      max_height = request.height;
    iter = va_arg (ap, GtkWidget *);
  }
  va_end (ap);

  va_start (ap, first);
  iter = first;
  while (iter)
  {
    gtk_widget_set_size_request (iter, max_width, max_height);
    iter = va_arg (ap, GtkWidget *);
  }
  va_end (ap);
}

void       
tsh_properties_dialog_add (TshPropertiesDialog *dialog, const char *name, const char *value)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
  gint column;
  gchar **lines;
  gchar *line;

  g_return_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog));

  lines = g_strsplit_set (value, "\r\n", -1);
  line = g_strjoinv (" ", lines);
  g_strfreev (lines);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_NAME, name,
	                    COLUMN_VALUE, line,
	                    COLUMN_FULL_VALUE, value,
	                    -1);

  g_free (line);

  column = gtk_combo_box_entry_get_text_column (GTK_COMBO_BOX_ENTRY (dialog->combo_box));
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->combo_box));
  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    gboolean found = FALSE;
    do {
      gint cmp;
      gchar *text;
      gtk_tree_model_get (model, &iter, column, &text, -1);

      cmp = g_utf8_collate (name, text);
      g_free (text);
      if (cmp)
        continue;

      found = TRUE;
      break;
    } while (gtk_tree_model_iter_next (model, &iter));

    if (!found)
    {
      gtk_combo_box_append_text (GTK_COMBO_BOX (dialog->combo_box), name);
    }
  }
}

void
tsh_properties_dialog_done (TshPropertiesDialog *dialog)
{
  g_return_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog));

	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->close);
}

gchar *
tsh_properties_dialog_get_key (TshPropertiesDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog), NULL);

  return gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog->combo_box));
}

gchar *
tsh_properties_dialog_get_selected_key (TshPropertiesDialog *dialog)
{
	GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name = NULL;

  g_return_val_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);
  }

  return name;
}

gchar *
tsh_properties_dialog_get_value (TshPropertiesDialog *dialog)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end;

  g_return_val_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

svn_depth_t
tsh_properties_dialog_get_depth (TshPropertiesDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  svn_depth_t depth;
  GValue value;

  memset(&value, 0, sizeof(GValue));

  g_return_val_if_fail (TSH_IS_PROPERTIES_DIALOG (dialog), svn_depth_unknown);

  g_return_val_if_fail (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (dialog->depth), &iter), svn_depth_unknown);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->depth));
  gtk_tree_model_get_value (model, &iter, 1, &value);

  depth = g_value_get_int (&value);

  g_value_unset(&value);

  return depth;
}

static void
selection_changed (GtkTreeView *tree_view, gpointer user_data)
{
	GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  gchar *name;
  gchar *value;

	TshPropertiesDialog *dialog = TSH_PROPERTIES_DIALOG (user_data);

  selection = gtk_tree_view_get_selection (tree_view);
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gint column;
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, COLUMN_FULL_VALUE, &value, -1);

    column = gtk_combo_box_entry_get_text_column (GTK_COMBO_BOX_ENTRY (dialog->combo_box));
    model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->combo_box));
    if (gtk_tree_model_get_iter_first (model, &iter))
    {
      gboolean found = FALSE;
      do {
        gint cmp;
        gchar *text;
        gtk_tree_model_get (model, &iter, column, &text, -1);

        cmp = g_utf8_collate (name, text);
        g_free (text);
        if (cmp)
          continue;
        
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (dialog->combo_box), &iter);

        found = TRUE;
        break;
      } while (gtk_tree_model_iter_next (model, &iter));

      if (!found)
      {
        gtk_combo_box_append_text (GTK_COMBO_BOX (dialog->combo_box), name);
        g_assert (gtk_tree_model_iter_next (model, &iter));

        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (dialog->combo_box), &iter);
      }
    }

    g_free (name);

    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view)), value, -1);
    g_free (value);
  }
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
	TshPropertiesDialog *dialog = TSH_PROPERTIES_DIALOG (user_data);
	
	gtk_widget_hide (dialog->cancel);
	gtk_widget_show (dialog->close);
	
  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
set_clicked (GtkButton *button, gpointer user_data)
{
	GtkTreeModel *model;
	TshPropertiesDialog *dialog = TSH_PROPERTIES_DIALOG (user_data);
	
	gtk_widget_hide (dialog->close);
	gtk_widget_show (dialog->cancel);
  
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_SET], 0);
}

static void
delete_clicked (GtkButton *button, gpointer user_data)
{
	GtkTreeModel *model;
	TshPropertiesDialog *dialog = TSH_PROPERTIES_DIALOG (user_data);
	
	gtk_widget_hide (dialog->close);
	gtk_widget_show (dialog->cancel);
  
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  g_signal_emit (dialog, signals[SIGNAL_DELETE], 0);
}

