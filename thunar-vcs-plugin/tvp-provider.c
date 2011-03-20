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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libxfce4util/libxfce4util.h>

#ifdef HAVE_SUBVERSION
#include <subversion-1/svn_types.h>
#include <thunar-vcs-plugin/tvp-svn-backend.h>
#include <thunar-vcs-plugin/tvp-svn-action.h>
#include <thunar-vcs-plugin/tvp-svn-property-page.h>
#endif

#ifdef HAVE_GIT
#include <thunar-vcs-plugin/tvp-git-action.h>
#endif

#include <thunar-vcs-plugin/tvp-provider.h>

/* use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) access((filename), (mode))
#endif


#define TVP_SVN_WORKING_COPY "tvp-svn-working-copy"



static void   tvp_provider_menu_provider_init   (ThunarxMenuProviderIface *iface);
static void   tvp_provider_property_page_provider_init (ThunarxPropertyPageProviderIface *iface);
static void   tvp_provider_finalize             (GObject                  *object);
static GList *tvp_provider_get_file_actions     (ThunarxMenuProvider      *menu_provider,
                                                 GtkWidget                *window,
                                                 GList                    *files);
static GList *tvp_provider_get_folder_actions   (ThunarxMenuProvider      *menu_provider,
                                                 GtkWidget                *window,
                                                 ThunarxFileInfo          *folder);
static GList *tvp_provider_get_pages            (ThunarxPropertyPageProvider *menu_provider,
                                                 GList                    *files);
static void   tvp_new_process                   (GtkAction                *action,
                                                 const GPid               *pid,
                                                 const gchar              *path,
                                                 TvpProvider              *tvp_provider);



typedef struct
{
  GPid pid;
  guint watch_id;
  gchar *path;
  TvpProvider *provider;
} TvpChildWatch;

struct _TvpProviderClass
{
  GObjectClass __parent__;
};

struct _TvpProvider
{
  GObject __parent__;

  TvpChildWatch *child_watch;

#if !GTK_CHECK_VERSION(2,9,0)
  /* GTK+ 2.9.0 and above provide an icon-name property
   * for GtkActions, so we don't need the icon factory.
   */
  GtkIconFactory *icon_factory;
#endif
};



//static GQuark tvp_action_files_quark;
//static GQuark tvp_action_provider_quark;



THUNARX_DEFINE_TYPE_WITH_CODE (TvpProvider,
                               tvp_provider,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_MENU_PROVIDER,
                                                            tvp_provider_menu_provider_init)
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
                                                            tvp_provider_property_page_provider_init));


static void
tvp_provider_class_init (TvpProviderClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the "tvp-action-files", "tvp-action-folder" and "tvp-action-provider" quarks */
  //tvp_action_files_quark = g_quark_from_string ("tvp-action-files");
  //tvp_action_provider_quark = g_quark_from_string ("tvp-action-provider");

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tvp_provider_finalize;
}



static void
tvp_provider_menu_provider_init (ThunarxMenuProviderIface *iface)
{
  iface->get_file_actions = tvp_provider_get_file_actions;
  iface->get_folder_actions = tvp_provider_get_folder_actions;
}



static void
tvp_provider_property_page_provider_init (ThunarxPropertyPageProviderIface *iface)
{
  iface->get_pages = tvp_provider_get_pages;
}



static void
tvp_provider_init (TvpProvider *tvp_provider)
{
#if !GTK_CHECK_VERSION(2,9,0)
  GtkIconSource *icon_source;
  GtkIconSet *icon_set;

  /* setup our icon factory */
  tvp_provider->icon_factory = gtk_icon_factory_new ();
  gtk_icon_factory_add_default (tvp_provider->icon_factory);

  /* add the "subversion" stock icon */
  icon_set = gtk_icon_set_new ();
  icon_source = gtk_icon_source_new ();
  gtk_icon_source_set_icon_name (icon_source, "subversion");
  gtk_icon_set_add_source (icon_set, icon_source);
  gtk_icon_factory_add (tvp_provider->icon_factory, "subversion", icon_set);
  gtk_icon_source_free (icon_source);
  gtk_icon_set_unref (icon_set);
#endif /* !GTK_CHECK_VERSION(2,9,0) */

#ifdef HAVE_SUBVERSION
  tvp_svn_backend_init();
#endif
}



