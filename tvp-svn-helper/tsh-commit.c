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
#include "tsh-file-selection-dialog.h"
#include "tsh-notify-dialog.h"
#include "tsh-log-message-dialog.h"

#include "tsh-commit.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
  GSList *files;
};

static gpointer commit_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  gboolean result = TRUE;
  svn_error_t *err;
  svn_commit_info_t *commit_info;
  apr_array_header_t *paths;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  TshNotifyDialog *dialog = args->dialog;
  GSList *files = args->files;
  GSList *iter;
  GSList *delete = NULL;
  //GSList *reverse_start = NULL, *reverse_end = NULL;
  gint delete_size = 0;
  gint size = 0;
  gint size_indirect = 0;
  gboolean recursive = TRUE;
  gchar *error_str;
  gchar *message;
  gchar buffer[256];

  g_free (args);

  subpool = svn_pool_create (pool);

  for (iter = files; result && iter; iter = g_slist_next (iter))
  {
    TshFileInfo *info;
    size_indirect++;

    info = iter->data;

    if (!(info->flags & TSH_FILE_INFO_INDIRECT))
    {
      if (!(info->flags & TSH_FILE_INFO_RECURSIVE))
        recursive = FALSE;
      size++;
      switch (info->status)
      {
        case TSH_FILE_STATUS_MISSING:
          delete_size++;
          delete = g_slist_prepend (delete, info);
          break;
        case TSH_FILE_STATUS_UNVERSIONED:
          if ((err = svn_client_add4(info->path, (info->flags&TSH_FILE_INFO_RECURSIVE)?svn_depth_infinity:svn_depth_empty, FALSE, FALSE, FALSE, ctx, subpool)))
          {
            error_str = tsh_strerror(err);
            gdk_threads_enter();
            tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
            gdk_threads_leave();
            g_free(error_str);

            svn_error_clear(err);
            result = FALSE;//FIXME: needed ??
          }
          break;
        default:
          break;
      }
    }
  }

  if (result && delete_size)
  {
    paths = apr_array_make (subpool, delete_size, sizeof (const char *));

    for (iter = delete; iter; iter = g_slist_next (iter))
    {
      TshFileInfo *info = iter->data;
      APR_ARRAY_PUSH (paths, const char *) = info->path;
    }

    if ((err = svn_client_delete3(NULL, paths, FALSE, FALSE, NULL, ctx, subpool)))
    {
      svn_pool_destroy (subpool);

      error_str = tsh_strerror(err);
      gdk_threads_enter();
      tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
      gdk_threads_leave();
      g_free(error_str);

      svn_error_clear(err);
      result = FALSE;//FIXME: needed ??
    }
  }

  g_slist_free (delete);

  svn_pool_destroy (subpool);

  /* check if an error occurred in add commands */
  if (result)//FIXME: needed ??
  {
    subpool = svn_pool_create (pool);

    if(recursive && size)
    {
      paths = apr_array_make (subpool, size, sizeof (const char *));

      for (iter = files; iter; iter = g_slist_next (iter))
      {
        TshFileInfo *info = iter->data;
        if (!(info->flags & TSH_FILE_INFO_INDIRECT))
        {
          APR_ARRAY_PUSH (paths, const char *) = info->path;
        }
      }
    }
    else if(size_indirect)
    {
      /* Set recursive to false if it wasn't already, FIXME: needed?? */
      recursive = FALSE;

      paths = apr_array_make (subpool, size_indirect, sizeof (const char *));

      for (iter = files; iter; iter = g_slist_next (iter))
      {
        TshFileInfo *info = iter->data;
        APR_ARRAY_PUSH (paths, const char *) = info->path;
      }
    }
    else
    {
      paths = apr_array_make (subpool, 1, sizeof (const char *));

      APR_ARRAY_PUSH (paths, const char *) = ""; // current directory
    }

    if ((err = svn_client_commit4(&commit_info, paths, recursive?svn_depth_infinity:svn_depth_empty, FALSE, FALSE, NULL, NULL, ctx, subpool)))
    {
      svn_pool_destroy (subpool);

      error_str = tsh_strerror(err);
      gdk_threads_enter();
      tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
      tsh_notify_dialog_done (dialog);
      gdk_threads_leave();
      g_free(error_str);

      svn_error_clear(err);
      tsh_reset_cancel();
      return GINT_TO_POINTER (FALSE);
    }

    if(SVN_IS_VALID_REVNUM(commit_info->revision))
    {
      g_snprintf(buffer, 256, _("At revision: %ld"), commit_info->revision);
      message = buffer;
    }
    else
    {
      message = _("Nothing to do");
    }

    svn_pool_destroy (subpool);
  }

  gdk_threads_enter();
  if (result)
    tsh_notify_dialog_add(dialog, _("Completed"), message, NULL);
  tsh_notify_dialog_done (dialog);
  gdk_threads_leave();

  tsh_reset_cancel();
  return GINT_TO_POINTER (result);
}

GThread *tsh_commit (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;
  GSList *file_list;

  dialog = tsh_file_selection_dialog_new (_("Commit"), NULL, 0, files, TSH_FILE_SELECTION_FLAG_RECURSIVE|TSH_FILE_SELECTION_FLAG_MODIFIED|TSH_FILE_SELECTION_FLAG_UNVERSIONED, ctx, pool);
	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }
  g_strfreev (files);
  file_list = tsh_file_selection_dialog_get_file_info_by_status (TSH_FILE_SELECTION_DIALOG (dialog), TSH_FILE_STATUS_INVALID, TRUE);
  gtk_widget_destroy (dialog);

  if(!file_list)
    return NULL;

	dialog = tsh_notify_dialog_new (_("Commit"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

  ctx->log_msg_func2 = tsh_log_msg_func2;
  ctx->log_msg_baton2 = tsh_log_message_dialog_new (_("Commit Message"), GTK_WINDOW (dialog), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->files = file_list;

	return g_thread_create (commit_thread, args, TRUE, NULL);
}

