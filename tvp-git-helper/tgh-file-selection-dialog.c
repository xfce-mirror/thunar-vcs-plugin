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

#include "tgh-common.h"
#include "tgh-file-selection-dialog.h"

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
  enum {STATUS_ADDED, STATUS_MODIFIED, STATUS_UNTRACKED} state;
} StatusParser;

static void status_parser_func(StatusParser *, gchar *);
static void selection_cell_toggled (GtkCellRendererToggle *, gchar *, gpointer);
static void selection_all_toggled (GtkToggleButton *, gpointer);

struct _TghFileSelectionDialog
{
  GtkDialog dialog;

  GtkWidget *tree_view;
  GtkWidget *all;

  TghFileSelectionFlags flags;
};

struct _TghFileSelectionDialogClass
{
  GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TghFileSelectionDialog, tgh_file_selection_dialog, GTK_TYPE_DIALOG)

static gchar *argv[] = {"git", "--no-pager", "status", NULL};

static void
tgh_file_selection_dialog_class_init (TghFileSelectionDialogClass *klass)
{
}

enum {
  COLUMN_PATH = 0,
  COLUMN_STAT,
  COLUMN_SELECTION,
  COLUMN_COUNT
};

static void
tgh_file_selection_dialog_init (TghFileSelectionDialog *dialog)
{
  GtkWidget *tree_view;
  GtkWidget *scroll_window;
  GtkWidget *all;
  GtkCellRenderer *renderer;
  GtkTreeModel *model;

  scroll_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->tree_view = tree_view = gtk_tree_view_new ();

  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled", G_CALLBACK (selection_cell_toggled), dialog);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               -1, "", renderer,
                                               "active", COLUMN_SELECTION,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               -1, _("Path"), renderer,
                                               "text", COLUMN_PATH,
                                               NULL);
  
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               -1, ("State"), renderer,
                                               "text", COLUMN_STAT,
                                               NULL);

  model = GTK_TREE_MODEL (gtk_list_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN));

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

  g_object_unref (model);

  gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scroll_window, TRUE, TRUE, 0);
  gtk_widget_show (tree_view);
  gtk_widget_show (scroll_window);

  dialog->all = all = gtk_check_button_new_with_label (_("Select/Unselect all"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (all), TRUE);
  gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (all), TRUE);
  g_signal_connect (all, "toggled", G_CALLBACK (selection_all_toggled), dialog);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), all, FALSE, FALSE, 0);
  gtk_widget_show (all);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Status"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                          NULL);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
}

static TghOutputParser* status_parser_new (GtkWidget *dialog)
{
  StatusParser *parser = g_new(StatusParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(status_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

GtkWidget*
tgh_file_selection_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, TghFileSelectionFlags selection_flags)
{
  GPid pid;
  gint fd_out, fd_err;
  GError *error = NULL;
  TghOutputParser *parser;
  GIOChannel *chan_out, *chan_err;

  TghFileSelectionDialog *dialog = g_object_new (TGH_TYPE_FILE_SELECTION_DIALOG, NULL);

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

  dialog->flags = selection_flags;

  if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &fd_out, &fd_err, &error))
  {
    return FALSE;
  }

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, status_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return GTK_WIDGET(dialog);
}

gchar**
tgh_file_selection_dialog_get_files (TghFileSelectionDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar **files, **files_iter;
  guint count = 0;
  gboolean selection;

  g_return_val_if_fail (TGH_IS_FILE_SELECTION_DIALOG (dialog), NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    do {
      gtk_tree_model_get (model, &iter,
                          COLUMN_SELECTION, &selection,
                          -1);
      if (selection)
        count++;
    } while (gtk_tree_model_iter_next (model, &iter));
  }

  if (!count)
    return NULL;

  files_iter = files = g_new(gchar *, count+1);

  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    do {
      gtk_tree_model_get (model, &iter,
                          COLUMN_SELECTION, &selection,
                          -1);
      if (selection)
      {
        gtk_tree_model_get (model, &iter,
                            COLUMN_PATH, files_iter,
                            -1);
        files_iter++;
      }
    } while (gtk_tree_model_iter_next (model, &iter));
  }

  *files_iter = NULL;

  return files;
}

static void
status_parser_func(StatusParser *parser, gchar *line)
{
  TghFileSelectionDialog *dialog = TGH_FILE_SELECTION_DIALOG(parser->dialog);
  if(line)
  {
    gboolean add = FALSE;
    gboolean select_ = FALSE;
    if(line[0] == '#' && line[1] == '\t')
    {
      gchar *file = strchr(line, ':');
      gchar *state = _("untracked");
      if(file && parser->state != STATUS_UNTRACKED)
      {
        *file = '\0';
        state = line+2;
        file = line+14;
      }
      else
        file = line+2;
      file[strlen(file)-1] = '\0';
      file = g_shell_unquote(file, NULL);

      switch(parser->state)
      {
        case STATUS_ADDED:
          if(dialog->flags & TGH_FILE_SELECTION_FLAG_ADDED)
            add = TRUE;
          select_ = TRUE;
          break;
        case STATUS_MODIFIED:
          if(dialog->flags & TGH_FILE_SELECTION_FLAG_MODIFIED)
            add = TRUE;
          if(!(dialog->flags & TGH_FILE_SELECTION_FLAG_ADDED))
            select_ = TRUE;
          break;
        case STATUS_UNTRACKED:
          if(dialog->flags & TGH_FILE_SELECTION_FLAG_UNTRACKED)
            add = TRUE;
          if(!(dialog->flags & (TGH_FILE_SELECTION_FLAG_ADDED|TGH_FILE_SELECTION_FLAG_MODIFIED)))
            select_ = TRUE;
          break;
      }

      if (add)
      {
        GtkTreeModel *model;
        GtkTreeIter iter;

        model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            COLUMN_PATH, file,
                            COLUMN_STAT, state,
                            COLUMN_SELECTION, select_,
                            -1);
      }

      g_free(file);
    }
    else if(strstr(line, "git reset"))
      parser->state = STATUS_ADDED;
    else if(strstr(line, "git add"))
      parser->state = STATUS_UNTRACKED;
    else if(strstr(line, "git checkout"))
      parser->state = STATUS_MODIFIED;
  }
  else
  {
    g_free(parser);
  }
}

static void selection_cell_toggled (GtkCellRendererToggle *renderer, gchar *path, gpointer user_data)
{
  TghFileSelectionDialog *dialog = TGH_FILE_SELECTION_DIALOG (user_data);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean selection;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  gtk_tree_model_get_iter_from_string (model, &iter, path);

  gtk_tree_model_get (model, &iter,
                      COLUMN_SELECTION, &selection,
                      -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_SELECTION, !selection,
                      -1);

  gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (dialog->all), TRUE);
}

static void selection_all_toggled (GtkToggleButton *button, gpointer user_data)
{
  TghFileSelectionDialog *dialog = TGH_FILE_SELECTION_DIALOG (user_data);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean selection;

  gtk_toggle_button_set_inconsistent (button, FALSE);
  
  selection = gtk_toggle_button_get_active (button);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    do {
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          COLUMN_SELECTION, selection,
                          -1);
    } while (gtk_tree_model_iter_next (model, &iter));
  }
}

