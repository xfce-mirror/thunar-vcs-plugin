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

#include <libxfce4util/libxfce4util.h>

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-tree-common.h"
#include "tsh-file-selection-dialog.h"

#if CHECK_SVN_VERSION(1,5)
static void tsh_file_selection_status_func2 (void *, const char *, svn_wc_status2_t *);
#elif CHECK_SVN_VERSION(1,6)
static svn_error_t *tsh_file_selection_status_func3 (void *, const char *, svn_wc_status2_t *, apr_pool_t *);
#else /* CHECK_SVN_VERSION(1,7) */
static svn_error_t *tsh_file_selection_status_func (void *, const char *, const svn_client_status_t *, apr_pool_t *);
#endif
static void selection_cell_toggled (GtkCellRendererToggle *, gchar *, gpointer);
static void selection_all_toggled (GtkToggleButton *, gpointer);

struct select_context { TshFileSelectionDialog *dialog; gboolean select; };
struct copy_context { union { gchar **string; GSList *linked; guint count; } list; TshFileStatus status; gboolean indirect; };
static gboolean count_selected (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static gboolean copy_selected_string (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static gboolean copy_selected_linked (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static gboolean set_selected (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static void move_info (GtkTreeStore*, GtkTreeIter*, GtkTreeIter*);

static void add_unversioned (GtkTreeStore*, const gchar*, gboolean, gboolean);
static void set_children_status_unversioned (GtkTreeStore*, GtkTreeIter*, gboolean, gboolean);
static void set_children_status (TshFileSelectionDialog *, GtkTreeStore*, GtkTreeIter*, gboolean, gboolean);

struct _TshFileSelectionDialog
{
	GtkDialog dialog;

	GtkWidget *tree_view;
  GtkWidget *all;

  TshFileSelectionFlags flags;
};

struct _TshFileSelectionDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshFileSelectionDialog, tsh_file_selection_dialog, GTK_TYPE_DIALOG)

static void
tsh_file_selection_dialog_class_init (TshFileSelectionDialogClass *klass)
{
}

enum {
  COLUMN_PATH = 0,
  COLUMN_NAME,
  COLUMN_TEXT_STAT,
  COLUMN_PROP_STAT,
  COLUMN_SELECTION,
  COLUMN_NON_RECURSIVE,
  COLUMN_ENABLED,
  COLUMN_STATUS,
  COLUMN_COUNT
};

static void
tsh_file_selection_dialog_init (TshFileSelectionDialog *dialog)
{
	GtkWidget *tree_view;
	GtkWidget *scroll_window;
	GtkWidget *all;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
  gint n_columns;

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	dialog->tree_view = tree_view = gtk_tree_view_new ();

	renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled", G_CALLBACK (selection_cell_toggled), dialog);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, "", renderer,
                                               "active", COLUMN_SELECTION,
                                               "inconsistent", COLUMN_NON_RECURSIVE,
                                               "activatable", COLUMN_ENABLED,
                                               NULL);

	renderer = gtk_cell_renderer_text_new ();
  n_columns = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                                           -1, _("Path"), renderer,
                                                           "text", COLUMN_NAME,
                                                           NULL);
  gtk_tree_view_set_expander_column (GTK_TREE_VIEW (tree_view), gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), n_columns - 1));
  
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("State"), renderer,
                                               "text", COLUMN_TEXT_STAT,
                                               NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
	                                             -1, ("Prop state"), renderer,
                                               "text", COLUMN_PROP_STAT,
                                               NULL);

  model = GTK_TREE_MODEL (gtk_tree_store_new (COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_INT));

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (scroll_window), tree_view);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (tree_view);
	gtk_widget_show (scroll_window);

	dialog->all = all = gtk_check_button_new_with_label (_("Select/Unselect all"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (all), TRUE);
  gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (all), TRUE);
  g_signal_connect (all, "toggled", G_CALLBACK (selection_all_toggled), dialog);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), all, FALSE, FALSE, 0);
	gtk_widget_show (all);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Status"));

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
	                        _("_Cancel"), GTK_RESPONSE_CANCEL,
	                        _("_OK"), GTK_RESPONSE_OK,
	                        NULL);
	gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
}