static void
tvp_provider_finalize (GObject *object)
{
  TvpProvider *tvp_provider = TVP_PROVIDER (object);

  if (tvp_provider->child_watch)
  {
    GSource *source = g_main_context_find_source_by_id (NULL, tvp_provider->child_watch->watch_id);
    g_source_set_callback (source, (GSourceFunc) g_spawn_close_pid, NULL, NULL);
  }

#if !GTK_CHECK_VERSION(2,9,0)
  /* release our icon factory */
  gtk_icon_factory_remove_default (tvp_provider->icon_factory);
  g_object_unref (G_OBJECT (tvp_provider->icon_factory));
#endif

#ifdef HAVE_SUBVERSION
  tvp_svn_backend_free();
#endif


  (*G_OBJECT_CLASS (tvp_provider_parent_class)->finalize) (object);
}



#ifdef HAVE_SUBVERSION
static gboolean
tvp_is_working_copy (ThunarxFileInfo *file_info)
{
  gboolean result = FALSE;
  gchar   *filename;
  gchar   *uri;

  /* determine the URI for the file info */
  uri = thunarx_file_info_get_uri (file_info);
  if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
        {
          /* check if the folder is a working copy */
          result = tvp_svn_backend_is_working_copy (filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}
#endif



#ifdef HAVE_SUBVERSION
static gboolean
tvp_is_parent_working_copy (ThunarxFileInfo *file_info)
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
          result = tvp_svn_backend_is_working_copy (filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}
#endif



#ifdef HAVE_SUBVERSION
static GSList *
tvp_get_parent_status (ThunarxFileInfo *file_info)
{
  GSList *result = NULL;
  gchar  *filename;
  gchar  *uri;

  /* determine the parent URI for the file info */
  uri = thunarx_file_info_get_parent_uri (file_info);
  if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
        {
          /* check if the folder is a working copy */
          result = tvp_svn_backend_get_status (filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}
#endif



#ifdef HAVE_SUBVERSION
static gint
tvp_compare_filename (const gchar *uri1, const gchar *uri2)
{
  gchar *path1, *path2;
  gint result;

  /* strip the "file://" part of the uri */
  if (strncmp (uri1, "file://", 7) == 0)
  {
    uri1 += 7;
  }

  /* strip the "file://" part of the uri */
  if (strncmp (uri2, "file://", 7) == 0)
  {
    uri2 += 7;
  }

  path1 = g_strdup (uri1);
  path2 = g_strdup (uri2);

  /* remove trailing '/' */
  if (path1[strlen (path1) - 1] == '/')
  {
    path1[strlen (path1) - 1] = '\0';
  }

  /* remove trailing '/'*/
  if (path2[strlen (path2) - 1] == '/')
  {
    path2[strlen (path2) - 1] = '\0';
  }
  
  result = strcmp (path1, path2);

  g_free (path1);
  g_free (path2);

  return result;
}
#endif



#ifdef HAVE_SUBVERSION
static gint
tvp_compare_path (TvpSvnFileStatus *file_status, ThunarxFileInfo *file_info)
{
  gint   result = 1;
  gchar *filename;
  gchar *uri;

  /* determine the parent URI for the file info */
  uri = thunarx_file_info_get_uri (file_info);
  if (G_LIKELY (uri != NULL))
    {
      /* determine the local filename for the URI */
      filename = g_filename_from_uri (uri, NULL, NULL);
      if (G_LIKELY (filename != NULL))
        {
          /* check if the folder is a working copy */
          result = tvp_compare_filename (file_status->path, filename);

          /* release the filename */
          g_free (filename);
        }

      /* release the URI */
      g_free (uri);
    }

  return result;
}
#endif



static GList*
tvp_provider_get_file_actions (ThunarxMenuProvider *menu_provider,
                               GtkWidget           *window,
                               GList               *files)
{
  GList              *actions = NULL;
  GtkAction          *action;
  GList              *lp;
  gint               n_files = 0;
  gchar              *scheme;
#ifdef HAVE_SUBVERSION
  gboolean            parent_wc = FALSE;
  gboolean            directory_is_wc = FALSE;
  gboolean            directory_is_not_wc = FALSE;
  gboolean            file_is_vc = FALSE;
  gboolean            file_is_not_vc = FALSE;
  GSList             *file_status;
  GSList             *iter;
#endif
#ifdef HAVE_GIT
  gboolean            directory = FALSE;
  gboolean            file = FALSE;
#endif

#ifdef HAVE_SUBVERSION
  file_status = tvp_get_parent_status (files->data);

  /* check all supplied files */
  for (lp = files; lp != NULL; lp = lp->next, ++n_files)
  {
    /* check if the file is a local file */
    scheme = thunarx_file_info_get_uri_scheme (lp->data);

    /* unable to handle non-local files */
    if (G_UNLIKELY (strcmp (scheme, "file")))
    {
      g_free (scheme);
      return NULL;
    }
    g_free (scheme);

    /* check if the parent folder is a working copy */
    if (!parent_wc && tvp_is_parent_working_copy (lp->data))
      parent_wc = TRUE;

    if (thunarx_file_info_is_directory (lp->data))
    {
      if (tvp_is_working_copy (lp->data))
      {
        directory_is_wc = TRUE;
        //g_object_set_data(lp->data, TVP_SVN_WORKING_COPY, GINT_TO_POINTER(TRUE));
      }
      else
      {
        directory_is_not_wc = TRUE;
        //g_object_set_data(lp->data, TVP_SVN_WORKING_COPY, GINT_TO_POINTER(FALSE));
      }
    }
    else
    {
      for (iter = file_status; iter; iter = iter->next)
      {
        if (!tvp_compare_path (iter->data, lp->data))
        {
          if (TVP_SVN_FILE_STATUS (iter->data)->flag.version_control)
          {
            file_is_vc = TRUE;
          }
          else
          {
            file_is_not_vc = TRUE;
          }
          break;
        }
      }
      if(!iter)
        file_is_not_vc = TRUE;
    }
  }

  /* append the svn submenu action */
  action = tvp_svn_action_new ("Tvp::svn", _("SVN"), files, window, FALSE, parent_wc, directory_is_wc, directory_is_not_wc, file_is_vc, file_is_not_vc);
  g_signal_connect(action, "new-process", G_CALLBACK(tvp_new_process), menu_provider);
  actions = g_list_append (actions, action);
#endif

#ifdef HAVE_GIT
  /* check all supplied files */
  for (lp = files; lp != NULL; lp = lp->next, ++n_files)
  {
    /* check if the file is a local file */
    scheme = thunarx_file_info_get_uri_scheme (lp->data);

    /* unable to handle non-local files */
    if (G_UNLIKELY (strcmp (scheme, "file")))
    {
      g_free (scheme);
      return NULL;
    }
    g_free (scheme);

    if (thunarx_file_info_is_directory (lp->data))
    {
      directory = TRUE;
    }
    else
    {
      file = TRUE;
    }
  }

  /* append the git submenu action */
  action = tvp_git_action_new ("Tvp::git", _("GIT"), files, window, FALSE, directory, file);
  g_signal_connect(action, "new-process", G_CALLBACK(tvp_new_process), menu_provider);
  actions = g_list_append (actions, action);
#endif

  return actions;
}



static GList*
tvp_provider_get_folder_actions (ThunarxMenuProvider *menu_provider,
                                 GtkWidget           *window,
                                 ThunarxFileInfo     *folder)
{
  GtkAction          *action;
  GList              *actions = NULL;
  gchar              *scheme;
  GList              *files;

  /* check if the file is a local file */
  scheme = thunarx_file_info_get_uri_scheme (folder);

  /* unable to handle non-local files */
  if (G_UNLIKELY (strcmp (scheme, "file")))
  {
    g_free (scheme);
    return NULL;
  }
  g_free (scheme);

  files = g_list_append (NULL, folder);

#ifdef HAVE_SUBVERSION
  /* Lets see if we are dealing with a working copy */
  action = tvp_svn_action_new ("Tvp::svn", _("SVN"), files, window, TRUE, tvp_is_working_copy (folder), FALSE, FALSE, FALSE, FALSE);
  g_signal_connect(action, "new-process", G_CALLBACK(tvp_new_process), menu_provider);
  /* append the svn submenu action */
  actions = g_list_append (actions, action);
#endif

#ifdef HAVE_GIT
  action = tvp_git_action_new ("Tvp::git", _("GIT"), files, window, TRUE, TRUE, FALSE);
  g_signal_connect(action, "new-process", G_CALLBACK(tvp_new_process), menu_provider);
  /* append the git submenu action */
  actions = g_list_append (actions, action);
#endif

  g_list_free (files);

  return actions;
}



static GList*
tvp_provider_get_pages (ThunarxPropertyPageProvider *page_provider, GList *files)
{
  GList *pages = NULL;
#ifdef HAVE_SUBVERSION
  if (g_list_length (files) == 1)
  {
    gboolean            is_vc = FALSE;
    gchar              *scheme;

    /* check if the file is a local file */
    scheme = thunarx_file_info_get_uri_scheme (files->data);

    /* unable to handle non-local files */
    if (G_UNLIKELY (strcmp (scheme, "file")))
    {
      g_free (scheme);
      return NULL;
    }
    g_free (scheme);

    if (thunarx_file_info_is_directory (files->data))
    {
      /* Lets see if we are dealing with a working copy */
      if (tvp_is_working_copy (files->data))
      {
        is_vc = TRUE;
      }
    }
    else
    {
      GSList             *file_status;
      GSList             *iter;

      file_status = tvp_get_parent_status (files->data);

      for (iter = file_status; iter; iter = iter->next)
      {
        if (!tvp_compare_path (iter->data, files->data))
        {
          if (TVP_SVN_FILE_STATUS (iter->data)->flag.version_control)
          {
            is_vc = TRUE;
          }
          break;
        }
      }
    }
    if(is_vc)
    {
      pages = g_list_prepend (pages, tvp_svn_property_page_new (files->data));
    }
  }
#endif
  return pages;
}



static void
tvp_child_watch (GPid pid, gint status, gpointer data)
{
  /*
  gchar *watch_path = data;
  ThunarVfsPath *path;

  if (G_LIKELY (data))
  {
    GDK_THREADS_ENTER ();

    path = thunar_vfs_path_new (watch_path, NULL);

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
  */
  g_spawn_close_pid (pid);
}



static void
tvp_child_watch_free (TvpChildWatch *watch)
{
  if (watch->provider->child_watch == watch)
    watch->provider->child_watch = NULL;
  g_free(watch->path);
  g_free(watch);
}



static void
tvp_new_process (GtkAction *action, const GPid *pid, const gchar *path, TvpProvider *tvp_provider)
{
  TvpChildWatch *watch;
  if (tvp_provider->child_watch)
  {
    GSource *source = g_main_context_find_source_by_id (NULL, tvp_provider->child_watch->watch_id);
    g_source_set_callback (source, (GSourceFunc) g_spawn_close_pid, NULL, NULL);
  }
  watch = g_new(TvpChildWatch, 1);
  watch->pid = *pid;
  watch->path = g_strdup (path);
  watch->provider = tvp_provider;
  watch->watch_id = g_child_watch_add_full (G_PRIORITY_LOW, *pid, tvp_child_watch, watch, (GDestroyNotify)tvp_child_watch_free);
  tvp_provider->child_watch = watch;
}

