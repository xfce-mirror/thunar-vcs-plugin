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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-svn-plugin/tsp-svn-backend.h>
#include <thunar-svn-plugin/tsp-svn-action.h>
#include <thunar-svn-plugin/tsp-provider.h>

/* use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) access((filename), (mode))
#endif


#define TSP_SVN_WORKING_COPY "tsp-svn-working-copy"



static void   tsp_provider_class_init           (TspProviderClass         *klass);
static void   tsp_provider_menu_provider_init   (ThunarxMenuProviderIface *iface);
static void   tsp_provider_init                 (TspProvider              *tsp_provider);
static void   tsp_provider_finalize             (GObject                  *object);
static GList *tsp_provider_get_file_actions     (ThunarxMenuProvider      *menu_provider,
                                                 GtkWidget                *window,
                                                 GList                    *files);
static GList *tsp_provider_get_folder_actions   (ThunarxMenuProvider      *menu_provider,
                                                 GtkWidget                *window,
                                                 ThunarxFileInfo          *folder);



struct _TspProviderClass
{
  GObjectClass __parent__;
};

struct _TspProvider
{
  GObject         __parent__;

#if !GTK_CHECK_VERSION(2,9,0)
  /* GTK+ 2.9.0 and above provide an icon-name property
   * for GtkActions, so we don't need the icon factory.
   */
  GtkIconFactory *icon_factory;
#endif

  /* child watch support for the last spawn command,
   * which allows us to refresh the folder contents
   * after the command terminates (i.e. files are
   * added).
   */
  gchar          *child_watch_path;
  gint            child_watch_id;
};



static GQuark tsp_action_files_quark;
static GQuark tsp_action_provider_quark;



THUNARX_DEFINE_TYPE_WITH_CODE (TspProvider,
                               tsp_provider,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_MENU_PROVIDER,
                                                            tsp_provider_menu_provider_init));


static void
tsp_provider_class_init (TspProviderClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the "tsp-action-files", "tsp-action-folder" and "tsp-action-provider" quarks */
  tsp_action_files_quark = g_quark_from_string ("tsp-action-files");
  tsp_action_provider_quark = g_quark_from_string ("tsp-action-provider");

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tsp_provider_finalize;
}



static void
tsp_provider_menu_provider_init (ThunarxMenuProviderIface *iface)
{
  iface->get_file_actions = tsp_provider_get_file_actions;
  iface->get_folder_actions = tsp_provider_get_folder_actions;
}



static void
tsp_provider_init (TspProvider *tsp_provider)
{
	tsp_svn_backend_init();
}



static void
tsp_provider_finalize (GObject *object)
{
  TspProvider *tsp_provider = TSP_PROVIDER (object);
  GSource     *source;

  /* give up maintaince of any pending child watch */
  if (G_UNLIKELY (tsp_provider->child_watch_id != 0))
    {
      /* reset the callback function to g_spawn_close_pid() so the plugin can be
       * safely unloaded and the child will still not become a zombie afterwards.
       * This also resets the child_watch_id and child_watch_path properties.
       */
      source = g_main_context_find_source_by_id (NULL, tsp_provider->child_watch_id);
      g_source_set_callback (source, (GSourceFunc) g_spawn_close_pid, NULL, NULL);
    }
  
#if !GTK_CHECK_VERSION(2,9,0)
  /* release our icon factory */
  gtk_icon_factory_remove_default (tsp_provider->icon_factory);
  g_object_unref (G_OBJECT (tsp_provider->icon_factory));
#endif

  (*G_OBJECT_CLASS (tsp_provider_parent_class)->finalize) (object);
}



