/*-
 * Copyright (c) 2006 Peter de Ridder <peter@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __TSH_FILE_SELECTION_DIALOG_H__
#define __TSH_FILE_SELECTION_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef enum {
  TSH_FILE_SELECTION_FLAG_RECURSIVE   = 1<<0,
  TSH_FILE_SELECTION_FLAG_MODIFIED    = 1<<1,
  TSH_FILE_SELECTION_FLAG_UNVERSIONED = 1<<2,
  TSH_FILE_SELECTION_FLAG_UNCHANGED   = 1<<3,
  TSH_FILE_SELECTION_FLAG_IGNORED     = 1<<4,
  TSH_FILE_SELECTION_FLAG_CONFLICTED  = 1<<5,

  TSH_FILE_SELECTION_FLAG_AUTO_SELECT_UNVERSIONED = 1<<6,
  TSH_FILE_SELECTION_FLAG_AUTO_SELECT_MISSING     = 1<<7,

  TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN = 1<<8
} TshFileSelectionFlags;

typedef enum {
  TSH_FILE_STATUS_INVALID = 0,
  TSH_FILE_STATUS_UNCHANGED,
  TSH_FILE_STATUS_ADDED,
  TSH_FILE_STATUS_DELETED,
  TSH_FILE_STATUS_MISSING,
  TSH_FILE_STATUS_UNVERSIONED,
  TSH_FILE_STATUS_OTHER
} TshFileStatus;

typedef enum {
  TSH_FILE_INFO_RECURSIVE   = 1<<0,
  TSH_FILE_INFO_INDIRECT    = 1<<1
} TshFileInfoFlags;

typedef struct {
  gchar *path;
  TshFileInfoFlags flags;
  TshFileStatus status;
} TshFileInfo;

typedef struct _TshFileSelectionDialogClass TshFileSelectionDialogClass;
typedef struct _TshFileSelectionDialog      TshFileSelectionDialog;

#define TSH_TYPE_FILE_SELECTION_DIALOG            (tsh_file_selection_dialog_get_type ())
#define TSH_FILE_SELECTION_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_FILE_SELECTION_DIALOG, TshFileSelectionDialog))
#define TSH_FILE_SELECTION_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_FILE_SELECTION_DIALOG, TshFileSelectionDialogClass))
#define TSH_IS_FILE_SELECTION_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_FILE_SELECTION_DIALOG))
#define TSH_IS_FILE_SELECTION_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_FILE_SELECTION_DIALOG))
#define TSH_FILE_SELECTION_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_FILE_SELECTION_DIALOG, TshFileSelectionDialogClass))

GType      tsh_file_selection_dialog_get_type   (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_file_selection_dialog_new        (const gchar *title,
                                                 GtkWindow *parent,
                                                 GtkDialogFlags flags,
                                                 gchar **files,
                                                 TshFileSelectionFlags selection_flags,
                                                 svn_client_ctx_t *ctx,
                                                 apr_pool_t *pool) G_GNUC_MALLOC G_GNUC_INTERNAL;

gchar**    tsh_file_selection_dialog_get_files              (TshFileSelectionDialog *dialog) G_GNUC_WARN_UNUSED_RESULT;
gchar**    tsh_file_selection_dialog_get_files_by_status    (TshFileSelectionDialog *dialog, TshFileStatus status, gboolean indirect) G_GNUC_WARN_UNUSED_RESULT;

GSList*    tsh_file_selection_dialog_get_file_info              (TshFileSelectionDialog *dialog) G_GNUC_WARN_UNUSED_RESULT;
GSList*    tsh_file_selection_dialog_get_file_info_by_status    (TshFileSelectionDialog *dialog, TshFileStatus status, gboolean indirect) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__TSH_FILE_SELECTION_DIALOG_H__ */
