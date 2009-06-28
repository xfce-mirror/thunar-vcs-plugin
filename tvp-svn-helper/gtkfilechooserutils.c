/* GTK - The GIMP Toolkit
 * gtkfilechooserutils.c: Private utility functions useful for
 *                        implementing a GtkFileChooser interface
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
#define GTK_FILE_SYSTEM_ENABLE_UNSUPPORTED
#define GTK_FILE_CHOOSER_ENABLE_UNSUPPORTED
#include "gtkfilechooserutils.h"
#undef GTK_FILE_CHOOSER_ENABLE_UNSUPPORTED
#undef GTK_FILE_SYSTEM_ENABLE_UNSUPPORTED

static gboolean       delegate_set_current_folder     (GtkFileChooser    *chooser,
						       const GtkFilePath *path,
						       GError           **error);
static GtkFilePath *  delegate_get_current_folder     (GtkFileChooser    *chooser);
static void           delegate_set_current_name       (GtkFileChooser    *chooser,
						       const gchar       *name);
static gboolean       delegate_select_path            (GtkFileChooser    *chooser,
						       const GtkFilePath *path,
						       GError           **error);
static void           delegate_unselect_path          (GtkFileChooser    *chooser,
						       const GtkFilePath *path);
static void           delegate_select_all             (GtkFileChooser    *chooser);
static void           delegate_unselect_all           (GtkFileChooser    *chooser);
static GSList *       delegate_get_paths              (GtkFileChooser    *chooser);
static GtkFilePath *  delegate_get_preview_path       (GtkFileChooser    *chooser);
static GtkFileSystem *delegate_get_file_system        (GtkFileChooser    *chooser);
static void           delegate_add_filter             (GtkFileChooser    *chooser,
						       GtkFileFilter     *filter);
static void           delegate_remove_filter          (GtkFileChooser    *chooser,
						       GtkFileFilter     *filter);
static GSList *       delegate_list_filters           (GtkFileChooser    *chooser);
static gboolean       delegate_add_shortcut_folder    (GtkFileChooser    *chooser,
						       const GtkFilePath *path,
						       GError           **error);
static gboolean       delegate_remove_shortcut_folder (GtkFileChooser    *chooser,
						       const GtkFilePath *path,
						       GError           **error);
static GSList *       delegate_list_shortcut_folders  (GtkFileChooser    *chooser);

/**
 * _gtk_file_chooser_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 *
 * Installs the necessary properties for a class implementing
 * #GtkFileChooser. A #GtkParamSpecOverride property is installed
 * for each property, using the values from the #GtkFileChooserProp
 * enumeration. The caller must make sure itself that the enumeration
 * values don't collide with some other property values they
 * are using.
 **/
void
_gtk_file_chooser_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_ACTION,
				    "action");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_EXTRA_WIDGET,
				    "extra-widget");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_FILE_SYSTEM_BACKEND,
				    "file-system-backend");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_FILTER,
				    "filter");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_LOCAL_ONLY,
				    "local-only");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_PREVIEW_WIDGET,
				    "preview-widget");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_PREVIEW_WIDGET_ACTIVE,
				    "preview-widget-active");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_USE_PREVIEW_LABEL,
				    "use-preview-label");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_SELECT_MULTIPLE,
				    "select-multiple");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_SHOW_HIDDEN,
				    "show-hidden");
  g_object_class_override_property (klass,
				    GTK_FILE_CHOOSER_PROP_DO_OVERWRITE_CONFIRMATION,
				    "do-overwrite-confirmation");
}

/**
 * _gtk_file_chooser_delegate_iface_init:
 * @iface: a #GtkFileChoserIface structure
 *
 * An interface-initialization function for use in cases where
 * an object is simply delegating the methods, signals of
 * the #GtkFileChooser interface to another object.
 * _gtk_file_chooser_set_delegate() must be called on each
 * instance of the object so that the delegate object can
 * be found.
 **/
