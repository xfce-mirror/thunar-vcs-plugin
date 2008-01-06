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
#include "tsh-status-dialog.h"

#include "tsh-status.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshStatusDialog *dialog;
	gchar **files;
};

static gpointer status_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  svn_opt_revision_t revision;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshStatusDialog *dialog = args->dialog;
	gchar **files = args->files;
  gboolean recursive;
  gboolean get_all;
  gboolean update;
  gboolean no_ignore;
  gboolean ignore_externals;

  gdk_threads_enter();
  recursive = tsh_status_dialog_get_show_recursive(dialog);
  get_all = tsh_status_dialog_get_show_unmodified(dialog);
  update = tsh_status_dialog_get_check_reposetory(dialog);
  no_ignore = tsh_status_dialog_get_show_ignore(dialog);
  ignore_externals = tsh_status_dialog_get_hide_externals(dialog);
  gdk_threads_leave();

  subpool = svn_pool_create (pool);

  revision.kind = svn_opt_revision_head;
	if ((err = svn_client_status2(NULL, files?files[0]:"", &revision, tsh_status_func2, dialog, recursive, get_all, update, no_ignore, ignore_externals, ctx, subpool)))
	{
    svn_pool_destroy (subpool);

		gdk_threads_enter();
		tsh_status_dialog_done (dialog);
		gdk_threads_leave();

		svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
	tsh_status_dialog_done (dialog);
	gdk_threads_leave();
	
	return GINT_TO_POINTER (TRUE);
}

static void create_status_thread(TshStatusDialog *dialog, struct thread_args *args)
{
	GThread *thread = g_thread_create (status_thread, args, TRUE, NULL);
  if (thread)
    tsh_replace_thread (thread);
  else
    tsh_status_dialog_done (dialog);
}

GThread *tsh_status (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_status_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = NULL;
	ctx->notify_baton2 = NULL;

  args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_STATUS_DIALOG (dialog);
	args->files = files;

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_status_thread), args);

	return g_thread_create (status_thread, args, TRUE, NULL);
}

