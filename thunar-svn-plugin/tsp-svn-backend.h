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

#ifndef __TSP_SVN_BACKEND_H__
#define __TSP_SVN_BACKEND_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct
{
	gchar *path;
	struct {
		unsigned version_control : 1;
	} flag;
} TspSvnFileStatus;

gboolean tsp_svn_backend_init();

gboolean tsp_svn_backend_is_working_copy (const gchar *uri);

GSList  *tsp_svn_backend_get_status (const gchar *uri);

G_END_DECLS;

#endif /* !__TSP_SVN_BACKEND_H__ */
