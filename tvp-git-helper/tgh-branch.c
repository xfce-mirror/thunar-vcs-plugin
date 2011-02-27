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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-branch-dialog.h"

#include "tgh-branch.h"

struct exit_args
{
  TghOutputParser *parser;
  TghBranchDialog *dialog;
};

static gboolean branch_spawn (TghBranchDialog *dialog, GPid *pid);

static void child_exit (GPid pid, gint status, gpointer user_data)
{
  struct exit_args *args = user_data;

  tgh_child_exit (pid, status, args->parser);

  if (branch_spawn (args->dialog, &pid))
    tgh_replace_child (TRUE, pid);
  else
    tgh_branch_dialog_done (args->dialog);

  g_free (args);
}

static gboolean branch_spawn (TghBranchDialog *dialog, GPid *pid)
{
  GError *error = NULL;
  gint fd_out;
  gint fd_err;
  GIOChannel *chan_out;
  GIOChannel *chan_err;
  TghOutputParser *parser;

  static const gchar *argv[] = {"git", "--no-pager", "branch", NULL};

  if(!g_spawn_async_with_pipes(NULL, (gchar**)argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    return FALSE;
  }

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_branch_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean branch_checkout_spawn (TghBranchDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  const gchar **argv;
  struct exit_args *args;

  argv = g_new (const gchar*, 6);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "checkout";
  argv[3] = "-q";
  argv[4] = name;
  argv[5] = NULL;

  if(!g_spawn_async_with_pipes(NULL, (gchar**)argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static gboolean branch_create_spawn (TghBranchDialog *dialog, const gchar *name, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  const gchar **argv;
  struct exit_args *args;

  argv = g_new (const gchar*, 5);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "branch";
  argv[3] = name;
  argv[4] = NULL;

  if(!g_spawn_async_with_pipes(NULL, (gchar**)argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add(*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static void checkout_branch(TghBranchDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (branch_checkout_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_branch_dialog_done(dialog);
}

static void create_branch(TghBranchDialog *dialog, const gchar *name, gpointer user_data)
{
  GPid pid;
  if (branch_create_spawn(dialog, name, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_branch_dialog_done(dialog);
}

gboolean tgh_branch (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  if (files)
    if (chdir(files[0]))
      return FALSE;

  dialog = tgh_branch_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  g_signal_connect(dialog, "checkout-clicked", G_CALLBACK (checkout_branch), NULL);
  g_signal_connect(dialog, "create-clicked", G_CALLBACK (create_branch), NULL);

  return branch_spawn(TGH_BRANCH_DIALOG(dialog), pid);
}

