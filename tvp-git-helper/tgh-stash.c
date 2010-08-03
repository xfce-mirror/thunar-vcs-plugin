/*-
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-stash-dialog.h"

#include "tgh-stash.h"

struct exit_args
{
  TghOutputParser *parser;
  TghStashDialog *dialog;
};

static gboolean stash_list_spawn (TghStashDialog *dialog, GPid *pid);

static void child_exit (GPid pid, gint status, gpointer user_data)
{
  struct exit_args *args = user_data;

  tgh_child_exit (pid, status, args->parser);

  if (stash_list_spawn (args->dialog, &pid))
    tgh_replace_child (TRUE, pid);
  else
    tgh_stash_dialog_done (args->dialog);

  g_free (args);
}

static gchar *argv_list[] = {"git", "--no-pager", "stash", "list", NULL};
static gchar *argv_clear[] = {"git", "--no-pager", "stash", "clear", NULL};

static gboolean stash_list_spawn (TghStashDialog *dialog, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;

  if(!g_spawn_async_with_pipes(NULL, argv_list, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    return FALSE;
  }

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_stash_list_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_show_spawn (TghStashDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gchar **argv;

  argv = g_new (gchar*, 7);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "stash";
  argv[3] = "show";
  argv[4] = "--numstat";
  argv[5] = (gchar*)name;
  argv[6] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  g_child_watch_add (*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new (fd_out);
  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_stash_show_parser_new (GTK_WIDGET (dialog)));
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_save_spawn (TghStashDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  gchar **argv;
  struct exit_args *args;

  argv = g_new (gchar*, 7);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "stash";
  argv[3] = "save";
  argv[4] = "-q";
  argv[5] = (gchar*)name;
  argv[6] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_apply_spawn (TghStashDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  gchar **argv;
  struct exit_args *args;

  argv = g_new (gchar*, 7);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "stash";
  argv[3] = "apply";
  argv[4] = "-q";
  argv[5] = (gchar*)name;
  argv[6] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_pop_spawn (TghStashDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  gchar **argv;
  struct exit_args *args;

  argv = g_new (gchar*, 7);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "stash";
  argv[3] = "pop";
  argv[4] = "-q";
  argv[5] = (gchar*)name;
  argv[6] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_drop_spawn (TghStashDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  gchar **argv;
  struct exit_args *args;

  argv = g_new (gchar*, 7);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "stash";
  argv[3] = "drop";
  argv[4] = "-q";
  argv[5] = (gchar*)name;
  argv[6] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean stash_clear_spawn (TghStashDialog *dialog, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  struct exit_args *args;

  if (!g_spawn_async_with_pipes (NULL, argv_clear, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
    return FALSE;

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static void show_stash (TghStashDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (stash_show_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

static void save_stash (TghStashDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (stash_save_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

static void apply_stash (TghStashDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (stash_apply_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

static void pop_stash (TghStashDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (stash_pop_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

static void drop_stash (TghStashDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (stash_drop_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

static void clear_stash (TghStashDialog *dialog, gpointer user_data)
{
  GPid pid;
  if (stash_clear_spawn(dialog, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_stash_dialog_done(dialog);
}

gboolean tgh_stash (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  if (files)
    if (chdir(files[0]))
      return FALSE;

  dialog = tgh_stash_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  g_signal_connect(dialog, "selection-changed", G_CALLBACK (show_stash), NULL);
  g_signal_connect(dialog, "save-clicked", G_CALLBACK (save_stash), NULL);
  g_signal_connect(dialog, "apply-clicked", G_CALLBACK (apply_stash), NULL);
  g_signal_connect(dialog, "pop-clicked", G_CALLBACK (pop_stash), NULL);
  g_signal_connect(dialog, "drop-clicked", G_CALLBACK (drop_stash), NULL);
  g_signal_connect(dialog, "clear-clicked", G_CALLBACK (clear_stash), NULL);

  return stash_list_spawn(TGH_STASH_DIALOG(dialog), pid);
}