void
_gtk_file_chooser_delegate_iface_init (GtkFileChooserIface *iface)
{
  iface->set_current_folder = delegate_set_current_folder;
  iface->get_current_folder = delegate_get_current_folder;
  iface->set_current_name = delegate_set_current_name;
  iface->select_path = delegate_select_path;
  iface->unselect_path = delegate_unselect_path;
  iface->select_all = delegate_select_all;
  iface->unselect_all = delegate_unselect_all;
  iface->get_paths = delegate_get_paths;
  iface->get_preview_path = delegate_get_preview_path;
  iface->get_file_system = delegate_get_file_system;
  iface->add_filter = delegate_add_filter;
  iface->remove_filter = delegate_remove_filter;
  iface->list_filters = delegate_list_filters;
  iface->add_shortcut_folder = delegate_add_shortcut_folder;
  iface->remove_shortcut_folder = delegate_remove_shortcut_folder;
  iface->list_shortcut_folders = delegate_list_shortcut_folders;
}

GQuark
_gtk_file_chooser_delegate_get_quark (void)
{
  static GQuark quark = 0;

  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("gtk-file-chooser-delegate");
  
  return quark;
}

static GtkFileChooser *
get_delegate (GtkFileChooser *receiver)
{
  return g_object_get_qdata (G_OBJECT (receiver),
			     GTK_FILE_CHOOSER_DELEGATE_QUARK);
}

static gboolean
delegate_select_path (GtkFileChooser    *chooser,
		      const GtkFilePath *path,
		      GError           **error)
{
  return _gtk_file_chooser_select_path (get_delegate (chooser), path, error);
}

static void
delegate_unselect_path (GtkFileChooser    *chooser,
			const GtkFilePath *path)
{
  _gtk_file_chooser_unselect_path (get_delegate (chooser), path);
}

static void
delegate_select_all (GtkFileChooser *chooser)
{
  gtk_file_chooser_select_all (get_delegate (chooser));
}

static void
delegate_unselect_all (GtkFileChooser *chooser)
{
  gtk_file_chooser_unselect_all (get_delegate (chooser));
}

static GSList *
delegate_get_paths (GtkFileChooser *chooser)
{
  return _gtk_file_chooser_get_paths (get_delegate (chooser));
}

static GtkFilePath *
delegate_get_preview_path (GtkFileChooser *chooser)
{
  return _gtk_file_chooser_get_preview_path (get_delegate (chooser));
}

static GtkFileSystem *
delegate_get_file_system (GtkFileChooser *chooser)
{
  return _gtk_file_chooser_get_file_system (get_delegate (chooser));
}

static void
delegate_add_filter (GtkFileChooser *chooser,
		     GtkFileFilter  *filter)
{
  gtk_file_chooser_add_filter (get_delegate (chooser), filter);
}

static void
delegate_remove_filter (GtkFileChooser *chooser,
			GtkFileFilter  *filter)
{
  gtk_file_chooser_remove_filter (get_delegate (chooser), filter);
}

static GSList *
delegate_list_filters (GtkFileChooser *chooser)
{
  return gtk_file_chooser_list_filters (get_delegate (chooser));
}

static gboolean
delegate_add_shortcut_folder (GtkFileChooser    *chooser,
			      const GtkFilePath *path,
			      GError           **error)
{
  return _gtk_file_chooser_add_shortcut_folder (get_delegate (chooser), path, error);
}

static gboolean
delegate_remove_shortcut_folder (GtkFileChooser    *chooser,
				 const GtkFilePath *path,
				 GError           **error)
{
  return _gtk_file_chooser_remove_shortcut_folder (get_delegate (chooser), path, error);
}

static GSList *
delegate_list_shortcut_folders (GtkFileChooser *chooser)
{
  return gtk_file_chooser_list_shortcut_folders (get_delegate (chooser));
}

static gboolean
delegate_set_current_folder (GtkFileChooser    *chooser,
			     const GtkFilePath *path,
			     GError           **error)
{
  return _gtk_file_chooser_set_current_folder_path (get_delegate (chooser), path, error);
}

static GtkFilePath *
delegate_get_current_folder (GtkFileChooser *chooser)
{
  return _gtk_file_chooser_get_current_folder_path (get_delegate (chooser));
}

static void
delegate_set_current_name (GtkFileChooser *chooser,
			   const gchar    *name)
{
  gtk_file_chooser_set_current_name (get_delegate (chooser), name);
}
