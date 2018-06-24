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

#include <string.h>
#include <sys/wait.h>
#include <libxfce4util/libxfce4util.h>
#include <thunar-vcs-plugin/tvp-svn-action.h>



static void tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data);



struct _TvpSvnActionClass
{
  ThunarxMenuItemClass __parent__;
};



struct _TvpSvnAction
{
  ThunarxMenuItem __parent__;

  struct {
    unsigned is_parent : 1;
    unsigned parent_version_control : 1;
    unsigned directory_version_control : 1;
    unsigned directory_no_version_control : 1;
    unsigned file_version_control : 1;
    unsigned file_no_version_control : 1;
  } property;

  GList *files;
  GtkWidget *window;
};



enum {
  PROPERTY_IS_PARENT = 1,
  PROPERTY_PARENT_VERSION_CONTROL,
  PROPERTY_DIRECTORY_VERSION_CONTROL,
  PROPERTY_DIRECTORY_NO_VERSION_CONTROL,
  PROPERTY_FILE_VERSION_CONTROL,
  PROPERTY_FILE_NO_VERSION_CONTROL
};



enum {
  SIGNAL_NEW_PROCESS = 0,
  SIGNAL_COUNT
};



static guint action_signal[SIGNAL_COUNT];



static void tvp_svn_action_create_menu_item (ThunarxMenuItem *action);

static void tvp_svn_action_finalize (GObject*);

static void tvp_svn_action_set_property (GObject*, guint, const GValue*, GParamSpec*);


static GQuark tvp_action_arg_quark = 0;


static void tvp_action_exec (ThunarxMenuItem *item, TvpSvnAction *tvp_action);

static void tvp_action_unimplemented (ThunarxMenuItem *, const gchar *);



THUNARX_DEFINE_TYPE (TvpSvnAction, tvp_svn_action, THUNARX_TYPE_MENU_ITEM)



