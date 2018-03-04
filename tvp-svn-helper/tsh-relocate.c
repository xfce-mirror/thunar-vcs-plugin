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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include <subversion-1/svn_version.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-relocate-dialog.h"

#include "tsh-relocate.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  GtkWidget *dialog;
	gchar *path;
	gchar *from;
	gchar *to;
};

static gpointer relocate_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  svn_error_t *err;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  GtkWidget *dialog = args->dialog;
  gchar *path = args->path;
  gchar *from = args->from;
  gchar *to = args->to;
  gchar *error_str;

  g_free (args);

  subpool = svn_pool_create (pool);

#if CHECK_SVN_VERSION_S(1,6)
  if ((err = svn_client_relocate(path, from, to, TRUE, ctx, subpool)))
#else /* CHECK_SVN_VERSION(1,7) */
    if ((err = svn_client_relocate2(path, from, to, TRUE, ctx, subpool)))
#endif
    {
      svn_pool_destroy (subpool);

      error_str = tsh_strerror(err);
      gdk_threads_enter();
      gtk_widget_destroy(dialog);
      dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Relocate failed"));
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error_str);
      tsh_dialog_start(GTK_DIALOG(dialog), TRUE);
      gdk_threads_leave();
      g_free(error_str);

      svn_error_clear(err);
      tsh_reset_cancel();
      return GINT_TO_POINTER (FALSE);
    }

  svn_pool_destroy (subpool);

  gdk_threads_enter();
  gtk_widget_destroy(dialog);
  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, _("Relocate finished"));
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);
  gdk_threads_leave();

  tsh_reset_cancel();
  return GINT_TO_POINTER (TRUE);
}

GThread *tsh_relocate (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
  struct thread_args *args;
  const char *url = NULL;
  gchar *repository = NULL;
  gchar *from;
  gchar *to;
  GtkWidget *dialog;
  gchar *path;
  apr_pool_t *subpool;

  path = files?files[0]:NULL;

  subpool = svn_pool_create(pool);

#if CHECK_SVN_VERSION_S(1,6)
  svn_error_clear(svn_client_url_from_path(&url, path?path:"", subpool));
#else /* CHECK_SVN_VERSION(1,7) */
  svn_error_clear(svn_client_url_from_path2(&url, path?path:"", ctx, subpool, subpool));
#endif
  repository = g_strdup(url);

  svn_pool_destroy(subpool);

  dialog = tsh_relocate_dialog_new (_("Relocate"), NULL, 0, repository, repository, path);
  g_free(repository);
  if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }

  from = tsh_relocate_dialog_get_from(TSH_RELOCATE_DIALOG(dialog));
  to = tsh_relocate_dialog_get_to(TSH_RELOCATE_DIALOG(dialog));
  path = tsh_relocate_dialog_get_directory(TSH_RELOCATE_DIALOG(dialog));

  gtk_widget_destroy (dialog);

  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CANCEL, _("Relocating..."));
  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (tsh_cancel), NULL);
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);

  args = g_malloc (sizeof (struct thread_args));
  args->ctx = ctx;
  args->pool = pool;
  args->dialog = dialog;
  args->path = path;
  args->from = from;
  args->to = to;

  return g_thread_create (relocate_thread, args, TRUE, NULL);
}

