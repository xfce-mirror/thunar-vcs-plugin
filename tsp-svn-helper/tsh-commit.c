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
#include "tsh-notify-dialog.h"
#include "tsh-log-message-dialog.h"

#include "tsh-commit.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
  gchar **files;
};

static gpointer commit_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
  svn_commit_info_t *commit_info;
  apr_array_header_t *paths;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *pool = args->pool;
	TshNotifyDialog *dialog = args->dialog;
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

	if ((err = svn_client_commit3(&commit_info, paths, TRUE, FALSE, ctx, pool)))
	{
		gdk_threads_enter();
		tsh_notify_dialog_done (dialog);
		gdk_threads_leave();

		svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

	gdk_threads_enter();
	tsh_notify_dialog_done (dialog);
	gdk_threads_leave();
	
	return GINT_TO_POINTER (TRUE);
}

static svn_error_t*
tsh_log_msg_func(const char **log_msg, const char **tmp_file, const apr_array_header_t *commit_items, void *baton, apr_pool_t *pool)
{
  int i;
  GtkWidget *dialog = baton;

  gdk_threads_enter();
	gtk_widget_show (dialog);
  gdk_threads_leave();

  if(commit_items)
  {
    for(i = 0; i < commit_items->nelts; i++)
    {
      const gchar *state = _("Unknown");
      svn_client_commit_item2_t *item = APR_ARRAY_IDX(commit_items, i, svn_client_commit_item2_t*);
      if((item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD) &&
        (item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE))
        state = _("Replaced");
      else if(item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD)
        state = _("Added");
      else if(item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE)
        state = _("Deleted");
      else if((item->state_flags & SVN_CLIENT_COMMIT_ITEM_TEXT_MODS) ||
        (item->state_flags & SVN_CLIENT_COMMIT_ITEM_PROP_MODS))
        state = _("Modified");
      //else if(item->state_flags & SVN_CLIENT_COMMIT_ITEM_PROP_MODS)
      //  state = _("Modified");
      else if(item->state_flags & SVN_CLIENT_COMMIT_ITEM_IS_COPY)
        state = _("Copied");
      else if(item->state_flags & SVN_CLIENT_COMMIT_ITEM_LOCK_TOKEN)
        state = _("Unlocked");
      gdk_threads_enter();
      tsh_log_message_dialog_add(TSH_LOG_MESSAGE_DIALOG(dialog), state, item->path);
      gdk_threads_leave();
    }
    gdk_threads_enter();
    if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
    {
      gdk_threads_leave();
      tsh_cancel();
      //gtk_widget_destroy(dialog);
      gdk_threads_enter();
      gtk_widget_hide (dialog);
      gdk_threads_leave();
      return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
    }
    gdk_threads_leave();
    gdk_threads_enter();
    *log_msg = tsh_log_message_dialog_get_message(TSH_LOG_MESSAGE_DIALOG(dialog));
    gdk_threads_leave();
    //gtk_widget_destroy(dialog);
  }

  gdk_threads_enter();
	gtk_widget_hide (dialog);
  gdk_threads_leave();

	return SVN_NO_ERROR;
}

GThread *tsh_commit (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_notify_dialog_new (_("Commit"), NULL, 0);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

  ctx->log_msg_func2 = tsh_log_msg_func;
  ctx->log_msg_baton2 = tsh_log_message_dialog_new (_("Commit Message"), GTK_WINDOW (dialog), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->files = files;

	return g_thread_create (commit_thread, args, TRUE, NULL);
}

