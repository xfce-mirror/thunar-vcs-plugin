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

#ifndef __TVP_SVN_PROPERTY_PAGE_H__
#define __TVP_SVN_PROPERTY_PAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TvpSvnPropertyPageClass TvpSvnPropertyPageClass;
typedef struct _TvpSvnPropertyPage      TvpSvnPropertyPage;

#define TVP_TYPE_SVN_PROPERTY_PAGE             (tvp_svn_property_page_get_type ())
#define TVP_SVN_PROPERTY_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TVP_TYPE_SVN_PROPERTY_PAGE, TvpSvnPropertyPage))
#define TVP_SVN_PROPERTY_PAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TVP_TYPE_SVN_PROPERTY_PAGE, TvpSvnPropertyPageClass))
#define TVP_IS_SVN_PROPERTY_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TVP_TYPE_SVN_PROPERTY_PAGE))
#define TVP_IS_SVN_PROPERTY_PAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TVP_TYPE_SVN_PROPERTY_PAGE))
#define TVP_SVN_PROPERTY_PAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TVP_TYPE_SVN_PROPERTY_PAGE, TvpSvnPropertyPageClass))

GType      tvp_svn_property_page_get_type       (void) G_GNUC_CONST G_GNUC_INTERNAL;
void       tvp_svn_property_page_register_type  (ThunarxProviderPlugin *) G_GNUC_INTERNAL;

GtkAction *tvp_svn_property_page_new            (ThunarxFileInfo *) G_GNUC_MALLOC G_GNUC_INTERNAL;

ThunarxFileInfo *tvp_svn_property_page_get_file (TvpSvnPropertyPage *) G_GNUC_INTERNAL;
void             tvp_svn_property_page_set_file (TvpSvnPropertyPage *, ThunarxFileInfo *) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__TVP_SVN_PROPERTY_PAGE_H__ */
