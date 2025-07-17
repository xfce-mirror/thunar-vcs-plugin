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

#include "tsh-tree-common.h"

static gint
path_compare (const gchar *path1, const gchar *path2)
{
  const gchar *i1, *i2;
  gchar c1, c2;
  gint depth = 0;
  i1 = path1;
  i2 = path2;
  while ((c1 = *i1++))
  {
    c2 = *i2++;

    if (c1 != c2)
    {
      if (c1 == '/'&& !c2)
        depth++;
      else if (!depth)
        depth = -1;

      return depth;
    }

    if (c1 == '/')
      depth++;
  }

  switch (*i2)
  {
    case '/':
      depth++;
      break;
    case '\0':
      return 0;
  }

  if (!depth)
    depth = -1;

  return depth;
}

static gint
path_depth (const gchar *path)
{
  const gchar *i;
  gchar c, last_c = '\0';
  gint depth = 0;
  i = path;
  while ((c = *i++))
  {
    if (c == '/')
      depth++;
    last_c = c;
  }
  if (last_c != '/')
    depth++;
  return depth;
}

static gchar*
path_get_prefix (const gchar *path, gint depth)
{
  const gchar *i;
  gchar c;
  gint lng = 0;
  i = path;
  while ((c = *i++))
  {
    if (c == '/')
      if (!--depth)
        break;
    lng++;
  }
  if (lng == 0)
    return g_strdup("/");
  return g_strndup (path, lng);
}

static const gchar*
path_remove_prefix (const gchar *path, gint depth)
{
  const gchar *i;
  gchar c;
  i = path;
  while ((c = *i++))
  {
    if (c == '/')
      if (!--depth)
        return i;
  }
  return NULL;
}

static void
move_children (GtkTreeStore *model, GtkTreeIter *dest, GtkTreeIter *src, gint path_column, TshTreeMoveInfoFunc move_info)
{
  gchar *path;
  GtkTreeIter iter_src;
  GtkTreeIter iter_dst;

  if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_src, src))
  {
    do
    {
      gtk_tree_store_append (model, &iter_dst, dest);
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter_src, path_column, &path, -1);
      move_info (model, &iter_dst, &iter_src);
      gtk_tree_store_set (model, &iter_dst, path_column, path, -1);
      g_free (path);

      move_children (model, &iter_dst, &iter_src, path_column, move_info);
    }
    while (gtk_tree_store_remove (model, &iter_src));
  }
}

void
tsh_tree_get_iter_for_path (GtkTreeStore *model, const gchar *file, GtkTreeIter *iter, gint path_column, TshTreeMoveInfoFunc move_info)
{
  const gchar *relative_file = file;
  GtkTreeIter real_parent, *parent = NULL;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), iter))
  {
    gint file_depth = path_depth (relative_file);
    for (;;)
    {
      gchar *path;
      gint depth;
      gtk_tree_model_get (GTK_TREE_MODEL (model), iter, path_column, &path, -1);

      depth = path_compare (path, relative_file);
      if(!depth)
      {
        g_free (path);
        return;
      }
      if (depth > 0)
      {
        gint depth_p = path_depth (path);

        if (depth_p == depth)
        {
          /* path is the parent */
          real_parent = *iter;
          parent = &real_parent;

          relative_file = path_remove_prefix (relative_file, depth);

          if (!gtk_tree_model_iter_children (GTK_TREE_MODEL (model), iter, parent))
          {
            g_free (path);
            break;
          }

          g_free(path);
          continue;
        }
        else if (depth == file_depth)
        {
          /* file is the parent */
          GtkTreeIter new;
          /* Add a new parent to the model, on iter removal iter will become this new iter */
          gtk_tree_store_insert_after (model, &real_parent, parent, iter);
          /* Add a child to copy the old iter's data to */
          gtk_tree_store_append (model, &new, &real_parent);
          /* Copy the data */
          move_info (model, &new, iter);
          /* Set the path names */
          gtk_tree_store_set (model, &real_parent, path_column, relative_file, -1);
          gtk_tree_store_set (model, &new, path_column, path_remove_prefix(path, depth), -1);
          /* Move the children */
          move_children (model, &new, iter, path_column, move_info);
          /* Iter becomes the next child, which is our new iter */
          gtk_tree_store_remove (model, iter);

          g_free (path);
          return;
        }
        else
        {
          /* common prefix */
          GtkTreeIter new;
          gchar *prefix = path_get_prefix (relative_file, depth);

          /* Add a new parent to the model */
          gtk_tree_store_insert_after (model, &real_parent, parent, iter);
          parent = &real_parent;
          /* Add a child to copy the old iter's data to */
          gtk_tree_store_append(model, &new, parent);
          /* Copy the data */
          move_info (model, &new, iter);
          /* Set the path names */
          gtk_tree_store_set (model, parent, path_column, prefix, -1);
          gtk_tree_store_set (model, &new, path_column, path_remove_prefix(path, depth), -1);
          /* Move the children */
          move_children (model, &new, iter, path_column, move_info);
          /* Remove the old iter */
          gtk_tree_store_remove (model, iter);

          relative_file = path_remove_prefix (relative_file, depth);

          g_free (prefix);
          g_free (path);
          break;
        }
      }

      g_free (path);
      if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (model), iter))
        break;
    }
  }

  if (relative_file)
  {
    gtk_tree_store_append (model, iter, parent);
    gtk_tree_store_set (model, iter, path_column, relative_file, -1);
  }
}

