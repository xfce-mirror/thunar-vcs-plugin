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
#include "tsh-blame-dialog.h"

#include "tsh-blame.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshBlameDialog *dialog;
	gchar *file;
};

static gpointer blame_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  svn_opt_revision_t revision, start, end;
  svn_diff_file_options_t diff_options;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshBlameDialog *dialog = args->dialog;
	gchar *file = args->file;
  GtkWidget *error;
  gchar *error_str;

  subpool = svn_pool_create (pool);

  diff_options.ignore_space = svn_diff_file_ignore_space_none;
  diff_options.ignore_eol_style = FALSE;
  revision.kind = svn_opt_revision_unspecified;
  start.kind = svn_opt_revision_number;
  start.value.number = 0;
  end.kind = svn_opt_revision_head;
	if ((err = svn_client_blame4(file, &revision, &start, &end, &diff_options, FALSE, FALSE, tsh_blame_func2, dialog, ctx, subpool)))
	{
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
		gdk_threads_enter();
		tsh_blame_dialog_done (dialog);

    error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Blame failed"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error), error_str);
    tsh_dialog_start(GTK_DIALOG(error), FALSE);
		gdk_threads_leave();
    g_free(error_str);

		svn_error_clear(err);
    tsh_reset_cancel();
		return GINT_TO_POINTER (FALSE);
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
	tsh_blame_dialog_done (dialog);
	gdk_threads_leave();
	
  tsh_reset_cancel();
	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_blame (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_blame_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = NULL;
	ctx->notify_baton2 = NULL;

  args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_BLAME_DIALOG (dialog);
	args->file = files?files[0]:"";

	return g_thread_create (blame_thread, args, TRUE, NULL);
}