static void
tvp_svn_action_class_init (TvpSvnActionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = tvp_svn_action_finalize;
  gobject_class->set_property = tvp_svn_action_set_property;

  g_object_class_install_property (gobject_class, PROPERTY_IS_PARENT,
    g_param_spec_boolean ("is-parent", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROPERTY_PARENT_VERSION_CONTROL,
    g_param_spec_boolean ("parent-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROPERTY_DIRECTORY_VERSION_CONTROL,
    g_param_spec_boolean ("directory-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROPERTY_DIRECTORY_NO_VERSION_CONTROL,
    g_param_spec_boolean ("directory-no-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROPERTY_FILE_VERSION_CONTROL,
    g_param_spec_boolean ("file-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, PROPERTY_FILE_NO_VERSION_CONTROL,
    g_param_spec_boolean ("file-no-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  action_signal[SIGNAL_NEW_PROCESS] = g_signal_new("new-process", G_OBJECT_CLASS_TYPE(gobject_class), G_SIGNAL_RUN_FIRST,
    0, NULL, NULL, tsh_cclosure_marshal_VOID__POINTER_STRING, G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_STRING);

  tvp_action_arg_quark = g_quark_from_string ("tvp-action-arg");
}



static void
tvp_svn_action_init (TvpSvnAction *self)
{
  self->property.is_parent = 0;
  self->property.parent_version_control = 0;
  self->property.directory_version_control = 0;
  self->property.directory_no_version_control = 0;
  self->property.file_version_control = 0;
  self->property.file_no_version_control = 0;
  self->files = NULL;
  self->window = NULL;
}



ThunarxMenuItem *
tvp_svn_action_new (const gchar *name,
                    const gchar *label,
                    GList *files,
                    GtkWidget *window,
                    gboolean is_parent,
                    gboolean parent_version_control,
                    gboolean directory_version_control,
                    gboolean directory_no_version_control,
                    gboolean file_version_control,
                    gboolean file_no_version_control)
{
    ThunarxMenuItem *item;

  g_return_val_if_fail(name, NULL);
  g_return_val_if_fail(label, NULL);

  item = g_object_new (TVP_TYPE_SVN_ACTION,
            "name", name,
            "label", label,
            "is-parent", is_parent,
            "parent-version-control", parent_version_control,
            "directory-version-control", directory_version_control,
            "directory-no-version-control", directory_no_version_control,
            "file-version-control", file_version_control,
            "file-no-version-control", file_no_version_control,
            "icon", "subversion",
            NULL);
  TVP_SVN_ACTION (item)->files = thunarx_file_info_list_copy (files);
  TVP_SVN_ACTION (item)->window = window;

  tvp_svn_action_create_menu_item (item);

  return item;
}



static void
tvp_svn_action_finalize (GObject *object)
{
  thunarx_file_info_list_free (TVP_SVN_ACTION (object)->files);
  TVP_SVN_ACTION (object)->files = NULL;
  TVP_SVN_ACTION (object)->window = NULL;

  G_OBJECT_CLASS (tvp_svn_action_parent_class)->finalize (object);
}



static void
tvp_svn_action_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
  {
    case PROPERTY_IS_PARENT:
      TVP_SVN_ACTION (object)->property.is_parent = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_PARENT_VERSION_CONTROL:
      TVP_SVN_ACTION (object)->property.parent_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_DIRECTORY_VERSION_CONTROL:
      TVP_SVN_ACTION (object)->property.directory_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_DIRECTORY_NO_VERSION_CONTROL:
      TVP_SVN_ACTION (object)->property.directory_no_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_FILE_VERSION_CONTROL:
      TVP_SVN_ACTION (object)->property.file_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_FILE_NO_VERSION_CONTROL:
      TVP_SVN_ACTION (object)->property.file_no_version_control = g_value_get_boolean (value)?1:0;
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
add_subaction(ThunarxMenuItem *item, ThunarxMenu *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *icon, gchar *arg)
{
    ThunarxMenuItem *subitem;
    subitem = thunarx_menu_item_new (name, text, tooltip, icon);
    thunarx_menu_append_item (menu, subitem);
    g_object_set_qdata (G_OBJECT (subitem), tvp_action_arg_quark, arg);
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_exec), item);
    g_object_unref (subitem);
}


static void
add_subaction_u (ThunarxMenu *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *icon, gchar *arg)
{
    /* keep the current behavior, only show menu items if they are implemented  */
    /*ThunarxMenuItem *subitem;
    subitem = thunarx_menu_item_new (name, text, tooltip, icon);
    thunarx_menu_append_item (menu, subitem);
    g_object_set_qdata (G_OBJECT (subitem), tvp_action_arg_quark, arg);
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_unimplemented), arg);
    g_object_unref (subitem);*/
}


static void
tvp_svn_action_create_menu_item (ThunarxMenuItem *item)
{
  ThunarxMenu *menu;
  TvpSvnAction *tvp_action = TVP_SVN_ACTION (item);

  menu = thunarx_menu_new ();
  thunarx_menu_item_set_menu (item, menu);

  /* No version control or version control (parent) */
  if (tvp_action->property.parent_version_control && (tvp_action->property.is_parent || tvp_action->property.directory_no_version_control || tvp_action->property.file_no_version_control)) 
  {
    add_subaction (item, menu, "tvp::add", C_("Menu", "Add"), _("Add"), "list-add", "--add");
  }
  /* Version control (file) */
  if (tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::blame", C_("Menu", "Blame"), _("Blame"), "gtk-index", "--blame");
  }
/* No need
  subitem = gtk_menu_item_new_with_label (_("Cat"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_unimplemented), "Cat");
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
  gtk_menu_shell_append (menu, subitem);
  gtk_widget_show(subitem);
*//* Version control (file) */
  if (tvp_action->property.file_version_control)
  {
    add_subaction_u (menu, "tvp::changelist", C_("Menu", "Changelist"), _("Changelist"), "gtk-index", _("Changelist"));
  }
  /* No version control (parent) */
  if (tvp_action->property.is_parent && !tvp_action->property.parent_version_control)
  {
    add_subaction (item, menu, "tvp::checkout", C_("Menu", "Checkout"), _("Checkout"), "gtk-connect", "--checkout");
  }
  /* Version control (parent) */
  if (tvp_action->property.is_parent && tvp_action->property.parent_version_control)
  {
    add_subaction (item, menu, "tvp::cleanup", C_("Menu", "Cleanup"), _("Cleanup"), "edit-clear", "--cleanup");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::commit", C_("Menu", "Commit"), _("Commit"), "gtk-apply", "--commit");
  }
  /* Version control (no parent) */
  if (!tvp_action->property.is_parent && tvp_action->property.parent_version_control && (tvp_action->property.directory_version_control || tvp_action->property.file_version_control))
  {
    add_subaction (item, menu, "tvp::copy", C_("Menu", "Copy"), _("Copy"), "edit-copy", "--copy");
  }
  /* Version control (no parent) */
  if (!tvp_action->property.is_parent && tvp_action->property.parent_version_control && (tvp_action->property.directory_version_control || tvp_action->property.file_version_control))
  {
    add_subaction (item, menu, "tvp::delete", C_("Menu", "Delete"), _("Delete"), "edit-delete", "--delete");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::diff", C_("Menu", "Diff"), _("Diff"), "gtk-convert", "--diff");
  }
  /* Version control and No version control (parent) */
  if (tvp_action->property.is_parent || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::export", C_("Menu", "Export"), _("Export"), "document-save", "--export");
  }
  /* No version control (all) */
  if (!tvp_action->property.parent_version_control && (tvp_action->property.is_parent || tvp_action->property.directory_no_version_control || tvp_action->property.file_no_version_control))
  {
    add_subaction (item, menu, "tvp::import", C_("Menu", "Import"), _("Import"), "network-workgroup", "--import");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction_u (menu, "tvp::info", C_("Menu", "Info"), _("Info"), "dialog-information", _("Info"));
  }
/* Ehmm...
  subitem = gtk_menu_item_new_with_label (_("List"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_unimplemented), "List");
  gtk_menu_shell_append (menu, subitem);
  gtk_widget_show(subitem);
*//* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::lock", C_("Menu", "Lock"), _("Lock"), "dialog-password", "--lock");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::log", C_("Menu", "Log"), _("Log"), "gtk-index", "--log");
  }
/* Ehmm ...
  subitem = gtk_menu_item_new_with_label (_("Merge"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_unimplemented), "Merge");
  gtk_menu_shell_append (menu, subitem);
  gtk_widget_show(subitem);
*//* No need
  subitem = gtk_menu_item_new_with_label (_("Make Dir"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_unimplemented), "Make Dir");
  gtk_menu_shell_append (menu, subitem);
  gtk_widget_show(subitem);
*//* Version control (no parent) */
  if (!tvp_action->property.is_parent && tvp_action->property.parent_version_control && (tvp_action->property.directory_version_control || tvp_action->property.file_version_control))
  {
    add_subaction (item, menu, "tvp::move", C_("Menu", "Move"), _("Move"), "gtk-dnd-multiple", "--move");
  }
/* Merged
  subitem = gtk_menu_item_new_with_label (_("Delete Properties"));
  subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
  subitem = gtk_menu_item_new_with_label (_("Get Properties"));
  subitem = gtk_menu_item_new_with_label (_("List Properties"));
  subitem = gtk_menu_item_new_with_label (_("Set Properties"));
*//* Version control */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::properties", C_("Menu", "Edit Properties"), _("Edit Properties"), "gtk-edit", "--properties");
  }
  /* Version control (parent) */
  if (tvp_action->property.is_parent && tvp_action->property.parent_version_control)
  {
    add_subaction (item, menu, "tvp::relocate", C_("Menu", "Relocate"), _("Relocate"), "edit-find-replace", "--relocate");
  }
/* Changed
  subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*/if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::resolved", C_("Menu", "Resolved"), _("Resolved"), "gtk-yes", "--resolved");
  }/*
*//* Version control (file) */
  if (tvp_action->property.file_version_control)
  {
    add_subaction_u (menu, "tvp::resolve", C_("Menu", "Resolve"), _("Resolve"), "gtk-yes", _("Resolve"));
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::revert", C_("Menu", "Revert"), _("Revert"), "edit-undo", "--revert");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::status", C_("Menu", "Status"), _("Status"), "dialog-information", "--status");
  }
  /* Version control (parent) */
  if (tvp_action->property.is_parent && tvp_action->property.parent_version_control)
  {
    add_subaction (item, menu, "tvp::switch", C_("Menu", "Switch"), _("Switch"), "go-jump", "--switch");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::unlock", C_("Menu", "Unlock"), _("Unlock"), NULL, "--unlock");
  }
  /* Version control (all) */
  if ((tvp_action->property.is_parent && tvp_action->property.parent_version_control) || tvp_action->property.directory_version_control || tvp_action->property.file_version_control)
  {
    add_subaction (item, menu, "tvp::update", C_("Menu", "Update"), _("Update"), "view-refresh", "--update");
  }
}



static void tvp_action_unimplemented (ThunarxMenuItem *item, const gchar *tvp_action)
{
  GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), tvp_action);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}


