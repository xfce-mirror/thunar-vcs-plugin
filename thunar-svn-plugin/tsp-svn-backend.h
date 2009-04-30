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

G_BEGIN_DECLS;

typedef struct
{
	gchar *path;
	struct {
		unsigned version_control : 1;
	} flag;
} TspSvnFileStatus;

#define TSP_SVN_FILE_STATUS(p) ((TspSvnFileStatus*)p)

typedef struct
{
	gchar *path;
  gchar *url;
  svn_revnum_t revision;
  gchar *repository;
  svn_revnum_t modrev;
  gchar *moddate;
  gchar *modauthor;
  gboolean has_wc_info;
  gchar *changelist;
  svn_depth_t depth;
} TspSvnInfo;

#define TSP_SVN_INFO(p) ((TspSvnInfo*)p)

gboolean tsp_svn_backend_init();
void     tsp_svn_backend_free();

gboolean tsp_svn_backend_is_working_copy (const gchar *uri);

GSList  *tsp_svn_backend_get_status (const gchar *uri);

TspSvnInfo *tsp_svn_backend_get_info (const gchar *uri);

void     tsp_svn_info_free (TspSvnInfo *info);

#define CHECK_SVN_VERSION(major, minor) ((major == SVN_VER_MAJOR) && (minor == SVN_VER_MINOR))

G_END_DECLS;

#endif /* !__TSP_SVN_BACKEND_H__ */
