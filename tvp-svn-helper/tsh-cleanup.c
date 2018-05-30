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

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_path.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"

#include "tsh-cleanup.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  GtkWidget *dialog;
	gchar *path;
};

static svn_error_t * tsh_client_cleanup(const char *path,
                                        svn_client_ctx_t *ctx,
                                        apr_pool_t *scratch_pool)
{
#if CHECK_SVN_VERSION_G(1,9)
  const char *local_abspath;

  if (svn_path_is_url(path))
    return svn_error_createf(SVN_ERR_ILLEGAL_TARGET, NULL,
                             _("'%s' is not a local path"), path);

  svn_dirent_get_absolute(&local_abspath, path, scratch_pool);

  return svn_client_cleanup2(local_abspath, TRUE, TRUE, TRUE, TRUE, FALSE, ctx,
                             scratch_pool);
#else
  return svn_client_cleanup(path, ctx, scratch_pool);
#endif
}

static gpointer cleanup_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
  GtkWidget *dialog = args->dialog;
	gchar *path = args->path;
  gchar *error_str;

	g_free (args);

  subpool = svn_pool_create (pool);

	if ((err = tsh_client_cleanup(path, ctx, subpool)))
	{
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
    gdk_threads_enter();
    gtk_widget_destroy(dialog);
    dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Cleanup failed"));
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
  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, _("Cleanup finished"));
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);
	gdk_threads_leave();

  tsh_reset_cancel();
	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_cleanup (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	struct thread_args *args;
  GtkWidget *dialog;
  gchar *path;

  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CANCEL, _("Cleaning up..."));
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (tsh_cancel), NULL);
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);

	path = files?files[0]:"";

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
  args->dialog = dialog;
	args->path = path;

	return g_thread_create (cleanup_thread, args, TRUE, NULL);
}

