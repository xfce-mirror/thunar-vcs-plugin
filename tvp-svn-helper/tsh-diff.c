/*-
 * Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
 * Copyright (C) 2012 Stefan Sperling <stsp@stsp.name>
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

#include <apr.h>
#include <apr_xlate.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-dialog-common.h"
#include "tsh-file-selection-dialog.h"
#include "tsh-diff-dialog.h"
#include "tsh-log-message-dialog.h"

#include "tsh-diff.h"

struct thread_args {
  svn_client_ctx_t *ctx;
  apr_pool_t *pool;
  TshDiffDialog *dialog;
  gchar **files;
};

static gpointer diff_thread (gpointer user_data)
{
  struct thread_args *args = user_data;
  svn_error_t *err;
  apr_array_header_t *paths;
  svn_client_ctx_t *ctx = args->ctx;
  apr_pool_t *subpool, *pool = args->pool;
  TshDiffDialog *dialog = args->dialog;
  svn_depth_t depth = tsh_diff_dialog_get_depth(dialog);
  svn_boolean_t notice_ancestry = tsh_diff_dialog_get_notice_ancestry(dialog);
  svn_boolean_t no_diff_deleted = tsh_diff_dialog_get_no_diff_deleted(dialog);
  svn_boolean_t show_copies_as_adds = tsh_diff_dialog_get_show_copies_as_adds(dialog);
  gchar **files = args->files;
  gint size, i;
  GtkWidget *error;
  apr_file_t *outfile;
  apr_file_t *errfile;

  size = files?g_strv_length(files):0;

  tsh_diff_dialog_start(dialog);

  subpool = svn_pool_create (pool);

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

  err = svn_io_open_unique_file3(&outfile, NULL, NULL,
                                 svn_io_file_del_on_pool_cleanup,
                                 pool, subpool);
  if (err)
    goto on_error;

  err = svn_io_open_unique_file3(&errfile, NULL, NULL,
                                 svn_io_file_del_on_pool_cleanup,
                                 pool, subpool);
  if (err)
    goto on_error;

  for (i = 0; i < paths->nelts; i++)
  {
    const char *path = APR_ARRAY_IDX(paths, i, const char *);
    svn_opt_revision_t revision1;
    svn_opt_revision_t revision2;
    apr_pool_t *iterpool;
    apr_off_t pos;
    svn_stream_t *stream;

    svn_pool_clear(subpool);

    /* Diff local changes. */
    revision1.kind = svn_opt_revision_base;
    revision2.kind = svn_opt_revision_working;

#if CHECK_SVN_VERSION_G(1,8)
    svn_stream_t *outstream = svn_stream_from_aprfile2(outfile, TRUE, pool);
    svn_stream_t *errstream = svn_stream_from_aprfile2(errfile, TRUE, pool);

#if CHECK_SVN_VERSION_G(1,11)
    if ((err = svn_client_diff7(NULL, path, &revision1, path, &revision2,
                                NULL, depth, !notice_ancestry, FALSE,
                                no_diff_deleted, show_copies_as_adds,
                                FALSE, FALSE, FALSE, FALSE, TRUE,
                                APR_LOCALE_CHARSET, outstream, errstream,
                                NULL, ctx, subpool)))
#else
    if ((err = svn_client_diff6(NULL, path, &revision1, path, &revision2,
                                NULL, depth, !notice_ancestry, FALSE,
                                no_diff_deleted, show_copies_as_adds,
                                FALSE, FALSE, FALSE, FALSE, APR_LOCALE_CHARSET,
                                outstream, errstream, NULL, ctx, subpool)))
#endif
#elif CHECK_SVN_VERSION_G(1,7)
    if ((err = svn_client_diff5(NULL, path, &revision1, path, &revision2,
                                NULL, depth, !notice_ancestry, no_diff_deleted,
                                show_copies_as_adds, FALSE, FALSE, APR_LOCALE_CHARSET,
                                outfile, errfile, NULL, ctx, subpool)))
#else
    if ((err = svn_client_diff4(NULL, path, &revision1, path, &revision2,
                                NULL, depth, !notice_ancestry,
                                no_diff_deleted, FALSE, APR_LOCALE_CHARSET,
                                outfile, errfile, NULL, ctx, subpool)))
#endif
    {
      goto on_error;
    }

    /* XXX Slurps the entire diff into memory. Is there a way to
     * make GTK display a file directly? */
    err = svn_io_file_flush_to_disk(outfile, subpool);
    if (err)
      goto on_error;
    pos = 0;
    err = svn_io_file_seek(outfile, APR_SET, &pos, subpool);
    if (err)
      goto on_error;

    stream = svn_stream_from_aprfile2(outfile, FALSE, subpool);
    iterpool = svn_pool_create(subpool);
    for (;;)
    {
      svn_stringbuf_t *buf;
      svn_boolean_t eof;

      svn_pool_clear(iterpool);

      err = svn_stream_readline(stream, &buf, APR_EOL_STR, &eof, iterpool);
      if (err)
        goto on_error;

      if (eof)
        break;

      svn_stringbuf_appendcstr(buf, APR_EOL_STR);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      gdk_threads_enter();
      tsh_diff_dialog_add(dialog, buf->data, buf->len);
      gdk_threads_leave();
G_GNUC_END_IGNORE_DEPRECATIONS
    }
    svn_pool_destroy(iterpool);
    err = svn_stream_close(stream);
    if (err)
      goto on_error;
  }
  svn_pool_destroy (subpool);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  gdk_threads_enter();
  tsh_diff_dialog_done (dialog);
  gdk_threads_leave();
G_GNUC_END_IGNORE_DEPRECATIONS

  tsh_reset_cancel();
  return GINT_TO_POINTER (TRUE);

on_error:
  svn_pool_destroy (subpool);
  
  if (err->apr_err != SVN_ERR_CANCELLED)
  {
    gchar *error_str;

    error_str = tsh_strerror(err);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_enter();
G_GNUC_END_IGNORE_DEPRECATIONS

    error = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Diff failed"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error), "%s", error_str);
    tsh_dialog_start(GTK_DIALOG(error), FALSE);
    tsh_diff_dialog_done (dialog);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_leave();
G_GNUC_END_IGNORE_DEPRECATIONS

    g_free(error_str);
  }
  
  svn_error_clear(err);
  tsh_reset_cancel();
  return GINT_TO_POINTER (FALSE);
}

static void create_diff_thread(TshDiffDialog *dialog, struct thread_args *args)
{
	GThread *thread = g_thread_new (NULL, diff_thread, args);
  if (thread)
    tsh_replace_thread(thread);
  else
    tsh_diff_dialog_done(dialog);
}


GThread *tsh_diff (gchar **files, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
  GtkWidget *dialog;
  struct thread_args *args;

  dialog = tsh_diff_dialog_new (_("Diff"), NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tsh_cancel, NULL);
  tsh_dialog_start (GTK_DIALOG (dialog), TRUE);

  args = g_malloc (sizeof (struct thread_args));
  args->ctx = ctx;
  args->pool = pool;
  args->dialog = TSH_DIFF_DIALOG (dialog);
  args->files = files;

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_diff_thread), args);

  return g_thread_new (NULL, diff_thread, args);
}

