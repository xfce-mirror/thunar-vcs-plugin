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

#ifndef __TVP_SVN_ACTION_H__
#define __TVP_SVN_ACTION_H__

#include <gtk/gtk.h>
#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _TvpSvnActionClass TvpSvnActionClass;
typedef struct _TvpSvnAction      TvpSvnAction;

#define TVP_TYPE_SVN_ACTION             (tvp_svn_action_get_type ())
#define TVP_SVN_ACTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TVP_TYPE_SVN_ACTION, TvpSvnAction))
#define TVP_SVN_ACTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TVP_TYPE_SVN_ACTION, TvpSvnActionClass))
#define TVP_IS_SVN_ACTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TVP_TYPE_SVN_ACTION))
#define TVP_IS_SVN_ACTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TVP_TYPE_SVN_ACTION))
#define TVP_SVN_ACTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TVP_TYPE_SVN_ACTION, TvpSvnActionClass))

GType      tvp_svn_action_get_type      (void) G_GNUC_CONST G_GNUC_INTERNAL;
void       tvp_svn_action_register_type (ThunarxProviderPlugin *) G_GNUC_INTERNAL;

GtkAction *tvp_svn_action_new           (const gchar*,
                                         const gchar*,
                                         GList *,
                                         GtkWidget *,
                                         gboolean,
                                         gboolean,
                                         gboolean,
                                         gboolean,
                                         gboolean,
                                         gboolean) G_GNUC_MALLOC G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__TVP_SVN_ACTION_H__ */
