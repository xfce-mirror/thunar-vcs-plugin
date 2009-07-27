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

#include "tsh-add.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshNotifyDialog *dialog;
	GSList *files;
};

static gpointer add_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  gboolean result = TRUE;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
	TshNotifyDialog *dialog = args->dialog;
	GSList *files = args->files;
  gchar *error_str;

	g_free (args);

  subpool = svn_pool_create (pool);

  if(files)
  {
    do
    {
      TshFileInfo *info = files->data;
      if (!(info->flags & TSH_FILE_INFO_INDIRECT))
      {
        if ((err = svn_client_add4(info->path, (info->flags&TSH_FILE_INFO_RECURSIVE)?svn_depth_infinity:svn_depth_empty, FALSE, FALSE, FALSE, ctx, subpool)))
        {
          error_str = tsh_strerror(err);
          gdk_threads_enter();
          tsh_notify_dialog_add(dialog, _("Failed"), error_str, NULL);
          gdk_threads_leave();
          g_free(error_str);

          svn_error_clear(err);
          result = FALSE;
          break;//FIXME: needed ??
        }
      }
    }
    while ((files = g_slist_next (files)));
  }
  else
  {
    if ((err = svn_client_add4("", svn_depth_infinity, FALSE, FALSE, FALSE, ctx, subpool)))
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

GThread *tsh_add (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;
  GSList *file_list;

  dialog = tsh_file_selection_dialog_new (_("Add"), NULL, 0, files, TSH_FILE_SELECTION_FLAG_RECURSIVE|TSH_FILE_SELECTION_FLAG_UNVERSIONED|TSH_FILE_SELECTION_FLAG_AUTO_SELECT_UNVERSIONED, ctx, pool);
	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }
  g_strfreev (files);
  file_list = tsh_file_selection_dialog_get_file_info (TSH_FILE_SELECTION_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if(!file_list)
    return NULL;

	dialog = tsh_notify_dialog_new (_("Add"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->files = file_list;

	return g_thread_create (add_thread, args, TRUE, NULL);
}

