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


static GQuark tsp_action_arg_quark = 0;


void tsp_action_exec (GtkAction *item, TspSvnAction *tsp_action);

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
	GtkAction *subaction;
	GtkWidget *subitem;
  const gchar *tooltip;
	TspSvnAction *tsp_action = TSP_SVN_ACTION (action);

	item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	/* No version control */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control)) 
	{
	  subaction = gtk_action_new ("tsp::add", Q_("Menu|Add"), _("Add"), GTK_STOCK_ADD);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--add");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::blame", Q_("Menu|Blame"), _("Blame"), NULL);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Blame"));

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
/* No need
	subitem = gtk_menu_item_new_with_label (_("Cat"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Cat");
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* No version control (parent) */
	if (tsp_action->property.is_parent && !tsp_action->property.parent_version_control)
  {
	  subaction = gtk_action_new ("tsp::checkout", Q_("Menu|Checkout"), _("Checkout"), GTK_STOCK_CONNECT);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--checkout");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
  }
  /* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
	  subaction = gtk_action_new ("tsp::cleanup", Q_("Menu|Cleanup"), _("Cleanup"), GTK_STOCK_CLEAR);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--cleanup");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::commit", Q_("Menu|Commit"), _("Commit"), GTK_STOCK_APPLY);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--commit");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
	{
	  subaction = gtk_action_new ("tsp::copy", Q_("Menu|Copy"), _("Copy"), GTK_STOCK_COPY);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--copy");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
        }
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
	{
	  subaction = gtk_action_new ("tsp::delete", Q_("Menu|Delete"), _("Delete"), GTK_STOCK_DELETE);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--delete");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control) 
	{
	  subaction = gtk_action_new ("tsp::diff", Q_("Menu|Diff"), _("Diff"), GTK_STOCK_FIND_AND_REPLACE);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Diff"));

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control and No version control (parent) */
	if (tsp_action->property.is_parent || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::export", Q_("Menu|Export"), _("Export"), GTK_STOCK_SAVE);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--export");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* No version control (all) */
	if (!tsp_action->property.parent_version_control && (tsp_action->property.is_parent || tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control))
	{
	  subaction = gtk_action_new ("tsp::import", Q_("Menu|Import"), _("Import"), GTK_STOCK_NETWORK);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--import");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
  /* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::info", Q_("Menu|Info"), _("Info"), GTK_STOCK_INFO);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Info"));

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
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
	  subaction = gtk_action_new ("tsp::lock", Q_("Menu|Lock"), _("Lock"), GTK_STOCK_DIALOG_AUTHENTICATION);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--lock");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::log", Q_("Menu|Log"), _("Log"), GTK_STOCK_INDEX);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--log");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
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
	  subaction = gtk_action_new ("tsp::move", Q_("Menu|Move"), _("Move"), GTK_STOCK_DND_MULTIPLE);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--move");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
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
subaction = gtk_action_new ("tsp::properties", Q_("Menu|Edit Properties"), _("Edit Properties"), GTK_STOCK_EDIT);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--properties");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
/* Changed
	subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*/if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::resolved", Q_("Menu|Resolved"), _("Resolved"), GTK_STOCK_YES);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--resolved");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}/*
*//* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::resolve", Q_("Menu|Resolve"), _("Resolve"), GTK_STOCK_YES);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Resolve"));

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::revert", Q_("Menu|Revert"), _("Revert"), GTK_STOCK_UNDO);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--revert");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::status", Q_("Menu|Status"), _("Status"), GTK_STOCK_DIALOG_INFO);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--status");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
	  subaction = gtk_action_new ("tsp::switch", Q_("Menu|Switch"), _("Switch"), GTK_STOCK_JUMP_TO);
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_unimplemented), _("Switch"));

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::unlock", Q_("Menu|Unlock"), _("Unlock"), NULL);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--unlock");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
	  gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	  gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
	  subaction = gtk_action_new ("tsp::update", Q_("Menu|Update"), _("Update"), GTK_STOCK_REFRESH);
    g_object_set_qdata (G_OBJECT (subaction), tsp_action_arg_quark, "--update");
	  g_signal_connect_after (subaction, "activate", G_CALLBACK (tsp_action_exec), action);

	  subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
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



void tsp_child_watch (GPid pid, gint status, gpointer data)
{
  gchar *watch_path = data;

  if (G_LIKELY (data))
  {
    GDK_THREADS_ENTER ();

    ThunarVfsPath *path = thunar_vfs_path_new (watch_path, NULL);

    if (G_LIKELY (path))
    {
      ThunarVfsMonitor *monitor = thunar_vfs_monitor_get_default ();
      thunar_vfs_monitor_feed (monitor, THUNAR_VFS_MONITOR_EVENT_CHANGED, path);
      g_object_unref (G_OBJECT (monitor));
      thunar_vfs_path_unref (path);
    }

    GDK_THREADS_LEAVE ();

    //this is done by destroy callback
    //g_free (watch_path);
  }

  g_spawn_close_pid (pid);
}



void tsp_action_exec (GtkAction *item, TspSvnAction *tsp_action)
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
    g_free (watch_path);
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tsp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}
  else
  {
    g_child_watch_add_full (G_PRIORITY_LOW, pid, tsp_child_watch, watch_path, g_free);
  }

	g_strfreev (argv);
}

