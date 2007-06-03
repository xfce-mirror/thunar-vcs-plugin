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

#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_fs.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-update-dialog.h"

#include "tsh-update.h"

static void tsh_notify_func2(void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool)
{
	TshUpdateDialog *dialog = TSH_UPDATE_DIALOG (baton);
	char buffer[256];

	gdk_threads_enter ();

	switch(notify->action)
	{
		case svn_wc_notify_update_delete:
			tsh_update_dialog_add(dialog, _("Deleted"), notify->path, notify->mime_type);
			break;
		case svn_wc_notify_update_add:
			tsh_update_dialog_add(dialog, _("Added"), notify->path, notify->mime_type);
			break;
		case svn_wc_notify_update_update:
			tsh_update_dialog_add(dialog, _("Updated"), notify->path, notify->mime_type);
			break;
		case svn_wc_notify_update_completed:
			g_snprintf(buffer, 256, _("At revision: %li"), notify->revision);
			tsh_update_dialog_add(dialog, _("Completed"), buffer, NULL);
			break;
		case svn_wc_notify_update_external:
			tsh_update_dialog_add(dialog, _("External"), notify->path, notify->mime_type);
			break;
		default:
			break;
	}

	gdk_threads_leave ();
}

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshUpdateDialog *dialog;
	gchar **files;
};

gpointer update_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  svn_opt_revision_t revision;
	svn_error_t *err;
	apr_array_header_t *paths;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *pool = args->pool;
	TshUpdateDialog *dialog = args->dialog;
	gchar **files = args->files;
	gint size, i;

	g_free (args);

	size = files?g_strv_length(files):0;

	if(size)
	{
		paths = apr_array_make (pool, size, sizeof (const char *));
		
		for (i = 0; i < size; i++)
		{
			APR_ARRAY_PUSH (paths, const char *) = files[i];
		}
	}
	else
	{
		paths = apr_array_make (pool, 1, sizeof (const char *));
		
		APR_ARRAY_PUSH (paths, const char *) = ""; // current directory
	}

	//APR_ARRAY_PUSH (paths, const char *) = "/home/cavalier/xfce/svn/squeeze";

  revision.kind = svn_opt_revision_head;
	if ((err = svn_client_update2(NULL, paths, &revision, TRUE, TRUE, ctx, pool)))
	{
		tsh_update_dialog_done (dialog);

		svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

	tsh_update_dialog_done (dialog);
	
	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_update (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_update_dialog_new (NULL, NULL, 0);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_UPDATE_DIALOG (dialog);
	args->files = files;

	return g_thread_create (update_thread, args, TRUE, NULL);
}

