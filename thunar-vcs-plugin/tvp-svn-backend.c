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

#include <glib.h>

#include <subversion-1/svn_version.h>
#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_fs.h>
#include <subversion-1/svn_dso.h>

#include <thunar-vcs-plugin/tvp-svn-backend.h>



static apr_pool_t *pool = NULL;
static svn_client_ctx_t *ctx = NULL;


gboolean
tvp_svn_backend_init (void)
{
	svn_error_t *err;

	if (pool)
		return TRUE;

    /* Initialize apr */
    if (apr_initialize())
        return FALSE;

    /* Initialize the DSO library, this must be done before svn_pool_create */
#if CHECK_SVN_VERSION(1,5)
    svn_dso_initialize ();
#else /* CHECK_SVN_VERSION(1,6) */
    err = svn_dso_initialize2 ();
	if(err)
  {
    svn_error_clear (err);
		return FALSE;
  }
#endif

	/* Create top-level memory pool */
	pool = svn_pool_create (NULL);

	/* Initialize the FS library */
	err = svn_fs_initialize (pool);
	if(err)
  {
    svn_error_clear (err);
		return FALSE;
  }

	/* Make sure the ~/.subversion run-time config files exist */
	err = svn_config_ensure (NULL, pool);
	if(err)
  {
    svn_error_clear (err);
		return FALSE;
  }

#ifdef G_OS_WIN32
	/* Set the working copy administrative directory name */
	if (getenv ("SVN_ASP_DOT_NET_HACK"))
	{
		err = svn_wc_set_adm_dir ("_svn", pool);
    if(err)
    {
      svn_error_clear (err);
      return FALSE;
    }
	}
#endif

	err = svn_client_create_context (&ctx, pool);
  if(err)
  {
    svn_error_clear (err);
    return FALSE;
  }

	err = svn_config_get_config (&(ctx->config), NULL, pool);
  if(err)
  {
    svn_error_clear (err);
    return FALSE;
  }

	/* We are ready now */

	return TRUE;
}



void
tvp_svn_backend_free (void)
{
	if (pool)
    {
    svn_pool_destroy (pool);
    apr_terminate ();
    }
    pool = NULL;
}



gboolean
tvp_svn_backend_is_working_copy (const gchar *uri)
{
  apr_pool_t *subpool;
  svn_error_t *err;
  int wc_format;
  gchar *path;
#if CHECK_SVN_VERSION_G(1,7)
  svn_wc_context_t *wc_ctx;
#endif

  /* strip the "file://" part of the uri */
  if (strncmp (uri, "file://", 7) == 0)
  {
    uri += 7;
  }

  path = g_strdup (uri);

  /* remove trailing '/' cause svn_wc_check_wc can't handle that */
  if (strlen (path) > 1 && path[strlen (path) - 1] == '/')
  {
    path[strlen (path) - 1] = '\0';
  }

  subpool = svn_pool_create (pool);

#if CHECK_SVN_VERSION(1,5) || CHECK_SVN_VERSION(1,6)
  /* check for the path is a working copy */
  err = svn_wc_check_wc (path, &wc_format, subpool);
#else /* CHECK_SVN_VERSION(1,7) */
  err = svn_wc_context_create (&wc_ctx, NULL, subpool, subpool);
  if (!err)
  {
    err = svn_wc_check_wc2 (&wc_format, wc_ctx, path, subpool);
  }
#endif

  svn_pool_destroy (subpool);

  g_free (path);

  /* if an error occured or wc_format in not set it is no working copy */
  if (err || !wc_format)
  {
    svn_error_clear (err);
    return FALSE;
  }

  return TRUE;
}



#if CHECK_SVN_VERSION(1,5)
static void
status_callback2 (void *baton, const char *path, svn_wc_status2_t *status)
#elif CHECK_SVN_VERSION(1,6)
static svn_error_t *
status_callback3 (void *baton, const char *path, svn_wc_status2_t *status, apr_pool_t *pool_)
#else /* CHECK_SVN_VERSION(1,7) */
static svn_error_t *
status_callback (void *baton, const char *path, const svn_client_status_t *status, apr_pool_t *pool_)
#endif
{
  GSList **list = baton;
  TvpSvnFileStatus *entry = g_new (TvpSvnFileStatus, 1);

  entry->path = g_strdup (path);
  switch (status->text_status)
  {
    case svn_wc_status_normal:
    case svn_wc_status_added:
    case svn_wc_status_missing:
    case svn_wc_status_deleted:
    case svn_wc_status_replaced:
    case svn_wc_status_modified:
    case svn_wc_status_merged:
    case svn_wc_status_conflicted:
    case svn_wc_status_incomplete:
      entry->flag.version_control = 1;
      break;
    default:
      entry->flag.version_control = 0;
      break;
  }

  *list = g_slist_prepend (*list, entry);
#if CHECK_SVN_VERSION_G(1,6)
  return SVN_NO_ERROR;
#endif
}




