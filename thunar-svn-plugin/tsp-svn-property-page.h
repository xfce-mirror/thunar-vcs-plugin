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

#ifndef __TSP_SVN_PROPERTY_PAGE_H__
#define __TSP_SVN_PROPERTY_PAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TspSvnPropertyPageClass TspSvnPropertyPageClass;
typedef struct _TspSvnPropertyPage      TspSvnPropertyPage;

#define TSP_TYPE_SVN_PROPERTY_PAGE             (tsp_svn_property_page_get_type ())
#define TSP_SVN_PROPERTY_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSP_TYPE_SVN_PROPERTY_PAGE, TspSvnPropertyPage))
#define TSP_SVN_PROPERTY_PAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSP_TYPE_SVN_PROPERTY_PAGE, TspSvnPropertyPageClass))
#define TSP_IS_SVN_PROPERTY_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSP_TYPE_SVN_PROPERTY_PAGE))
#define TSP_IS_SVN_PROPERTY_PAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSP_TYPE_SVN_PROPERTY_PAGE))
#define TSP_SVN_PROPERTY_PAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSP_TYPE_SVN_PROPERTY_PAGE, TspSvnPropertyPageClass))

GType      tsp_svn_property_page_get_type  (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkAction *tsp_svn_property_page_new       (ThunarxFileInfo *) G_GNUC_MALLOC G_GNUC_INTERNAL;

ThunarxFileInfo *tsp_svn_property_page_get_file (TspSvnPropertyPage *) G_GNUC_INTERNAL;
void             tsp_svn_property_page_set_file (TspSvnPropertyPage *, ThunarxFileInfo *) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__TSP_SVN_PROPERTY_PAGE_H__ */
