/*-
 * Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
 * Copyright (C) 2012 Stefan Sperling <stsp@stsp.name>
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

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_version.h>

#include "tsh-common.h"
#include "tsh-diff-dialog.h"

static void cancel_clicked (GtkButton*, gpointer);
static void refresh_clicked (GtkButton*, gpointer);

struct _TshDiffDialog
{
  GtkDialog dialog;

  GtkWidget *text_view;
  GtkTextTag *tag_red;
  GtkTextTag *tag_green;
  GtkTextTag *tag_bold;
  GtkWidget *close;
  GtkWidget *cancel;
  GtkWidget *refresh;
  gint current_line;
  GtkWidget *depth;
#if CHECK_SVN_VERSION_S(1,7)
  GtkWidget *show_copies_as_adds;
#endif
};

struct _TshDiffDialogClass
{
  GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshDiffDialog, tsh_diff_dialog, GTK_TYPE_DIALOG)

enum {
  SIGNAL_CANCEL = 0,
  SIGNAL_REFRESH,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT];

static void
tsh_diff_dialog_class_init (TshDiffDialogClass *klass)
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

static void
tsh_diff_dialog_init (TshDiffDialog *dialog)
{
  GtkWidget *text_view;
  GtkTextBuffer *text_buffer;
  GtkWidget *scroll_window;
  GtkWidget *button;
  PangoFontDescription *font_desc;
  GtkWidget *table;
  GtkTreeModel *model;
  GtkWidget *depth;
#if CHECK_SVN_VERSION_S(1,7)
  GtkWidget *show_copies_as_adds;
#endif
  GtkCellRenderer *renderer;
  GtkTreeIter iter;

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->text_view = text_view = gtk_text_view_new ();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dialog->text_view));
  dialog->tag_red = gtk_text_buffer_create_tag(text_buffer, NULL,
                                               "foreground", "red", NULL);
  dialog->tag_green = gtk_text_buffer_create_tag(text_buffer, NULL,
                                                 "foreground",
                                                 "forestgreen", NULL);
  dialog->tag_bold = gtk_text_buffer_create_tag(text_buffer, NULL,
                                                "weight",
                                                PANGO_WEIGHT_BOLD, NULL);
  dialog->current_line = 0;

  font_desc = pango_font_description_from_string("Monospace");
  if (font_desc)
  {
    gtk_widget_modify_font(text_view, font_desc);
    pango_font_description_free(font_desc);
  }

  gtk_container_add (GTK_CONTAINER (scroll_window), text_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scroll_window, TRUE, TRUE, 0);
  gtk_widget_show (text_view);
  gtk_widget_show (scroll_window);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Diff"));

  dialog->close = button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  gtk_widget_hide (button);

  dialog->cancel = button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_clicked), dialog);
  gtk_widget_show (button);

  dialog->refresh = button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), button, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (refresh_clicked), dialog);
  gtk_widget_hide (button);

  table = gtk_table_new (2, 1, FALSE);
  model = GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT));
  dialog->depth = depth = gtk_combo_box_new_with_model (model);

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

#if CHECK_SVN_VERSION_S(1,7)
  dialog->show_copies_as_adds = show_copies_as_adds = gtk_check_button_new_with_label (_("Show copies as additions"));
  gtk_table_attach (GTK_TABLE (table), show_copies_as_adds, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (show_copies_as_adds);
#endif

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 400);
}

GtkWidget*
tsh_diff_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
  TshDiffDialog *dialog = g_object_new (TSH_TYPE_DIFF_DIALOG, NULL);

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
tsh_diff_dialog_add (TshDiffDialog *dialog, const char *line, gint len)
{
  GtkTextBuffer *text_buffer;
  GtkTextIter line_start;
  GtkTextTag *tag = NULL;

  g_return_if_fail (TSH_IS_DIFF_DIALOG (dialog));

  if (line[0] == '-')
    tag = dialog->tag_red;
  else if (line[0] == '+')
    tag = dialog->tag_green;
  else if (strncmp(line, "Index", 5) == 0)
    tag = dialog->tag_bold;

  gdk_threads_enter();

  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dialog->text_view));
  gtk_text_buffer_get_iter_at_line(text_buffer, &line_start,
                                   dialog->current_line);
  if (tag)
    gtk_text_buffer_insert_with_tags(text_buffer, &line_start, line, len,
                                     tag, NULL);
  else
    gtk_text_buffer_insert(text_buffer, &line_start, line, len);

  gdk_threads_leave();

  dialog->current_line++;
}

void
tsh_diff_dialog_done (TshDiffDialog *dialog)
{
  g_return_if_fail (TSH_IS_DIFF_DIALOG (dialog));

  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);
  gtk_widget_show (dialog->refresh);
}

static void
cancel_clicked (GtkButton *button, gpointer user_data)
{
  TshDiffDialog *dialog = TSH_DIFF_DIALOG (user_data);
  
  gtk_widget_hide (dialog->cancel);
  gtk_widget_show (dialog->close);
  gtk_widget_show (dialog->refresh);
  
  g_signal_emit (dialog, signals[SIGNAL_CANCEL], 0);
}

static void
refresh_clicked(GtkButton *button, gpointer user_data)
{
  TshDiffDialog *dialog = TSH_DIFF_DIALOG(user_data);
  GtkTextBuffer *text_buffer;

  gtk_widget_hide(dialog->refresh);
  gtk_widget_show(dialog->cancel);

  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dialog->text_view));
  gtk_text_buffer_set_text(text_buffer, "", -1);

  g_signal_emit(dialog, signals[SIGNAL_REFRESH], 0);
}

svn_depth_t
tsh_diff_dialog_get_depth (TshDiffDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  svn_depth_t depth;
  GValue value;

  memset(&value, 0, sizeof(GValue));

  g_return_val_if_fail (TSH_IS_DIFF_DIALOG (dialog), svn_depth_unknown);

  g_return_val_if_fail (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (dialog->depth), &iter), svn_depth_unknown);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dialog->depth));
  gtk_tree_model_get_value (model, &iter, 1, &value);

  depth = g_value_get_int (&value);

  g_value_unset(&value);

  return depth;
}

gboolean
tsh_diff_dialog_get_show_copies_as_adds (TshDiffDialog *dialog)
{
#if CHECK_SVN_VERSION_S(1,7)
  g_return_val_if_fail (TSH_IS_DIFF_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->show_copies_as_adds));
#else /* 1.6 or earlier */
  return FALSE;
#endif
}
