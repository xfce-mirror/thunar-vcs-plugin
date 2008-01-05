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



void tsp_action_add (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_cleanup (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_commit (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_copy (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_delete (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_lock (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_log (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_move (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_properties (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_resolved (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_revert (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_status (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_unlock (GtkMenuItem *item, TspSvnAction *action);

void tsp_action_update (GtkMenuItem *item, TspSvnAction *action);



G_DEFINE_TYPE (TspSvnAction, tsp_svn_action, GTK_TYPE_ACTION)



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
	GtkWidget *subitem;
	TspSvnAction *tsp_action = TSP_SVN_ACTION (action);

	item = GTK_ACTION_CLASS(tsp_svn_action_parent_class)->create_menu_item (action);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	/* No version control */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_no_version_control || tsp_action->property.file_no_version_control)) 
	{
		subitem = gtk_menu_item_new_with_label (_("Add"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_add), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Blame"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Blame");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* No need
	subitem = gtk_menu_item_new_with_label (_("Cat"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Cat");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Not here
	subitem = gtk_menu_item_new_with_label (_("Checkout"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Checkout");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Cleanup"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_cleanup), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Commit"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_commit), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
  {
    subitem = gtk_menu_item_new_with_label (_("Copy"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_copy), action, G_CONNECT_AFTER);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
    gtk_widget_show(subitem);
  }
  /* Version control (no parent) */
	if (!tsp_action->property.is_parent && tsp_action->property.parent_version_control && (tsp_action->property.directory_version_control || tsp_action->property.file_version_control))
	{
		subitem = gtk_menu_item_new_with_label (_("Delete"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_delete), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (file) */
	if (tsp_action->property.file_version_control) 
	{
		subitem = gtk_menu_item_new_with_label (_("Diff"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Diff");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Export"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_export), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Not here
	subitem = gtk_menu_item_new_with_label (_("Import"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Import");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
	gtk_widget_show(subitem);
*//* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Info"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Info");
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
		subitem = gtk_menu_item_new_with_label (_("Lock"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_lock), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Log"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_log), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
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
    subitem = gtk_menu_item_new_with_label (_("Move"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_move), action, G_CONNECT_AFTER);
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
		subitem = gtk_menu_item_new_with_label (_("Edit Properties"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_properties), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
/* Changed
	subitem = gtk_menu_item_new_with_label (_("Mark Resolved"));
*/if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Resolved"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_resolved), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}/*
*//* Version control (file) */
	if (tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Resolve"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Resolve");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Revert"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_revert), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Status"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_status), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (parent) */
	if (tsp_action->property.is_parent && tsp_action->property.parent_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Switch"));
		g_signal_connect_after (subitem, "activate", G_CALLBACK (tsp_action_unimplemented), "Switch");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Unlock"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_unlock), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}
	/* Version control (all) */
	if ((tsp_action->property.is_parent && tsp_action->property.parent_version_control) || tsp_action->property.directory_version_control || tsp_action->property.file_version_control)
	{
		subitem = gtk_menu_item_new_with_label (_("Update"));
		g_signal_connect_object (subitem, "activate", G_CALLBACK (tsp_action_update), action, G_CONNECT_AFTER);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), subitem);
		gtk_widget_show(subitem);
	}

	return item;
}



void tsp_action_unimplemented (GtkMenuItem *item, const gchar *action)
{
  GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), _(action));
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}



void tsp_action_add (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_checkout (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_cleanup (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_commit (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_copy (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_delete (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_export (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_import (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_lock (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_log (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_move (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_properties (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_resolved (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_revert (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_status (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_unlock (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}



void tsp_action_update (GtkMenuItem *item, TspSvnAction *action)
{
	guint size, i;
	gchar **argv;
	GList *iter;
	gchar *uri;
	gchar *filename;
	gchar *file;
	gint pid;
	GError *error = NULL;
	GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (action->window));

	iter = action->files;

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
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TSP_SVN_HELPER "\'");
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
	}

	g_strfreev (argv);
}

