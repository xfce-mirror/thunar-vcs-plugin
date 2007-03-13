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
};



static GtkWidget *tsp_svn_action_create_menu_item (GtkAction *action);



G_DEFINE_TYPE (TspSvnAction, tsp_svn_action, GTK_TYPE_ACTION)



static void
tsp_svn_action_class_init (TspSvnActionClass *klass)
{
	GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

	gtkaction_class->create_menu_item = tsp_svn_action_create_menu_item;
}



static void
tsp_svn_action_init (TspSvnAction *self)
{
}



GtkAction *
tsp_svn_action_new (const gchar *name,
                    const gchar *label)
{
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(label, NULL);

	return g_object_new (TSP_TYPE_SVN_ACTION,
	                     "hide-if-empty", FALSE,
											 "name", name,
											 "label", label,
											 NULL);
}



static GtkWidget *
tsp_svn_action_create_menu_item (GtkAction *action)
{
	GtkWidget *item;
	GtkWidget *menu;
	GtkWidget *subitem;

	item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	/* No version control */
	subitem = gtk_menu_item_new_with_label (_("Add"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (file) */
	subitem = gtk_menu_item_new_with_label (_("Blame"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
/* No need
	subitem = gtk_menu_item_new_with_label (_("Cat"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Not here
	subitem = gtk_menu_item_new_with_label (_("Checkout"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (parent) */
	subitem = gtk_menu_item_new_with_label (_("Cleanup"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Commit"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
/* Ehmm ...
	subitem = gtk_menu_item_new_with_label (_("Copy"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (no parent) */
	subitem = gtk_menu_item_new_with_label (_("Delete"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (file) */
	subitem = gtk_menu_item_new_with_label (_("Diff"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control */
	subitem = gtk_menu_item_new_with_label (_("Export"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
/* Not here
	subitem = gtk_menu_item_new_with_label (_("Import"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Info"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
/* Ehmm...
	subitem = gtk_menu_item_new_with_label (_("List"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Lock"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Log"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
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
	subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);

/* Changed
	subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*//* Version control (file) */
	subitem = gtk_menu_item_new_with_label (_("Resolve"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Revert"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Status"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (parent) */
	subitem = gtk_menu_item_new_with_label (_("Switch"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Unlock"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
	/* Version control (all) */
	subitem = gtk_menu_item_new_with_label (_("Update"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);

	return item;
}