GtkWidget*
tsh_file_selection_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, gchar **files, TshFileSelectionFlags selection_flags, svn_client_ctx_t *ctx, apr_pool_t *pool)
{
  svn_opt_revision_t revision;
  svn_error_t *err;
  apr_pool_t *subpool;

  TshFileSelectionDialog *dialog = g_object_new (TSH_TYPE_FILE_SELECTION_DIALOG, NULL);

  if(title)
    gtk_window_set_title (GTK_WINDOW(dialog), title);

  if(parent)
    gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

  if(flags & GTK_DIALOG_MODAL)
    gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

  if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
    gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

  dialog->flags = selection_flags;

  subpool = svn_pool_create (pool);

  revision.kind = svn_opt_revision_head;
  if(files)
  {
    while (*files)
    {
      svn_pool_clear(subpool);

#if CHECK_SVN_VERSION_G(1,9)
      if((err = svn_client_status6(NULL, ctx, *files, &revision,
                                   (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                     svn_depth_infinity : svn_depth_immediates,
                                   selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                   FALSE, TRUE,
                                   selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                   TRUE, TRUE, NULL,
                                   tsh_file_selection_status_func, dialog, subpool)))
#elif CHECK_SVN_VERSION_G(1,7)
      if((err = svn_client_status5(NULL, ctx, *files, &revision,
                                   (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                     svn_depth_infinity : svn_depth_immediates,
                                   selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                   FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                   TRUE, TRUE, NULL,
                                   tsh_file_selection_status_func, dialog, subpool)))
#elif CHECK_SVN_VERSION_G(1,6)
      if((err = svn_client_status4(NULL, *files, &revision,
                                   tsh_file_selection_status_func3, dialog,
                                   (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                     svn_depth_infinity : svn_depth_immediates,
                                   selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                   FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                   TRUE, NULL, ctx, subpool)))
#else
      if((err = svn_client_status3(NULL, *files, &revision,
                                   tsh_file_selection_status_func2, dialog,
                                   (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                     svn_depth_infinity : svn_depth_immediates,
                                   selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                   FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                   TRUE, NULL, ctx, subpool)))
#endif
      {
	svn_pool_destroy (subpool);

	g_object_unref(GTK_WIDGET(dialog));

	svn_error_clear(err);
	return NULL;  //FIXME: needed ??
      }
      files++;
    }
  }
  else
  {

#if CHECK_SVN_VERSION_G(1,9)
  if((err = svn_client_status6(NULL, ctx, "", &revision,
                               (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                 svn_depth_infinity : svn_depth_immediates,
                               selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                               FALSE, TRUE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                               TRUE, TRUE, NULL, tsh_file_selection_status_func,
                               dialog, subpool)))
#elif CHECK_SVN_VERSION_G(1,7)
  if((err = svn_client_status5(NULL, ctx, "", &revision,
                               (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                 svn_depth_infinity : svn_depth_immediates,
                               selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                               FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                               TRUE, TRUE, NULL, tsh_file_selection_status_func,
                               dialog, subpool)))
#elif CHECK_SVN_VERSION(1,6)
    if((err = svn_client_status4(NULL, "", &revision,
                                 tsh_file_selection_status_func3, dialog,
                                 (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                   svn_depth_infinity : svn_depth_immediates,
                                 selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                 FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                 TRUE, NULL, ctx, subpool)))
#else
    if((err = svn_client_status3(NULL, "", &revision, 
                                 tsh_file_selection_status_func2, dialog,
                                 (selection_flags & TSH_FILE_SELECTION_FLAG_RECURSIVE) ?
                                   svn_depth_infinity : svn_depth_immediates,
                                 selection_flags & TSH_FILE_SELECTION_FLAG_UNCHANGED,
                                 FALSE, selection_flags & TSH_FILE_SELECTION_FLAG_IGNORED,
                                 TRUE, NULL, ctx, subpool)))
#endif
    {
      svn_pool_destroy (subpool);

      g_object_unref(GTK_WIDGET(dialog));

      svn_error_clear(err);
      return NULL;
    }
  }

  svn_pool_destroy (subpool);

  gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tree_view));

  return GTK_WIDGET(dialog);
}

gchar**
tsh_file_selection_dialog_get_files (TshFileSelectionDialog *dialog)
{
  return tsh_file_selection_dialog_get_files_by_status (dialog, TSH_FILE_STATUS_INVALID, FALSE);
}

gchar**
tsh_file_selection_dialog_get_files_by_status (TshFileSelectionDialog *dialog, TshFileStatus status, gboolean indirect)
{
  GtkTreeModel *model;
  gchar **files;
  struct copy_context ctx;

  g_return_val_if_fail (TSH_IS_FILE_SELECTION_DIALOG (dialog), NULL);

  ctx.status = status;
  ctx.indirect = indirect;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  ctx.list.count = 0;
  gtk_tree_model_foreach (model, count_selected, &ctx);

  if (!ctx.list.count)
    return NULL;

  ctx.list.string = files = g_new(gchar *, ctx.list.count+1);

  gtk_tree_model_foreach (model, copy_selected_string, &ctx);

  *ctx.list.string = NULL;

  return files;
}

GSList*
tsh_file_selection_dialog_get_file_info (TshFileSelectionDialog *dialog)
{
  return tsh_file_selection_dialog_get_file_info_by_status (dialog, TSH_FILE_STATUS_INVALID, FALSE);
}

GSList*
tsh_file_selection_dialog_get_file_info_by_status (TshFileSelectionDialog *dialog, TshFileStatus status, gboolean indirect)
{
  GtkTreeModel *model;
  struct copy_context ctx;

  g_return_val_if_fail (TSH_IS_FILE_SELECTION_DIALOG (dialog), NULL);

  ctx.status = status;
  ctx.indirect = indirect;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  ctx.list.linked = NULL;

  gtk_tree_model_foreach (model, copy_selected_linked, &ctx);

  return g_slist_reverse (ctx.list.linked);
}

static TshFileStatus
get_parent_status(GtkTreeModel *model, GtkTreeIter *iter)
{
  GtkTreeIter parent, child;
  TshFileStatus status = TSH_FILE_STATUS_INVALID;

  child = *iter;

  while (gtk_tree_model_iter_parent (model, &parent, &child))
  {
    gint parent_status;
    gtk_tree_model_get (model, &parent, COLUMN_STATUS, &parent_status, -1);

    if (parent_status != TSH_FILE_STATUS_INVALID)
    {
      status = parent_status;
      break;
    }

    child = parent;
  }

  return status;
}

#if CHECK_SVN_VERSION(1,5)
static void
tsh_file_selection_status_func2(void *baton, const char *path, svn_wc_status2_t *status)
#elif CHECK_SVN_VERSION(1,6)
static svn_error_t*
tsh_file_selection_status_func3(void *baton, const char *path, svn_wc_status2_t *status, apr_pool_t *pool)
#else /* CHECK_SVN_VERSION(1,7) */
static svn_error_t*
tsh_file_selection_status_func (void *baton, const char *path, const svn_client_status_t *status, apr_pool_t *pool)
#endif
{
  TshFileSelectionDialog *dialog = TSH_FILE_SELECTION_DIALOG (baton);
  gboolean add = FALSE;

#if CHECK_SVN_VERSION_S(1,6)
  if (status->entry)
#else /* CHECK_SVN_VERSION(1,7) */
  if (status->versioned)
#endif
  {
    if (dialog->flags & TSH_FILE_SELECTION_FLAG_CONFLICTED)
      if (status->text_status == svn_wc_status_conflicted || status->prop_status == svn_wc_status_conflicted)
        add = TRUE;

    if (dialog->flags & TSH_FILE_SELECTION_FLAG_UNCHANGED)
      if (status->text_status == svn_wc_status_normal && (status->prop_status == svn_wc_status_normal || status->prop_status == svn_wc_status_none))
        add = TRUE;

    if (dialog->flags & (TSH_FILE_SELECTION_FLAG_MODIFIED|TSH_FILE_SELECTION_FLAG_IGNORED))
      add = TRUE;

  }
  else if (dialog->flags & TSH_FILE_SELECTION_FLAG_UNVERSIONED)
    add = TRUE;

  if (add)
  {
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean select_ = TRUE;
    gboolean enable = TRUE;
    gint file_status = TSH_FILE_STATUS_OTHER;
    TshFileStatus parent_status;

#if CHECK_SVN_VERSION_S(1,6)
    if (G_LIKELY (status->entry))
#else /* CHECK_SVN_VERSION(1,7) */
    if (G_LIKELY (status->versioned))
#endif
    {
      if (status->text_status == svn_wc_status_added)
      {
        file_status = TSH_FILE_STATUS_ADDED;
      }
      else if (status->text_status == svn_wc_status_deleted)
      {
        file_status = TSH_FILE_STATUS_DELETED;
      }
      else if (status->text_status == svn_wc_status_missing)
      {
        file_status = TSH_FILE_STATUS_MISSING;
        if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
          enable = FALSE;
        else if (!(dialog->flags & TSH_FILE_SELECTION_FLAG_AUTO_SELECT_MISSING))
          select_ = FALSE;
      }
    }
    else
    {
      file_status = TSH_FILE_STATUS_UNVERSIONED;
      if (!(dialog->flags & TSH_FILE_SELECTION_FLAG_AUTO_SELECT_UNVERSIONED))
        select_ = FALSE;
    }

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

    tsh_tree_get_iter_for_path (GTK_TREE_STORE (model), path, &iter, COLUMN_NAME, move_info);

    parent_status = get_parent_status (model, &iter);

    if (status->text_status == svn_wc_status_normal && (status->prop_status == svn_wc_status_normal || status->prop_status == svn_wc_status_none))
    {
      file_status = TSH_FILE_STATUS_UNCHANGED;
      if (parent_status == TSH_FILE_STATUS_UNCHANGED)
        enable = FALSE;
    }
#if CHECK_SVN_VERSION_S(1,6)
    else if (status->entry && status->text_status != svn_wc_status_missing)
#else /* CHECK_SVN_VERSION(1,7) */
    else if (status->versioned && status->text_status != svn_wc_status_missing)
#endif
    {
      if (parent_status != TSH_FILE_STATUS_INVALID)
        enable = FALSE;
    }

    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        COLUMN_PATH, path,
                        COLUMN_TEXT_STAT, tsh_status_to_string(status->text_status),
                        COLUMN_PROP_STAT, tsh_status_to_string(status->prop_status),
                        COLUMN_SELECTION, select_,
                        COLUMN_NON_RECURSIVE, FALSE,
                        COLUMN_ENABLED, enable,
                        COLUMN_STATUS, file_status,
                        -1);

    switch (file_status)
    {
      case TSH_FILE_STATUS_UNCHANGED:
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, !select_);
        break;
      case TSH_FILE_STATUS_ADDED:
        if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
          set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, !select_);
        else
          set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, FALSE);
        break;
      case TSH_FILE_STATUS_DELETED:
        if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
          set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, FALSE);
        else
          set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, !select_);
        break;
      case TSH_FILE_STATUS_UNVERSIONED:
        /* Unversioned: get all children */
        add_unversioned (GTK_TREE_STORE (model), path, select_, FALSE);
        break;
      default:
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, select_, !select_);
        break;
    }
  }

