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

#include "tsh-resolved.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
	gchar **files;
};

static gpointer resolved_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
  gboolean result;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshNotifyDialog *dialog = args->dialog;
	gchar **files = args->files;
	gint size, i;
  gchar *error_str;

	g_free (args);

	size = files?g_strv_length(files):0;

  subpool = svn_pool_create (pool);

	if(size)
	{
		for (i = 0; i < size; i++)
		{
      if ((err = svn_client_resolved(files[i], TRUE, ctx, subpool)))
      {
        error_str = tsh_strerror(err);
        gdk_threads_enter();
        tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
        gdk_threads_leave();
        g_free(error_str);

        svn_error_clear(err);
        result = FALSE;
        break; //FIXME: needed ??
      }
		}
	}
	else
	{
    if ((err = svn_client_resolved("", TRUE, ctx, subpool)))
    {
      error_str = tsh_strerror(err);
      gdk_threads_enter();
      tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
      gdk_threads_leave();
      g_free(error_str);

      svn_error_clear(err);
      result = FALSE;
    }
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
	tsh_notify_dialog_done (dialog);
	gdk_threads_leave();
	
  tsh_reset_cancel();
	return GINT_TO_POINTER (result);
}

GThread *tsh_resolved (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_notify_dialog_new (_("Resolved"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->files = files;

	return g_thread_create (resolved_thread, args, TRUE, NULL);
}

