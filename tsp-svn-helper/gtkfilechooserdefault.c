/* -*- Mode: C; c-file-style: "gnu"; tab-width: 8 -*- */
/* GTK - The GIMP Toolkit
 * gtkfilechooserdefault.c: Default implementation of GtkFileChooser
 * Copyright (C) 2003, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <gtk/gtk.h>

#include <string.h>

#include "gtkintl.h"

/* FIXME: GtkFileSystem needs a function to split a remote path
 * into hostname and path components, or maybe just have a 
 * gtk_file_system_path_get_display_name().
 *
 * This function is also used in gtkfilechooserbutton.c
 */
gchar *
_gtk_file_chooser_label_for_uri (const gchar *uri)
{
  const gchar *path, *start, *end, *p;
  gchar *host, *label;
  
  start = strstr (uri, "://");
  start += 3;
  path = strchr (start, '/');
  
  if (path)
    end = path;
  else
    {
      end = uri + strlen (uri);
      path = "/";
    }

  /* strip username */
  p = strchr (start, '@');
  if (p && p < end)
    {
      start = p + 1;
    }
  
  p = strchr (start, ':');
  if (p && p < end)
    end = p;
  
  host = g_strndup (start, end - start);

  /* Translators: the first string is a path and the second string 
   * is a hostname. Nautilus and the panel contain the same string 
   * to translate. 
   */
  label = g_strdup_printf (_("%1$s on %2$s"), path, host);
  
  g_free (host);

  return label;
}