#if CHECK_SVN_VERSION_G(1,6)
  return SVN_NO_ERROR;
#endif
}

static void
selection_cell_toggled (GtkCellRendererToggle *renderer, gchar *path, gpointer user_data)
{
  TshFileSelectionDialog *dialog = TSH_FILE_SELECTION_DIALOG (user_data);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean selection, non_recursive;
  gint status;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  gtk_tree_model_get_iter_from_string (model, &iter, path);

  gtk_tree_model_get (model, &iter,
                      COLUMN_SELECTION, &selection,
                      COLUMN_NON_RECURSIVE, &non_recursive,
                      COLUMN_STATUS, &status,
                      -1);
  switch (status)
  {
    case TSH_FILE_STATUS_DELETED:
      if (!(dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN))
      {
        selection = !selection;
        break;
      }
    default:
      if (gtk_tree_model_iter_has_child (model, &iter))
      {
#if 0   /* 4 states, not selected -> non recursive (no children selected) -> selected -> non recursive (children selected) */
        if (non_recursive)
        {
          non_recursive = FALSE;
          selection = !selection;
        }
        else
          non_recursive = TRUE;
#else   /* 3 states, not selected -> selected -> non recursive (children selected) */
        if (non_recursive)
        {
          non_recursive = FALSE;
          selection = FALSE;
        }
        else if (selection)
          non_recursive = TRUE;
        else
          selection = TRUE;
#endif
        break;
      }
    case TSH_FILE_STATUS_UNCHANGED:
      selection = !selection;
      break;
  }

  gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                      COLUMN_SELECTION, selection,
                      COLUMN_NON_RECURSIVE, non_recursive,
                      -1);
  switch (status)
  {
    case TSH_FILE_STATUS_UNCHANGED:
      set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, !selection);
      break;
    case TSH_FILE_STATUS_ADDED:
      if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, !selection);
      else
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, non_recursive);
      break;
    case TSH_FILE_STATUS_DELETED:
      if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, non_recursive);
      else
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, !selection);
      break;
    case TSH_FILE_STATUS_UNVERSIONED:
      set_children_status_unversioned (GTK_TREE_STORE (model), &iter, selection, non_recursive);
      break;
    default:
      if(selection)
        set_children_status (dialog, GTK_TREE_STORE (model), &iter, selection, non_recursive);
      break;
  }

  gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (dialog->all), TRUE);
}

