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

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-svn-plugin/tsp-svn-action.h>



struct _TspSvnActionClass
{
	GtkActionClass __parent__;
};



struct _TspSvnAction
{
	GtkAction __parent__;

	struct {
		unsigned is_parent : 1;
		unsigned is_directory : 1;
		unsigned is_file : 1;
		unsigned parent_version_control : 1;
		unsigned version_control : 1;
		unsigned no_version_control : 1;
	} property;
};



enum {
	PROPERTY_IS_PARENT = 1,
	PROPERTY_IS_DIRECTORY,
	PROPERTY_IS_FILE,
	PROPERTY_PARENT_VERSION_CONTROL,
	PROPERTY_VERSION_CONTROL,
	PROPERTY_NO_VERSION_CONTROL
};



static GtkWidget *tsp_svn_action_create_menu_item (GtkAction *action);



static void tsp_svn_action_set_property (GObject*, guint, const GValue*, GParamSpec*);



G_DEFINE_TYPE (TspSvnAction, tsp_svn_action, GTK_TYPE_ACTION)



static void
tsp_svn_action_class_init (TspSvnActionClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

	gobject_class->set_property = tsp_svn_action_set_property;

	gtkaction_class->create_menu_item = tsp_svn_action_create_menu_item;

	g_object_class_install_property (gobject_class, PROPERTY_IS_PARENT,
		g_param_spec_boolean ("is-parent", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

	g_object_class_install_property (gobject_class, PROPERTY_IS_DIRECTORY,
		g_param_spec_boolean ("is-directory", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

	g_object_class_install_property (gobject_class, PROPERTY_IS_FILE,
		g_param_spec_boolean ("is-file", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

	g_object_class_install_property (gobject_class, PROPERTY_PARENT_VERSION_CONTROL,
		g_param_spec_boolean ("parent-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

	g_object_class_install_property (gobject_class, PROPERTY_VERSION_CONTROL,
		g_param_spec_boolean ("version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

	g_object_class_install_property (gobject_class, PROPERTY_NO_VERSION_CONTROL,
		g_param_spec_boolean ("no-version-control", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));
}



static void
tsp_svn_action_init (TspSvnAction *self)
{
	self->property.is_parent = 0;
	self->property.is_directory = 0;
	self->property.is_file = 0;
	self->property.parent_version_control = 0;
	self->property.version_control = 0;
	self->property.no_version_control = 0;
}



GtkAction *
tsp_svn_action_new (const gchar *name,
                    const gchar *label,
										gboolean is_parent,
										gboolean is_directory,
										gboolean is_file,
										gboolean parent_version_control,
										gboolean version_control,
										gboolean no_version_control)
{
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(label, NULL);

	return g_object_new (TSP_TYPE_SVN_ACTION,
	                     "hide-if-empty", FALSE,
											 "name", name,
											 "label", label,
											 "is-parent", is_parent,
											 "is-directory", is_directory,
											 "is-file", is_file,
											 "parent-version-control", parent_version_control,
											 "version-control", version_control,
											 "no-version-control", no_version_control,
											 NULL);
}



static void
tsp_svn_action_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
		case PROPERTY_IS_PARENT:
			TSP_SVN_ACTION (object)->property.is_parent = g_value_get_boolean (value)?1:0;
		break;
		case PROPERTY_IS_DIRECTORY:
			TSP_SVN_ACTION (object)->property.is_directory = g_value_get_boolean (value)?1:0;
		break;
		case PROPERTY_IS_FILE:
			TSP_SVN_ACTION (object)->property.is_file = g_value_get_boolean (value)?1:0;
		break;
		case PROPERTY_PARENT_VERSION_CONTROL:
			TSP_SVN_ACTION (object)->property.parent_version_control = g_value_get_boolean (value)?1:0;
		break;
		case PROPERTY_VERSION_CONTROL:
			TSP_SVN_ACTION (object)->property.version_control = g_value_get_boolean (value)?1:0;
		break;
		case PROPERTY_NO_VERSION_CONTROL:
			TSP_SVN_ACTION (object)->property.no_version_control = g_value_get_boolean (value)?1:0;
		break;
	}
}


static GtkWidget *
tsp_svn_action_create_menu_item (GtkAction *action)
{
	GtkWidget *item;
	GtkWidget *menu;
	GtkWidget *subitem;
	TspSvnAction *tsp_action = TSP_SVN_ACTION (action);

	item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	/* No version control */
	if (tsp_action->property.parent_version_control && tsp_action->property.no_version_control && !tsp_action->property.is_parent) 
	{
		subitem = gtk_menu_item_new_with_label (_("Add"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.parent_version_control && tsp_action->property.version_control && tsp_action->property.is_file) 
	{
		subitem = gtk_menu_item_new_with_label (_("Blame"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* No need
	subitem = gtk_menu_item_new_with_label (_("Cat"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Not here
	subitem = gtk_menu_item_new_with_label (_("Checkout"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (parent) */
	if (tsp_action->property.parent_version_control && tsp_action->property.is_parent) 
	{
		subitem = gtk_menu_item_new_with_label (_("Cleanup"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && !tsp_action->property.is_parent) )
	{
		subitem = gtk_menu_item_new_with_label (_("Commit"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Ehmm ...
	subitem = gtk_menu_item_new_with_label (_("Copy"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (no parent) */
	if (tsp_action->property.parent_version_control && tsp_action->property.version_control && !tsp_action->property.is_parent) 
	{
		subitem = gtk_menu_item_new_with_label (_("Delete"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.parent_version_control && tsp_action->property.version_control && tsp_action->property.is_file) 
	{
		subitem = gtk_menu_item_new_with_label (_("Diff"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && !tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Export"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Not here
	subitem = gtk_menu_item_new_with_label (_("Import"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Info"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Ehmm...
	subitem = gtk_menu_item_new_with_label (_("List"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Lock"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Log"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Ehmm ...
	subitem = gtk_menu_item_new_with_label (_("Merge"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* No need
	subitem = gtk_menu_item_new_with_label (_("Make Dir"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Ehmm ...
	subitem = gtk_menu_item_new_with_label (_("Move"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Merged
	subitem = gtk_menu_item_new_with_label (_("Delete Properties"));
	subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
	subitem = gtk_menu_item_new_with_label (_("Get Properties"));
	subitem = gtk_menu_item_new_with_label (_("List Properties"));
	subitem = gtk_menu_item_new_with_label (_("Set Properties"));
*//* Version control */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Changed
	subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*//* Version control (file) */
	if (tsp_action->property.parent_version_control && tsp_action->property.version_control && tsp_action->property.is_file)
	{
		subitem = gtk_menu_item_new_with_label (_("Resolve"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Revert"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Status"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (parent) */
	if (tsp_action->property.version_control && tsp_action->property.is_parent)
	{
		subitem = gtk_menu_item_new_with_label (_("Switch"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Unlock"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if (tsp_action->property.parent_version_control || (tsp_action->property.version_control && tsp_action->property.is_parent))
	{
		subitem = gtk_menu_item_new_with_label (_("Update"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}

	return item;
}

