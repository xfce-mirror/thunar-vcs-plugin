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

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-properties-dialog.h"

#include "tsh-properties.h"

struct thread_args {
	svn_client_ctx_t *ctx;
	apr_pool_t *pool;
  TshPropertiesDialog *dialog;
	gchar *path;
};

static gpointer properties_thread (gpointer user_data)
{
	struct thread_args *args = user_data;
  svn_opt_revision_t revision;
	svn_error_t *err;
	svn_client_ctx_t *ctx = args->ctx;
	apr_pool_t *pool = args->pool;
  TshPropertiesDialog *dialog = args->dialog;
	gchar *path = args->path;
  apr_array_header_t *prop_items;

	g_free (args);

  revision.kind = svn_opt_revision_unspecified;
	if ((err = svn_client_proplist2(&prop_items, path, &revision, &revision, FALSE, ctx, pool)))
	{
		gdk_threads_enter();
    tsh_properties_dialog_done (dialog);
		gdk_threads_leave();

		svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
		svn_error_clear(err);
		return GINT_TO_POINTER (FALSE);
	}

  if(prop_items)
  {
    if(prop_items->nelts)
    {
      apr_hash_index_t *hi;
      svn_client_proplist_item_t *item = APR_ARRAY_IDX(prop_items, 0, svn_client_proplist_item_t*);

      for (hi = apr_hash_first(pool, item->prop_hash); hi; hi = apr_hash_next(hi)) {
        const char *name;
        svn_string_t *value;
        gchar *str_value;
        apr_hash_this(hi, (const void**)&name, NULL, (void**)&value);
        str_value = g_strndup (value->data, value->len);
        gdk_threads_enter();
        tsh_properties_dialog_add (dialog, name, str_value);
        gdk_threads_leave();
        g_free (str_value);
      }
    }
  }

  gdk_threads_enter();
  tsh_properties_dialog_done (dialog);
  gdk_threads_leave();

	return GINT_TO_POINTER (TRUE);
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

	return g_thread_create (properties_thread, args, TRUE, NULL);
}

