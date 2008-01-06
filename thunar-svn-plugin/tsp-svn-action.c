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



static GtkWidget *tsp_svn_action_create_menu_item (GtkAction *action);



static void tsp_svn_action_finalize (GObject*);

static void tsp_svn_action_set_property (GObject*, guint, const GValue*, GParamSpec*);



void tsp_action_add (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_checkout (GtkAction *, TspSvnAction *);

void tsp_action_cleanup (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_commit (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_copy (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_delete (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_export (GtkAction *, TspSvnAction *);

void tsp_action_import (GtkAction *, TspSvnAction *);

void tsp_action_lock (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_log (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_move (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_properties (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_resolved (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_revert (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_status (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_unlock (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_update (GtkAction *item, TspSvnAction *tsp_action);

void tsp_action_unimplemented (GtkAction *, const gchar *);



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
//	TSP_SVN_ACTION (action)->window = gtk_widget_ref (window);
	TSP_SVN_ACTION (action)->window = window;
	return action;
}



static void
tsp_svn_action_finalize (GObject *object)
{
	thunarx_file_info_list_free (TSP_SVN_ACTION (object)->files);
	TSP_SVN_ACTION (object)->files = NULL;
//	gtk_widget_unref (TSP_SVN_ACTION (object)->window);
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


static GtkWidget *
tsp_svn_action_create_menu_item (GtkAction *action)
{
	GtkWidget *item;
	GtkWidget *menu;
	GtkWidget *subaction;
	GtkWidget *subitem;
	TspSvnAction *tsp_action = TSP_SVN_ACTION (action);

	item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	/* No version control */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control)) 
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::add", "label", _("Add"),
				    "stock-id", GTK_STOCK_ADD,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_add), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::blame", "label", _("Blame"), NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Blame"));

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
/* No need
	subitem = gtk_menu_item_new_with_label (_("Cat"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Cat");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* No version control (parent) */
	if (tsp_action->property.is_parent && !tsp_action->property.parent_version_control)
  {
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::checkout", "label", _("Checkout"),
				    "stock-id", GTK_STOCK_CONNECT,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_checkout), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
  }
  /* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::cleanup", "label", _("Cleanup"),
				    "stock-id", GTK_STOCK_CLEAR,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_cleanup), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::commit", "label", _("Commit"),
				    "stock-id", GTK_STOCK_APPLY,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_commit), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::copy", "label", _("Copy"),
				    "stock-id", GTK_STOCK_COPY,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_copy), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
        }
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::delete", "label", _("Delete"),
				    "stock-id", GTK_STOCK_DELETE,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_delete), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control) 
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::diff", "label", _("Diff"),
				    "stock-id", GTK_STOCK_FIND_AND_REPLACE,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Diff"));

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control and No version control (parent) */
	if (tsp_action->property.is_parent || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::export", "label", _("Export"),
				    "stock-id", GTK_STOCK_SAVE,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_export), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* No version control (all) */
	if (!tsp_action->property.parent_version_control && (tsp_action->property.is_parent || tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control))
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::import", "label", _("Import"),
				    "stock-id", GTK_STOCK_NETWORK,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_import), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::info", "label", _("Info"),
				    "stock-id", GTK_STOCK_INFO,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Info"));

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
/* Ehmm...
	subitem = gtk_menu_item_new_with_label (_("List"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "List");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::lock", "label", _("Lock"),
				    "stock-id", GTK_STOCK_DIALOG_AUTHENTICATION,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_lock), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::log", "label", _("Log"),
				    "stock-id", GTK_STOCK_INDEX,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_log), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
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
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::move", "label", _("Move"),
				    "stock-id", GTK_STOCK_DND_MULTIPLE,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_move), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
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
subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::properties", "label", _("Edit Properties"),
				    "stock-id", GTK_STOCK_EDIT,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_properties), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
/* Changed
	subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*/if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::resolved", "label", _("Resolved"),
				    "stock-id", GTK_STOCK_YES,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_resolved), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}/*
*//* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::resolve", "label", _("Resolve"),
				    "stock-id", GTK_STOCK_YES,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Resolve"));

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::revert", "label", _("Revert"),
				    "stock-id", GTK_STOCK_UNDO,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_revert), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::status", "label", _("Status"),
				    "stock-id", GTK_STOCK_DIALOG_INFO,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_status), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::switch", "label", _("Switch"),
				    "stock-id", GTK_STOCK_JUMP_TO,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Switch"));

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::unlock", "label", _("Unlock"), NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unlock), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = g_object_new (GTK_TYPE_ACTION, "name", "tsp::update", "label", _("Update"),
				    "stock-id", GTK_STOCK_REFRESH,
				    NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_update), action);

	  subitem = gtk_action_create_menu_item (GTK_ACTION (subaction));
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}

	return item;
}



void tsp_action_unimplemented (GtkAction *item, const gchar *tsp_action)
{
  GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), tsp_action);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}



void tsp_action_add (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--add");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_checkout (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--checkout");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_cleanup (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--cleanup");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_commit (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--commit");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_copy (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--copy");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_delete (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--delete");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_export (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--export");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_import (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--import");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_lock (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--lock");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_log (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--log");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_move (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--move");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_properties (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--properties");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_resolved (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--resolved");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_revert (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--revert");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_status (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--status");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_unlock (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--unlock");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_update (GtkAction *item, TspSvnAction *tsp_action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tsp_action->window));

	iter = tsp_action->files;

	size = g_list_length (iter);

	argv = g_new (gchar *, size + 3);

	argv[0] = g_strdup (TSP_SVN_HELPER);
	argv[1] = g_strdup ("--update");
	argv[size + 2] = NULL;

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

	if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, 0, NULL, NULL, &pid, &error))
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}

