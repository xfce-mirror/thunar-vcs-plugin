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
#include <dirent.h>

#include <libxfce4util/libxfce4util.h>

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-notify-dialog.h"

#include "tsh-move.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  TshNotifyDialog *dialog;
	gchar *from;
	gchar *to;
};

static gpointer move_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
	svn_error_t *err;
  svn_commit_info_t *commit_info;
  apr_array_header_t *paths;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *subpool, *pool = args->pool;
  TshNotifyDialog *dialog = args->dialog;
	gchar *from = args->from;
	gchar *to = args->to;
  gchar *error_str;
  gchar *message;
  gchar buffer[256];

	g_free (args);

  subpool = svn_pool_create (pool);

    paths = apr_array_make (subpool, 1, sizeof (const char *));

    APR_ARRAY_PUSH (paths, const char *) = from;

	if ((err = svn_client_move5(&commit_info, paths, to, FALSE, FALSE, FALSE, NULL, ctx, subpool)))
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
    message = _("Local move");
  }

  svn_pool_destroy (subpool);

	gdk_threads_enter();
  tsh_notify_dialog_add(dialog, _("Completed"), message, NULL);
  tsh_notify_dialog_done (dialog);
  gdk_threads_leave();

  tsh_reset_cancel();
	return GINT_TO_POINTER (TRUE);
}

GThread *tsh_move (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	struct thread_args *args;
  GtkWidget *dialog;
  gchar *from;
  gchar *to;
  gboolean isdir = TRUE;
  gchar *absolute = NULL;
  DIR *dir;
  FILE *fp;

	from = files?files[0]:"";

  if(!g_path_is_absolute (from))
  {
    //TODO: ".."
    gchar *currdir = g_get_current_dir();
    absolute = g_build_filename(currdir, (from[0] == '.' && (!from[1] || from[1] == G_DIR_SEPARATOR || from[1] == '/'))?&from[1]:from, NULL);
    g_free (currdir);
  }
  dir = opendir(absolute?absolute:from);
  if(dir)
    closedir(dir);
  else if((fp = fopen(absolute?absolute:from, "r")))
  {
    fclose(fp);
    isdir = FALSE;
  }

  dialog = gtk_file_chooser_dialog_new (_("Move To"), NULL,
                               /*isdir?GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER:*/GTK_FILE_CHOOSER_ACTION_SAVE,
                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                               GTK_STOCK_OK, GTK_RESPONSE_OK,
                               NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), isdir?(absolute?absolute:from):g_path_get_dirname(absolute?absolute:from));

  g_free (absolute);

	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return NULL;
  }

  to = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

	gtk_widget_destroy (dialog);

  dialog = tsh_notify_dialog_new(_("Move"), NULL, 0);
	g_signal_connect (dialog, "cancel-clicked", tsh_cancel, NULL);
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);

	ctx->notify_func2 = tsh_notify_func2;
	ctx->notify_baton2 = dialog;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
  args->dialog = TSH_NOTIFY_DIALOG (dialog);
	args->from = from;
	args->to = to;

	return g_thread_create (move_thread, args, TRUE, NULL);
}

