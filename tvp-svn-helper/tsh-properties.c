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
#include "tsh-properties-dialog.h"

#include "tsh-properties.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  TshPropertiesDialog *dialog;
	gchar *path;
  gchar *set_key;
  gchar *set_value;
  gboolean depth;
};

static gpointer properties_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  svn_opt_revision_t revision;
  svn_error_t *err;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  TshPropertiesDialog *dialog = args->dialog;
  gchar *path = args->path;
  gchar *set_key = args->set_key;
  gchar *set_value = args->set_value;
  gboolean depth = args->depth;
  svn_string_t *value;
  GtkWidget *error;
  gchar *error_str;
#if CHECK_SVN_VERSION_G(1,7)
  apr_array_header_t *paths;
#endif

  args->set_key = NULL;
  args->set_value = NULL;

  subpool = svn_pool_create (pool);

  if (set_key)
  {
    value = set_value?svn_string_create(set_value, subpool):NULL;

#if CHECK_SVN_VERSION_S(1,6)
    if ((err = svn_client_propset3(NULL, set_key, value, path, depth, FALSE, SVN_INVALID_REVNUM, NULL, NULL, ctx, subpool)))
#else /* CHECK_SVN_VERSION(1,7) */
    paths = apr_array_make (subpool, 1, sizeof (const char *));
    APR_ARRAY_PUSH (paths, const char *) = path;

    if ((err = svn_client_propset_local(set_key, value, paths, depth, FALSE, NULL, ctx, subpool)))
#endif
    {
      //svn_pool_destroy (subpool);
      error_str = tsh_strerror(err);
      gdk_threads_enter();
      //tsh_properties_dialog_done (dialog);

      error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Set property failed"));
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error), "%s", error_str);
      tsh_dialog_start(GTK_DIALOG(error), FALSE);
      gdk_threads_leave();
      g_free(error_str);

      svn_error_clear(err);
      //return GINT_TO_POINTER (FALSE);
    }
  }

  g_free (set_key);
  g_free (set_value);

  revision.kind = svn_opt_revision_unspecified;
  if ((err = svn_client_proplist3(path, &revision, &revision, svn_depth_empty, NULL, tsh_proplist_func, dialog, ctx, subpool)))
  {
    svn_pool_destroy (subpool);

    error_str = tsh_strerror(err);
    gdk_threads_enter();
    tsh_properties_dialog_done (dialog);

    error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Properties failed"));
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
  tsh_properties_dialog_done (dialog);
  gdk_threads_leave();

  tsh_reset_cancel();
  return GINT_TO_POINTER (TRUE);
}

static void create_properties_thread (TshPropertiesDialog *dialog, struct thread_args *args)
{
	GThread *thread = g_thread_create (properties_thread, args, TRUE, NULL);
  if (thread)
    tsh_replace_thread (thread);
  else
    tsh_properties_dialog_done (dialog);
}

static void set_property (TshPropertiesDialog *dialog, struct thread_args *args)
{
  args->set_key = tsh_properties_dialog_get_key (dialog);
  args->set_value = tsh_properties_dialog_get_value (dialog);
  args->depth = tsh_properties_dialog_get_depth (dialog);

  create_properties_thread (dialog, args);
}

static void delete_property (TshPropertiesDialog *dialog, struct thread_args *args)
{
  args->set_key = tsh_properties_dialog_get_selected_key (dialog);
  args->set_value = NULL;

  create_properties_thread (dialog, args);
}

GThread *tsh_properties (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
	struct thread_args *args;
  GtkWidget *dialog;
  gchar *path;

	path = files?files[0]:"";

  dialog = tsh_properties_dialog_new(NULL, NULL, 0);
	g_signal_connect (dialog, "cancel-clicked", tsh_cancel, NULL);
  tsh_dialog_start(GTK_DIALOG(dialog), TRUE);

	ctx->notify_func2 = NULL;
	ctx->notify_baton2 = NULL;

	args = g_malloc (sizeof (struct thread_args));
	args->ctx = ctx;
	args->pool = pool;
  args->dialog = TSH_PROPERTIES_DIALOG (dialog);
	args->path = path;
  args->set_key = NULL;
  args->set_value = NULL;
  args->depth = svn_depth_unknown;

  g_signal_connect(dialog, "set-clicked", G_CALLBACK(set_property), args);
  g_signal_connect(dialog, "delete-clicked", G_CALLBACK(delete_property), args);

	return g_thread_create (properties_thread, args, TRUE, NULL);
}

