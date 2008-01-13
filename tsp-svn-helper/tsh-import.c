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

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-notify-dialog.h"
#include "tsh-transfer-dialog.h"
#include "tsh-log-message-dialog.h"

#include "tsh-import.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
	gchar *path;
	gchar *url;
};

static gpointer import_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
  svn_commit_info_t *commit_info;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshNotifyDialog *dialog = args->dialog;
	gchar *path = args->path;
	gchar *url = args->url;
  gchar *error_str;

	g_free (args);

  subpool = svn_pool_create (pool);

	if ((err = svn_client_import2(&commit_info, path, url, FALSE, FALSE, ctx, subpool)))
	{
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
		gdk_threads_enter();
    tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
		tsh_notify_dialog_done (dialog);
		gdk_threads_leave();
    g_free(error_str);

		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
	tsh_notify_dialog_done (dialog);
	gdk_threads_leave();
	
	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_import (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;
  gchar *repository = NULL;
  gchar *path = NULL;

	dialog = tsh_transfer_dialog_new (_("Import"), NULL, 0, NULL, files?files[0]:NULL);
	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }

  repository = tsh_transfer_dialog_get_reposetory(TSH_TRANSFER_DIALOG(dialog));
  path = tsh_is_working_copy(repository, pool);
  if(path)
  {
    g_free(repository);
    repository = path;
  }
  path = tsh_transfer_dialog_get_directory(TSH_TRANSFER_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	dialog = tsh_notify_dialog_new (_("Import"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

  ctx->log_msg_func2 = tsh_log_msg_func2;
  ctx->log_msg_baton2 = tsh_log_message_dialog_new (_("Import Message"), GTK_WINDOW (dialog), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->path = path;
	args->url =	repository;

	return g_thread_create (import_thread, args, TRUE, NULL);
}

