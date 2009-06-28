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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <thunarx/thunarx.h>

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-svn-plugin/tsp-svn-action.h>

#include <string.h>

#include <sys/wait.h>



static void tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data);



struct _TspSvnActionClass
{
  GtkActionClass __parent__;
};



struct _TspSvnAction
{
  GtkAction __parent__;

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



static GtkWidget *tsp_svn_action_create_menu_item (GtkAction *action);



static void tsp_svn_action_finalize (GObject*);

static void tsp_svn_action_set_property (GObject*, guint, const GValue*, GParamSpec*);


static GQuark tsp_action_arg_quark = 0;


static void tsp_action_exec (GtkAction *item, TspSvnAction *tsp_action);

static void tsp_action_unimplemented (GtkAction *, const gchar *);



THUNARX_DEFINE_TYPE (TspSvnAction, tsp_svn_action, GTK_TYPE_ACTION)



static void
tsp_svn_action_class_init (TspSvnActionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

  gobject_class->finalize = tsp_svn_action_finalize;
  gobject_class->set_property = tsp_svn_action_set_property;

  gtkaction_class->create_menu_item = tsp_svn_action_create_menu_item;

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

  tsp_action_arg_quark = g_quark_from_static_string ("tsp-action-arg");
}



static void
tsp_svn_action_init (TspSvnAction *self)
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



GtkAction *
tsp_svn_action_new (const gchar *name,
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
  g_return_val_if_fail(name, NULL);
  g_return_val_if_fail(label, NULL);

  GtkAction *action = g_object_new (TSP_TYPE_SVN_ACTION,
            "hide-if-empty", FALSE,
            "name", name,
            "label", label,
            "is-parent", is_parent,
            "parent-version-control", parent_version_control,
            "directory-version-control", directory_version_control,
            "directory-no-version-control", directory_no_version_control,
            "file-version-control", file_version_control,
            "file-no-version-control", file_no_version_control,
#if !GTK_CHECK_VERSION(2,9,0)
            "stock-id", "subversion",
#else
            "icon-name", "subversion",
#endif
            NULL);
  TSP_SVN_ACTION (action)->files = thunarx_file_info_list_copy (files);
//  TSP_SVN_ACTION (action)->window = gtk_widget_ref (window);
  TSP_SVN_ACTION (action)->window = window;
  return action;
}



static void
tsp_svn_action_finalize (GObject *object)
{
  thunarx_file_info_list_free (TSP_SVN_ACTION (object)->files);
  TSP_SVN_ACTION (object)->files = NULL;
//  gtk_widget_unref (TSP_SVN_ACTION (object)->window);
  TSP_SVN_ACTION (object)->window = NULL;

  G_OBJECT_CLASS (tsp_svn_action_parent_class)->finalize (object);
}



static void
tsp_svn_action_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
  {
    case PROPERTY_IS_PARENT:
      TSP_SVN_ACTION (object)->property.is_parent = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_PARENT_VERSION_CONTROL:
      TSP_SVN_ACTION (object)->property.parent_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_DIRECTORY_VERSION_CONTROL:
      TSP_SVN_ACTION (object)->property.directory_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_DIRECTORY_NO_VERSION_CONTROL:
      TSP_SVN_ACTION (object)->property.directory_no_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_FILE_VERSION_CONTROL:
      TSP_SVN_ACTION (object)->property.file_version_control = g_value_get_boolean (value)?1:0;
    break;
    case PROPERTY_FILE_NO_VERSION_CONTROL:
      TSP_SVN_ACTION (object)->property.file_no_version_control = g_value_get_boolean (value)?1:0;
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
add_subaction (GtkAction *action, GtkMenuShell *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *stock, gchar *arg)
{
    GtkAction *subaction;
    GtkWidget *subitem;

    subaction = gtk_action_new (name, text, tooltip, stock);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, arg);
    g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

    subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
    gtk_menu_shell_append (menu, subitem);
    gtk_widget_show(subitem);
}


static void
add_subaction_u (GtkMenuShell *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *stock, gchar *arg)
{
    GtkAction *subaction;
    GtkWidget *subitem;

    subaction = gtk_action_new (name, text, tooltip, stock);
    g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), arg);

    subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
    gtk_menu_shell_append (menu, subitem);
    gtk_widget_show(subitem);
}


static GtkWidget *
tsp_svn_action_create_menu_item (GtkAction *action)
{
  GtkWidget *item;
  GtkWidget *menu;
  TspSvnAction *tsp_action = TSP_SVN_ACTION (action);

  item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  /* No version control */
  if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control)) 
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::add", Q_("Menu|Add"), _("Add"), GTK_STOCK_ADD, "--add");
  }
  /* Version control (file) */
  if (tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::blame", Q_("Menu|Blame"), _("Blame"), GTK_STOCK_INDEX, "--blame");
  }