static void
selection_all_toggled (GtkToggleButton *button, gpointer user_data)
{
  TshFileSelectionDialog *dialog = TSH_FILE_SELECTION_DIALOG (user_data);
  GtkTreeModel *model;
  struct select_context ctx;
  
  ctx.dialog = dialog;

  gtk_toggle_button_set_inconsistent (button, FALSE);

  ctx.select = gtk_toggle_button_get_active (button);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tree_view));

  gtk_tree_model_foreach (model, set_selected, &ctx);
}

static gboolean
count_selected (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer ctx)
{
  gboolean selection, enabled;
  TshFileStatus status;
  gtk_tree_model_get (model, iter,
                      COLUMN_SELECTION, &selection,
                      COLUMN_STATUS, &status,
                      COLUMN_ENABLED, &enabled,
                      -1);
  if (selection && (((struct copy_context*)ctx)->status == TSH_FILE_STATUS_INVALID || ((struct copy_context*)ctx)->status == status) && (enabled || ((struct copy_context*)ctx)->indirect))
    ((struct copy_context*)ctx)->list.count++;
  return FALSE;
}

static gboolean
copy_selected_string (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer ctx)
{
  gboolean selection, enabled;
  TshFileStatus status;
  gtk_tree_model_get (model, iter,
                      COLUMN_SELECTION, &selection,
                      COLUMN_STATUS, &status,
                      COLUMN_ENABLED, &enabled,
                      -1);
  if (selection && (((struct copy_context*)ctx)->status == TSH_FILE_STATUS_INVALID || ((struct copy_context*)ctx)->status == status) && (enabled || ((struct copy_context*)ctx)->indirect))
  {
    gtk_tree_model_get (model, iter,
                        COLUMN_PATH, ((struct copy_context*)ctx)->list.string,
                        -1);
    ((struct copy_context*)ctx)->list.string++;
  }
  return FALSE;
}

