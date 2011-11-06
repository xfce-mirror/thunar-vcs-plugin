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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include <subversion-1/svn_version.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-status-dialog.h"

#include "tsh-status.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
	TshStatusDialog *dialog;
	gchar **files;
};

static gpointer status_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  svn_opt_revision_t revision;
  svn_error_t *err;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  TshStatusDialog *dialog = args->dialog;
  gchar **files = args->files;
  svn_depth_t depth;
  gboolean get_all;
  gboolean update;
  gboolean no_ignore;
  gboolean ignore_externals;
  GtkWidget *error;
  gchar *error_str;

  gdk_threads_enter();
  depth = tsh_status_dialog_get_depth(dialog);
  get_all = tsh_status_dialog_get_show_unmodified(dialog);
  update = tsh_status_dialog_get_check_reposetory(dialog);
  no_ignore = tsh_status_dialog_get_show_ignore(dialog);
  ignore_externals = tsh_status_dialog_get_hide_externals(dialog);
  gdk_threads_leave();

  subpool = svn_pool_create (pool);

  revision.kind = svn_opt_revision_head;
#if CHECK_SVN_VERSION(1,5)
  if ((err = svn_client_status3(NULL, files?files[0]:"", &revision, tsh_status_func2, dialog, depth, get_all, update, no_ignore, ignore_externals, NULL, ctx, subpool)))
#elif CHECK_SVN_VERSION(1,6)
  if ((err = svn_client_status4(NULL, files?files[0]:"", &revision, tsh_status_func3, dialog, depth, get_all, update, no_ignore, ignore_externals, NULL, ctx, subpool)))
#else /* CHECK_SVN_VERSION(1,7) */
  if ((err = svn_client_status5(NULL, ctx, files?files[0]:"", &revision, depth, get_all, update, no_ignore, ignore_externals, TRUE, NULL, tsh_status_func, dialog, subpool)))
#endif
  {
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
    gdk_threads_enter();
    tsh_status_dialog_done (dialog);

    error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Status failed"));
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
  tsh_status_dialog_done (dialog);
  gdk_threads_leave();

  tsh_reset_cancel();
  return GINT_TO_POINTER (TRUE);
}

static void create_status_thread(TshStatusDialog *dialog, struct thread_args *args)
{
	GThread *thread = g_thread_create (status_thread, args, TRUE, NULL);
  if (thread)
    tsh_replace_thread (thread);
  else
    tsh_status_dialog_done (dialog);
}

GThread *tsh_status (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	GtkWidget *dialog;
	struct thread_args *args;

	dialog = tsh_status_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
	tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

	ctx->notify_func2 = NULL;
	ctx->notify_baton2 = NULL;

  args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
	args->dialog = TSH_STATUS_DIALOG (dialog);
	args->files = files;

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_status_thread), args);

	return g_thread_create (status_thread, args, TRUE, NULL);
}