static void
tvp_setup_display_cb (gpointer data)
{
    g_setenv ("DISPLAY", (char *) data, TRUE);
}



static void tvp_action_exec (ThunarxMenuItem *item, TvpSvnAction *tvp_action)
{
  guint size, i;
  gchar **argv;
  GList *iter;
  gchar *uri;
  gchar *filename;
  gchar *file;
  gchar *watch_path = NULL;
  gint pid;
  GError *error = NULL;
  char *display_name = NULL;
  GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tvp_action->window));
  GdkDisplay *display = gdk_screen_get_display (screen);

  iter = tvp_action->files;

  size = g_list_length (iter);

  argv = g_new (gchar *, size + 3);

  argv[0] = g_strdup (TVP_SVN_HELPER);
  argv[1] = g_strdup (g_object_get_qdata (G_OBJECT (item), tvp_action_arg_quark));
  argv[size + 2] = NULL;

  if(iter)
  {
    if(tvp_action->property.is_parent)
    {
      uri = thunarx_file_info_get_uri (iter->data);
      watch_path = g_filename_from_uri (uri, NULL, NULL);
      g_free (uri);
    }
    else
    {
      uri = thunarx_file_info_get_parent_uri (iter->data);
      watch_path = g_filename_from_uri (uri, NULL, NULL);
      g_free (uri);
    }
  }

  for (i = 0; i < size; i++)
  {
    /* determine the URI for the file info */
    uri = thunarx_file_info_get_uri (iter->data);
    if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
      {
        file = filename;
        /* strip the "file://" part of the uri */
        if (strncmp (file, "file://", 7) == 0)
        {
          file += 7;
        }

        file = g_strdup (file);

        /* remove trailing '/' cause svn can't handle that */
        if (strlen (file) > 1 && file[strlen (file) - 1] == '/')
        {
          file[strlen (file) - 1] = '\0';
        }

        argv[i+2] = file;

        /* release the filename */
        g_free (filename);
      }

      /* release the URI */
      g_free (uri);
    }

    iter = g_list_next (iter);
  }

  pid = 0;
  if (screen != NULL)
    display_name = g_strdup (gdk_display_get_name (display));

  if (!g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, tvp_setup_display_cb, display_name, &pid, &error))
  {
    GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tvp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TVP_SVN_HELPER "\'");
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_error_free (error);
  }
  else
  {
    g_signal_emit(tvp_action, action_signal[SIGNAL_NEW_PROCESS], 0, &pid, watch_path);
  }

  g_free (display_name);
  g_free (watch_path);
  g_strfreev (argv);
}



static void
tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                           GValue       *return_value G_GNUC_UNUSED,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint G_GNUC_UNUSED,
                                           gpointer      marshal_data)
{
  typedef void (*TshMarshalFunc_VOID__POINTER_STRING) (gpointer       data1,
                                                       gconstpointer  arg_1,
                                                       gconstpointer  arg_2,
                                                       gpointer       data2);
  register TshMarshalFunc_VOID__POINTER_STRING callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
  {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  }
  else
  {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback = (TshMarshalFunc_VOID__POINTER_STRING) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_pointer (param_values + 1),
            g_value_get_string (param_values + 2),
            data2);
}