static gboolean
copy_selected_linked (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer ctx)
{
  gboolean selection, enabled;
  TshFileStatus status;
  gtk_tree_model_get (model, iter,
                      COLUMN_SELECTION, &selection,
                      COLUMN_STATUS, &status,
                      COLUMN_ENABLED, &enabled,
                      -1);
  if (selection && (((struct copy_context*)ctx)->status == TSH_FILE_STATUS_INVALID || ((struct copy_context*)ctx)->status == status) && (enabled || ((struct copy_context*)ctx)->indirect))
  {
    gboolean non_recursive;
    TshFileInfo *info = g_new (TshFileInfo, 1);
    gtk_tree_model_get (model, iter,
                        COLUMN_PATH, &info->path,
                        COLUMN_NON_RECURSIVE, &non_recursive,
                        -1);
    info->flags = (non_recursive?0:TSH_FILE_INFO_RECURSIVE) | (enabled?0:TSH_FILE_INFO_INDIRECT);
    info->status = status;
    ((struct copy_context*)ctx)->list.linked = g_slist_prepend (((struct copy_context*)ctx)->list.linked, info);
  }
  return FALSE;
}

static gboolean
set_selected (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer ctx)
{
  TshFileStatus parent_status;
  gint status;
  gboolean enabled = TRUE;
  gtk_tree_model_get (model, iter,
                      COLUMN_STATUS, &status,
                      -1);

  if (status != TSH_FILE_STATUS_INVALID)
  {
    parent_status = get_parent_status (model, iter);

    switch (parent_status)
    {
      case TSH_FILE_STATUS_UNCHANGED:
        enabled = !((struct select_context*)ctx)->select;
        break;
      case TSH_FILE_STATUS_ADDED:
        if (((struct select_context*)ctx)->dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
          enabled = !((struct select_context*)ctx)->select;
        else
          enabled = status == TSH_FILE_STATUS_UNVERSIONED && ((struct select_context*)ctx)->select;
        break;
      case TSH_FILE_STATUS_DELETED:
        enabled = !(((struct select_context*)ctx)->dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN) && !((struct select_context*)ctx)->select;
        break;
      case TSH_FILE_STATUS_UNVERSIONED:
        enabled = FALSE;
        break;
      case TSH_FILE_STATUS_OTHER:
        enabled = status == TSH_FILE_STATUS_UNVERSIONED || (status == TSH_FILE_STATUS_MISSING && !(((struct select_context*)ctx)->dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)) || !((struct select_context*)ctx)->select;
        break;
      case TSH_FILE_STATUS_MISSING:
      case TSH_FILE_STATUS_INVALID:
        break;
    }

    gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                        COLUMN_SELECTION, ((struct select_context*)ctx)->select,
                        COLUMN_NON_RECURSIVE, FALSE,
                        COLUMN_ENABLED, enabled,
                        -1);
  }

  return FALSE;
}

