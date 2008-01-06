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

#include <glib.h>

#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_fs.h>

#include <thunar-svn-plugin/tsp-svn-backend.h>



static apr_pool_t *pool = NULL;
static svn_client_ctx_t *ctx = NULL;


gboolean
tsp_svn_backend_init ()
{
	if (pool)
		return TRUE;

	svn_error_t *err;

	if (svn_cmdline_init (NULL, NULL) == EXIT_FAILURE)
		return FALSE;

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
tsp_svn_backend_free ()
{
	if (pool)
    svn_pool_destroy (pool);
}



gboolean
tsp_svn_backend_is_working_copy (const gchar *uri)
{
  apr_pool_t *subpool;
	svn_error_t *err;
	int wc_format;

	/* strip the "file://" part of the uri */
	if (strncmp (uri, "file://", 7) == 0)
	{
		uri += 7;
	}

	gchar *path = g_strdup (uri);

	/* remove trailing '/' cause svn_wc_check_wc can't handle that */
	if (path[strlen (path) - 1] == '/')
	{
		path[strlen (path) - 1] = '\0';
	}

  subpool = svn_pool_create (pool);

	/* check for the path is a working copy */
	err = svn_wc_check_wc (path, &wc_format, subpool);

  svn_pool_destroy (subpool);

	g_free (path);

	/* if an error occured or wc_format in not set it is no working copy */
	if(err || !wc_format)
	{
    svn_error_clear (err);
		return FALSE;
	}
	
	return TRUE;
}



static void
status_callback (void *baton, const char *path, svn_wc_status2_t *status)
{
	GSList **list = baton;
	TspSvnFileStatus *entry = g_new (TspSvnFileStatus, 1);
	
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
}



GSList *
tsp_svn_backend_get_status (const gchar *uri)
{
  apr_pool_t *subpool;
	svn_error_t *err;
	svn_opt_revision_t revision = {svn_opt_revision_working};
	GSList *list = NULL;

	/* strip the "file://" part of the uri */
	if (strncmp (uri, "file://", 7) == 0)
	{
		uri += 7;
	}

	gchar *path = g_strdup (uri);

	/* remove trailing '/' cause svn_client_status2 can't handle that */
	if (path[strlen (path) - 1] == '/')
	{
		path[strlen (path) - 1] = '\0';
	}

  subpool = svn_pool_create (pool);

	/* check for the path is a working copy */
	err = svn_client_status2 (NULL, path, &revision, status_callback, &list, FALSE, TRUE, FALSE, TRUE, TRUE, ctx, subpool);

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



static svn_error_t *
info_callback (void *baton, const char *path, const svn_info_t *info, apr_pool_t *pool)
{
  TspSvnInfo **pinfo = baton;
  g_return_val_if_fail (*pinfo == NULL, SVN_NO_ERROR);

  *pinfo = g_new (TspSvnInfo, 1);
  (*pinfo)->path = g_strdup (path);
  (*pinfo)->url = g_strdup (info->URL);
  (*pinfo)->revision = info->rev;
  (*pinfo)->repository = g_strdup (info->repos_root_URL);
  (*pinfo)->modrev = info->last_changed_rev;
  apr_ctime (((*pinfo)->moddate = g_new0(gchar, APR_CTIME_LEN)), info->last_changed_date);
  (*pinfo)->modauthor = g_strdup (info->last_changed_author);

  return SVN_NO_ERROR;
}



TspSvnInfo *
tsp_svn_backend_get_info (const gchar *uri)
{
  apr_pool_t *subpool;
	svn_error_t *err;
	svn_opt_revision_t revision = {svn_opt_revision_unspecified};
  TspSvnInfo *info = NULL;

	/* strip the "file://" part of the uri */
	if (strncmp (uri, "file://", 7) == 0)
	{
		uri += 7;
	}

	gchar *path = g_strdup (uri);

	/* remove trailing '/' cause svn_client_status2 can't handle that */
	if (path[strlen (path) - 1] == '/')
	{
		path[strlen (path) - 1] = '\0';
	}

  subpool = svn_pool_create (pool);

	/* check for the path is a working copy */
	err = svn_client_info (path, &revision, &revision, info_callback, &info, FALSE, ctx, subpool);

  svn_pool_destroy (subpool);

	g_free (path);

	if (err)
	{
    tsp_svn_info_free (info);
    svn_error_clear (err);
		return NULL;
	}

	return info;
}



void
tsp_svn_info_free (TspSvnInfo *info)
{
  if (!info)
    return;

  g_free (info->path);
  g_free (info->url);
  g_free (info->repository);
  g_free (info->moddate);
  g_free (info->modauthor);

  g_free (info);
}

