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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-file-selection-dialog.h"
#include "tsh-notify-dialog.h"
#include "tsh-log-message-dialog.h"

#include "tsh-delete.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
  gchar **files;
};

static gpointer delete_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  svn_error_t *err;
  apr_array_header_t *paths;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  TshNotifyDialog *dialog = args->dialog;
  gchar **files = args->files;
  gint size, i;
  gchar *error_str;
#if CHECK_SVN_VERSION_S(1,6)
  svn_commit_info_t *commit_info;
  gchar *message;
  gchar buffer[256];
#endif

  g_free (args);

  size = files?g_strv_length(files):0;

  subpool = svn_pool_create (pool);

  if(size)
  {
    paths = apr_array_make (subpool, size, sizeof (const char *));

    for (i = 0; i < size; i++)
    {
      APR_ARRAY_PUSH (paths, const char *) = files[i];
    }
  }
  else
  {
    paths = apr_array_make (subpool, 1, sizeof (const char *));

    APR_ARRAY_PUSH (paths, const char *) = ""; // current directory
  }

#if CHECK_SVN_VERSION_S(1,6)
  if ((err = svn_client_delete3(&commit_info, paths, FALSE, FALSE, NULL, ctx, subpool)))
#else /* CHECK_SVN_VERSION(1,7) */
  if ((err = svn_client_delete4(paths, FALSE, FALSE, NULL, tsh_commit_func2, dialog, ctx, subpool)))
#endif
  {
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_enter();
    tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
    tsh_notify_dialog_done (dialog);
    gdk_threads_leave();
G_GNUC_END_IGNORE_DEPRECATIONS

    g_free(error_str);

    svn_error_clear(err);
    tsh_reset_cancel();
    return GINT_TO_POINTER (FALSE);
  }

#if CHECK_SVN_VERSION_S(1,6)
  if(commit_info && SVN_IS_VALID_REVNUM(commit_info->revision))
  {
    g_snprintf(buffer, 256, _("At revision: %ld"), commit_info->revision);
    message = buffer;
  }
  else
  {
    message = _("Local delete");
  }
#endif

  svn_pool_destroy (subpool);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  gdk_threads_enter();
#if CHECK_SVN_VERSION_S(1,6)
  tsh_notify_dialog_add(dialog, _("Completed"), message, NULL);
#endif
  tsh_notify_dialog_done (dialog);
  gdk_threads_leave();
G_GNUC_END_IGNORE_DEPRECATIONS

  tsh_reset_cancel();
  return GINT_TO_POINTER (TRUE);
}

GThread *tsh_delete (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

  dialog = tsh_file_selection_dialog_new (_("Delete"), NULL, 0, files, TSH_FILE_SELECTION_FLAG_RECURSIVE|TSH_FILE_SELECTION_FLAG_UNCHANGED, ctx, pool);
	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }
  g_strfreev (files);
  files = tsh_file_selection_dialog_get_files (TSH_FILE_SELECTION_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if(!files)
    return NULL;

	dialog = tsh_notify_dialog_new (_("Delete"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

  ctx->log_msg_func2 = tsh_log_msg_func2;
  ctx->log_msg_baton2 = tsh_log_message_dialog_new (_("Delete Message"), GTK_WINDOW (dialog), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->files = files;

	return g_thread_new (NULL, delete_thread, args);
}