static void
move_info (GtkTreeStore *store, GtkTreeIter *dest, GtkTreeIter *src)
{
  gchar *path, *text, *prop;
  gboolean selected, non_recursive, enabled;
  gint status;

  gtk_tree_model_get (GTK_TREE_MODEL (store), src,
                      COLUMN_PATH, &path,
                      COLUMN_TEXT_STAT, &text,
                      COLUMN_PROP_STAT, &prop,
                      COLUMN_SELECTION, &selected,
                      COLUMN_NON_RECURSIVE, &non_recursive,
                      COLUMN_ENABLED, &enabled,
                      COLUMN_STATUS, &status,
                      -1);

  gtk_tree_store_set (store, dest,
                      COLUMN_PATH, path,
                      COLUMN_TEXT_STAT, text,
                      COLUMN_PROP_STAT, prop,
                      COLUMN_SELECTION, selected,
                      COLUMN_NON_RECURSIVE, non_recursive,
                      COLUMN_ENABLED, enabled,
                      COLUMN_STATUS, status,
                      -1);

  g_free (path);
  g_free (text);
  g_free (prop);
}

static void
add_unversioned (GtkTreeStore *model, const gchar *path, gboolean select_, gboolean enabled)
{
  GDir *dir = g_dir_open (path, 0, NULL);
  if (dir)
  {
    const gchar *file;
    while ((file = g_dir_read_name (dir)))
    {
      GtkTreeIter iter;
      gchar *file_path = g_build_filename (path, file, NULL);
      tsh_tree_get_iter_for_path (model, file_path, &iter, COLUMN_NAME, move_info);
      gtk_tree_store_set (model, &iter,
                          COLUMN_PATH, file_path,
                          COLUMN_TEXT_STAT, "",
                          COLUMN_PROP_STAT, "",
                          COLUMN_SELECTION, select_,
                          COLUMN_NON_RECURSIVE, FALSE,
                          COLUMN_ENABLED, enabled,
                          COLUMN_STATUS, TSH_FILE_STATUS_UNVERSIONED,
                          -1);
      add_unversioned (model, file_path, select_, FALSE);
      g_free (file_path);
    }
    g_dir_close (dir);
  }
}

