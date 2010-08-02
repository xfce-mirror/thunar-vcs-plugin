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
#include <subversion-1/svn_props.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-log-dialog.h"

#include "tsh-log.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshLogDialog *dialog;
	gchar **files;
	apr_array_header_t *paths;
};

static gpointer log_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  svn_opt_revision_t revision;
  svn_opt_revision_range_t range;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshLogDialog *dialog = args->dialog;
	gchar **files = args->files;
	apr_array_header_t *paths = args->paths;
	apr_array_header_t *ranges;
	apr_array_header_t *revprops;
	gint size, i;
  gboolean strict_history;
  gboolean merged_revisions;
  GtkWidget *error;
  gchar *error_str;

  gdk_threads_enter ();
  strict_history = tsh_log_dialog_get_hide_copied (dialog);
  merged_revisions = tsh_log_dialog_get_show_merged (dialog);
  gdk_threads_leave ();

  if(!paths)
  {
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

    args->paths = paths;
  }

  subpool = svn_pool_create (pool);

  revprops = apr_array_make (subpool, 3, sizeof (const char*));
  APR_ARRAY_PUSH (revprops, const char*) = SVN_PROP_REVISION_AUTHOR;
  APR_ARRAY_PUSH (revprops, const char*) = SVN_PROP_REVISION_DATE;
  APR_ARRAY_PUSH (revprops, const char*) = SVN_PROP_REVISION_LOG;

  revision.kind = svn_opt_revision_unspecified;
  range.start.kind = svn_opt_revision_head;
  range.end.kind = svn_opt_revision_number;
  range.end.value.number = 0;
  ranges = apr_array_make (subpool, 1, sizeof (svn_opt_revision_range_t *));
  APR_ARRAY_PUSH (ranges, svn_opt_revision_range_t *) = &range;
#if CHECK_SVN_VERSION(1,5)
	if ((err = svn_client_log4(paths, &revision, &range.start, &range.end, 0, TRUE, strict_history, merged_revisions, revprops, tsh_log_func, dialog, ctx, subpool)))
#else /* CHECK_SVN_VERSION(1,6) */
	if ((err = svn_client_log5(paths, &revision, ranges, 0, TRUE, strict_history, merged_revisions, revprops, tsh_log_func, dialog, ctx, subpool)))
#endif
	{
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
		gdk_threads_enter();
		tsh_log_dialog_done (dialog);

    error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Log failed"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error), "%s", error_str);
    tsh_dialog_start(GTK_DIALOG(error), FALSE);
    gdk_threads_leave();
    g_free(error_str);

		svn_error_clear(err);
    tsh_reset_cancel();
		return GINT_TO_POINTER (FALSE);
	}

  svn_pool_destroy (subpool);

	gdk_threads_enter();
	tsh_log_dialog_done (dialog);
	gdk_threads_leave();
	
  tsh_reset_cancel();
	return GINT_TO_POINTER (TRUE);
}

static void create_log_thread(TshLogDialog *dialog, struct thread_args *args)
{
	GThread *thread = g_thread_create (log_thread, args, TRUE, NULL);
  if (thread)
    tsh_replace_thread (thread);
  else
    tsh_log_dialog_done (dialog);
}

GThread *tsh_log (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_log_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = NULL;
	ctx->notify_baton2 = NULL;

  args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_LOG_DIALOG (dialog);
	args->files = files;
  args->paths = NULL;

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_log_thread), args);

	return g_thread_create (log_thread, args, TRUE, NULL);
}