/* No need
  subitem = gtk_menu_item_new_with_label (_("Cat"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Cat");
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
  gtk_widget_show(subitem);
*//* Version control (file) */
  if (tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::changelist", Q_("Menu|Changelist"), _("Changelist"), GTK_STOCK_INDEX, "--changelist");
  }
  /* No version control (parent) */
  if (tsp_action->property.is_parent && !tsp_action->property.parent_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::checkout", Q_("Menu|Checkout"), _("Checkout"), GTK_STOCK_CONNECT, "--checkout");
  }
  /* Version control (parent) */
  if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::cleanup", Q_("Menu|Cleanup"), _("Cleanup"), GTK_STOCK_CLEAR, "--cleanup");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::commit", Q_("Menu|Commit"), _("Commit"), GTK_STOCK_APPLY, "--commit");
  }
  /* Version control (no parent) */
  if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::copy", Q_("Menu|Copy"), _("Copy"), GTK_STOCK_COPY, "--copy");
  }
  /* Version control (no parent) */
  if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::delete", Q_("Menu|Delete"), _("Delete"), GTK_STOCK_DELETE, "--delete");
  }
  /* Version control (file) */
  if (tsp_action->property.file_version_control) 
  {
    add_subaction_u (GTK_MENU_SHELL (menu), "tsp::diff", Q_("Menu|Diff"), _("Diff"), GTK_STOCK_FIND_AND_REPLACE, _("Diff"));
  }
  /* Version control and No version control (parent) */
  if (tsp_action->property.is_parent || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::export", Q_("Menu|Export"), _("Export"), GTK_STOCK_SAVE, "--export");
  }
  /* No version control (all) */
  if (!tsp_action->property.parent_version_control && (tsp_action->property.is_parent || tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control))
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::import", Q_("Menu|Import"), _("Import"), GTK_STOCK_NETWORK, "--import");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction_u (GTK_MENU_SHELL (menu), "tsp::info", Q_("Menu|Info"), _("Info"), GTK_STOCK_INFO, _("Info"));
  }
/* Ehmm...
  subitem = gtk_menu_item_new_with_label (_("List"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "List");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
  gtk_widget_show(subitem);
*//* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::lock", Q_("Menu|Lock"), _("Lock"), GTK_STOCK_DIALOG_AUTHENTICATION, "--lock");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::log", Q_("Menu|Log"), _("Log"), GTK_STOCK_INDEX, "--log");
  }
/* Ehmm ...
  subitem = gtk_menu_item_new_with_label (_("Merge"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Merge");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
  gtk_widget_show(subitem);
*//* No need
  subitem = gtk_menu_item_new_with_label (_("Make Dir"));
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Make Dir");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
  gtk_widget_show(subitem);
*//* Version control (no parent) */
  if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::move", Q_("Menu|Move"), _("Move"), GTK_STOCK_DND_MULTIPLE, "--move");
  }
/* Merged
  subitem = gtk_menu_item_new_with_label (_("Delete Properties"));
  subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
  subitem = gtk_menu_item_new_with_label (_("Get Properties"));
  subitem = gtk_menu_item_new_with_label (_("List Properties"));
  subitem = gtk_menu_item_new_with_label (_("Set Properties"));
*//* Version control */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::properties", Q_("Menu|Edit Properties"), _("Edit Properties"), GTK_STOCK_EDIT, "--properties");
  }
  /* Version control (parent) */
  if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::relocate", Q_("Menu|Relocate"), _("Relocate"), GTK_STOCK_FIND_AND_REPLACE, "--relocate");
  }
/* Changed
  subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*/if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::resolved", Q_("Menu|Resolved"), _("Resolved"), GTK_STOCK_YES, "--resolved");
  }/*
*//* Version control (file) */
  if (tsp_action->property.file_version_control)
  {
    add_subaction_u (GTK_MENU_SHELL (menu), "tsp::resolve", Q_("Menu|Resolve"), _("Resolve"), GTK_STOCK_YES, _("Resolve"));
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::revert", Q_("Menu|Revert"), _("Revert"), GTK_STOCK_UNDO, "--revert");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::status", Q_("Menu|Status"), _("Status"), GTK_STOCK_DIALOG_INFO, "--status");
  }
  /* Version control (parent) */
  if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::switch", Q_("Menu|Switch"), _("Switch"), GTK_STOCK_JUMP_TO, "--switch");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::unlock", Q_("Menu|Unlock"), _("Unlock"), NULL, "--unlock");
  }
  /* Version control (all) */
  if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
  {
    add_subaction (action, GTK_MENU_SHELL (menu), "tsp::update", Q_("Menu|Update"), _("Update"), GTK_STOCK_REFRESH, "--update");
  }

  return item;
}



static void tsp_action_unimplemented (GtkAction *item, const gchar *tsp_action)
{
  GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), tsp_action);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}



static void tsp_action_exec (GtkAction *item, TspSvnAction *tsp_action)
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
  GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

  iter = tsp_action->files;

  size = g_list_length (iter);

  argv = g_new (gchar *, size + 3);

  argv[0] = g_strdup (TSP_SVN_HELPER);
  argv[1] = g_strdup (g_object_get_qdata (G_OBJECT (item), tsp_action_arg_quark));
  argv[size + 2] = NULL;

  if(iter)
  {
    if(tsp_action->property.is_parent)
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
        if (file[strlen (file) - 1] == '/')
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
  if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &error))
  {
    GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_error_free (error);
  }
  else
  {
    g_signal_emit(tsp_action, action_signal[SIGNAL_NEW_PROCESS], 0, &pid, watch_path);
  }
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