GSList *
tvp_svn_backend_get_status (const gchar *uri)
{
  apr_pool_t *subpool;
  svn_error_t *err;
  svn_opt_revision_t revision = {svn_opt_revision_working};
  GSList *list = NULL;
  gchar *path;

  /* strip the "file://" part of the uri */
  if (strncmp (uri, "file://", 7) == 0)
  {
    uri += 7;
  }

  path = g_strdup (uri);

  /* remove trailing '/' cause svn_client_status2 can't handle that */
  if (strlen (path) > 1 && path[strlen (path) - 1] == '/')
  {
    path[strlen (path) - 1] = '\0';
  }

  subpool = svn_pool_create (pool);

  /* get the status of all files in the directory */
#if CHECK_SVN_VERSION(1,5)
  err = svn_client_status3 (NULL, path, &revision, status_callback2, &list, svn_depth_immediates, TRUE, FALSE, TRUE, TRUE, NULL, ctx, subpool);
#elif CHECK_SVN_VERSION(1,6)
  err = svn_client_status4 (NULL, path, &revision, status_callback3, &list, svn_depth_immediates, TRUE, FALSE, TRUE, TRUE, NULL, ctx, subpool);
#else /* CHECK_SVN_VERSION(1,7) */
  err = svn_client_status5 (NULL, ctx, path, &revision, svn_depth_immediates, TRUE, FALSE, TRUE, TRUE, TRUE, NULL, status_callback, &list, subpool);
#endif

  svn_pool_destroy (subpool);

  g_free (path);

  if (err)
  {
    GSList *iter;
    for (iter = list; iter; iter = iter->next)
    {
      g_free (iter->data);
    }
    g_slist_free (list);
    svn_error_clear (err);
    return NULL;
  }

  return list;
}



#if CHECK_SVN_VERSION(1,5) || CHECK_SVN_VERSION(1,6)
static svn_error_t *
info_callback (void *baton, const char *path, const svn_info_t *info, apr_pool_t *pool_)
#else /* CHECK_SVN_VERSION(1,7) */
static svn_error_t *
info_callback (void *baton, const char *path, const svn_client_info2_t *info, apr_pool_t *pool_)
#endif
{
  TvpSvnInfo **pinfo = baton;
  g_return_val_if_fail (*pinfo == NULL, SVN_NO_ERROR);

  *pinfo = g_new0 (TvpSvnInfo, 1);
  (*pinfo)->path = g_strdup (path);
  (*pinfo)->url = g_strdup (info->URL);
  (*pinfo)->revision = info->rev;
  (*pinfo)->repository = g_strdup (info->repos_root_URL);
  (*pinfo)->modrev = info->last_changed_rev;
  apr_ctime (((*pinfo)->moddate = g_new0(gchar, APR_CTIME_LEN)), info->last_changed_date);
  (*pinfo)->modauthor = g_strdup (info->last_changed_author);
#if CHECK_SVN_VERSION(1,5) || CHECK_SVN_VERSION(1,6)
  if (((*pinfo)->has_wc_info = info->has_wc_info))
  {
    (*pinfo)->changelist = g_strdup (info->changelist);
    (*pinfo)->depth = info->depth;
  }
#else /* CHECK_SVN_VERSION(1,7) */
  if (info->wc_info)
  {
    (*pinfo)->has_wc_info = TRUE;
    (*pinfo)->changelist = g_strdup (info->wc_info->changelist);
    (*pinfo)->depth = info->wc_info->depth;
  }
  else
  {
    (*pinfo)->has_wc_info = FALSE;
  }
#endif

  return SVN_NO_ERROR;
}



TvpSvnInfo *
tvp_svn_backend_get_info (const gchar *uri)
{
  apr_pool_t *subpool;
  svn_error_t *err;
  svn_opt_revision_t revision = {svn_opt_revision_unspecified};
  TvpSvnInfo *info = NULL;
  gchar *path;

  /* strip the "file://" part of the uri */
  if (strncmp (uri, "file://", 7) == 0)
  {
    uri += 7;
  }

  path = g_strdup (uri);

  /* remove trailing '/' cause svn_client_info can't handle that */
  if (strlen (path) > 1 && path[strlen (path) - 1] == '/')
  {
    path[strlen (path) - 1] = '\0';
  }

  subpool = svn_pool_create (pool);

#if CHECK_SVN_VERSION(1,5) || CHECK_SVN_VERSION(1,6)
  /* get svn info for this file or directory */
  err = svn_client_info2 (path, &revision, &revision, info_callback, &info, svn_depth_empty, NULL, ctx, subpool);
#else /* CHECK_SVN_VERSION(1,7) */
  /* get svn info for this file or directory */
  err = svn_client_info3 (path, &revision, &revision, svn_depth_empty, FALSE, TRUE, NULL, info_callback, &info, ctx, subpool);
#endif

  svn_pool_destroy (subpool);

  g_free (path);

  if (err)
  {
    tvp_svn_info_free (info);
    svn_error_clear (err);
    return NULL;
  }

  return info;
}



void
tvp_svn_info_free (TvpSvnInfo *info)
{
  if (!info)
    return;

  g_free (info->path);
  g_free (info->url);
  g_free (info->repository);
  g_free (info->moddate);
  g_free (info->modauthor);
  if(info->has_wc_info)
  {
    g_free (info->changelist);
  }

  g_free (info);
}

