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

#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_fs.h>

#include <thunar-svn-plugin/tsp-svn-backend.h>



static apr_pool_t *pool = NULL;


gboolean
tsp_svn_backend_init()
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
		return FALSE;

	/* Make sure the ~/.subversion run-time config files exist */
	err = svn_config_ensure (NULL, pool);
	if(err)
		return FALSE;

#ifdef G_OS_WIN32
	/* Set the working copy administrative directory name */
	if (getenv ("SVN_ASP_DOT_NET_HACK"))
	{
		err = svn_wc_set_adm_dir ("_svn", pool);
		if(err)
			return FALSE;
	}
#endif

	/* We are ready now */

	return TRUE;
}



gboolean
tsp_svn_backend_is_working_copy (const gchar *uri)
{
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

	/* check for the path is a working copy */
	err = svn_wc_check_wc (path, &wc_format, pool);

	g_free (path);

	/* if an error occured or wc_format in not set it is no working copy */
	if(err || !wc_format)
	{
		return FALSE;
	}
	
	return TRUE;
}