static void
set_children_status (TshFileSelectionDialog *dialog, GtkTreeStore *model, GtkTreeIter *parent, gboolean select_, gboolean enabled)
{
  gint parent_status;
  GtkTreeIter iter;

  gtk_tree_model_get (GTK_TREE_MODEL (model), parent,
                      COLUMN_STATUS, &parent_status,
                      -1);

  if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, parent))
    do
    {
      gboolean selection;
      gboolean non_recursive;
      gboolean enable;
      gint status;
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                          COLUMN_SELECTION, &selection,
                          COLUMN_NON_RECURSIVE, &non_recursive,
                          COLUMN_STATUS, &status,
                          -1);
      switch (parent_status)
      {
        case TSH_FILE_STATUS_ADDED:
          switch (status)
          {
            case TSH_FILE_STATUS_UNVERSIONED:
              enable = select_;
              if (!select_)
                selection = FALSE;
              break;
            default:
              enable = enabled;
              selection = select_;
              break;
          }
          break;
        default:
          enable = enabled;
          switch (status)
          {
            case TSH_FILE_STATUS_UNVERSIONED:
              if (select_)
                continue;
              break;
            case TSH_FILE_STATUS_MISSING:
              if (select_ && !(dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN))
                continue;
              break;
            default:
              selection = select_;
          }
          break;
      }
      if (!enable)
        non_recursive = FALSE;
      gtk_tree_store_set (model, &iter,
                          COLUMN_SELECTION, selection,
                          COLUMN_NON_RECURSIVE, non_recursive,
                          COLUMN_ENABLED, enable,
                          -1);
      switch (status)
      {
        case TSH_FILE_STATUS_UNVERSIONED:
          if (!non_recursive)
            set_children_status_unversioned (model, &iter, selection, FALSE);
          break;
        case TSH_FILE_STATUS_DELETED:
          if (dialog->flags & TSH_FILE_SELECTION_FLAG_REVERSE_DISABLE_CHILDREN)
          {
            set_children_status (dialog, model, &iter, selection, non_recursive && selection);
            break;
          }
        case TSH_FILE_STATUS_UNCHANGED:
          set_children_status (dialog, model, &iter, selection, non_recursive || !selection);
          break;
        default:
          set_children_status (dialog, model, &iter, selection, non_recursive);
          break;
      }
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
}

static void
set_children_status_unversioned (GtkTreeStore *model, GtkTreeIter *parent, gboolean select_, gboolean enabled)
{
  GtkTreeIter iter;
  if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, parent))
    do
    {
      gtk_tree_store_set (model, &iter,
                          COLUMN_SELECTION, select_,
                          COLUMN_NON_RECURSIVE, FALSE,
                          COLUMN_ENABLED, enabled,
                          -1);
      set_children_status_unversioned (model, &iter, select_, FALSE);
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
}