static gboolean
tsp_is_working_copy (ThunarxFileInfo *file_info)
{
  gboolean result = FALSE;
  gchar   *filename;
  gchar   *uri;

  /* determine the parent URI for the file info */
  uri = thunarx_file_info_get_uri (file_info);
  if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
        {
          /* check if the folder is a working copy */
          result = tsp_svn_backend_is_working_copy (filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}



static gboolean
tsp_is_parent_working_copy (ThunarxFileInfo *file_info)
{
  gboolean result = FALSE;
  gchar   *filename;
  gchar   *uri;

  /* determine the parent URI for the file info */
  uri = thunarx_file_info_get_parent_uri (file_info);
  if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
        {
          /* check if the folder is a working copy */
          result = tsp_svn_backend_is_working_copy (filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}



static GList*
tsp_provider_get_file_actions (ThunarxMenuProvider *menu_provider,
                               GtkWidget           *window,
                               GList               *files)
{
  ThunarVfsPathScheme scheme;
  ThunarVfsInfo      *info;
  gboolean            parent_wc = FALSE;
	gboolean            one_is_wc = FALSE;
	gboolean            one_is_not_wc = FALSE;
  GtkAction          *action;
  GList              *actions = NULL;
  GList              *lp;
  gint                n_files = 0;

  /* check all supplied files */
  for (lp = files; lp != NULL; lp = lp->next, ++n_files)
	{
		/* check if the file is a local file */
		info = thunarx_file_info_get_vfs_info (lp->data);
		scheme = thunar_vfs_path_get_scheme (info->path);
		thunar_vfs_info_unref (info);

		/* unable to handle non-local files */
		if (G_UNLIKELY (scheme != THUNAR_VFS_PATH_SCHEME_FILE))
			return NULL;

		/* check if the parent folder is a working copy */
		if (!parent_wc && tsp_is_parent_working_copy (lp->data))
			parent_wc = TRUE;

		if (tsp_is_working_copy (lp->data))
		{
			one_is_wc = TRUE;
			g_object_set_data(lp->data, TSP_SVN_WORKING_COPY, GINT_TO_POINTER(TRUE));
		}
		else
		{
			one_is_not_wc = TRUE;
			g_object_set_data(lp->data, TSP_SVN_WORKING_COPY, GINT_TO_POINTER(FALSE));
		}
	}

	/* is the parent folder a working copy */
	if (!parent_wc && one_is_not_wc)
	{
		/* It's not a working copy
		 * append the "Import" action */
		action = g_object_new (GTK_TYPE_ACTION,
													 "name", "Tsp::import",
													 "label", _("SVN _Import"),
													 NULL);
		actions = g_list_append (actions, action);
	}
	if (parent_wc || one_is_wc)
	{
		/* append the svn submenu action */
		action = tsp_svn_action_new ("Tsp::svn", _("SVN"));
		actions = g_list_append (actions, action);
	}

  return actions;
}



static GList*
tsp_provider_get_folder_actions (ThunarxMenuProvider *menu_provider,
                                 GtkWidget           *window,
                                 ThunarxFileInfo     *folder)
{
  GtkAction          *action;
  GList              *actions = NULL;
  ThunarVfsPathScheme scheme;
  ThunarVfsInfo      *info;


	/* check if the file is a local file */
	info = thunarx_file_info_get_vfs_info (folder);
	scheme = thunar_vfs_path_get_scheme (info->path);
	thunar_vfs_info_unref (info);

	/* unable to handle non-local files */
	if (G_UNLIKELY (scheme != THUNAR_VFS_PATH_SCHEME_FILE))
		return NULL;

	/* Lets see if we are dealing with a working copy */
	if (tsp_is_working_copy (folder))
	{
		/* append the svn submenu action */
		action = tsp_svn_action_new ("Tsp::svn", _("SVN"));
		actions = g_list_append (actions, action);
	}
	else
	{
		/* It's not a working copy
		 * append the "Checkout" action */
		action = g_object_new (GTK_TYPE_ACTION,
													 "name", "Tsp::checkout",
													 "label", _("SVN _Checkout"),
													 NULL);
		actions = g_list_append (actions, action);
	}

  return actions;
}

