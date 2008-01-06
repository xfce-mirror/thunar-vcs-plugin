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

#include "tsh-cleanup.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  GtkWidget *dialog;
	gchar *path;
};

static gpointer cleanup_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
  GtkWidget *dialog = args->dialog;
	gchar *path = args->path;

	g_free (args);

  if(!path)
    path = "";

  subpool = svn_pool_create (pool);

	if ((err = svn_client_cleanup(path, ctx, subpool)))
	{
    svn_pool_destroy (subpool);

		svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
  gtk_widget_destroy(dialog);
  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, _("Cleanup finnished"));
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);
	gdk_threads_leave();

	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_cleanup (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	struct thread_args *args;
  GtkWidget *dialog;
  gchar *path;

  dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CANCEL, _("Cleaning up ..."));
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (tsh_cancel), NULL);
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);

	path = files?files[0]:NULL;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
  args->dialog = dialog;
	args->path = path;

	return g_thread_create (cleanup_thread, args, TRUE, NULL);
}

