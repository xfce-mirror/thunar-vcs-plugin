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

#ifndef __TSP_SVN_ACTION_H__
#define __TSP_SVN_ACTION_H__

#include <gtk/gtk.h>
#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _TspSvnActionClass TspSvnActionClass;
typedef struct _TspSvnAction      TspSvnAction;

#define TSP_TYPE_SVN_ACTION             (tsp_svn_action_get_type ())
#define TSP_SVN_ACTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSP_TYPE_SVN_ACTION, TspSvnAction))
#define TSP_SVN_ACTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSP_TYPE_SVN_ACTION, TspSvnActionClass))
#define TSP_IS_SVN_ACTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSP_TYPE_SVN_ACTION))
#define TSP_IS_SVN_ACTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSP_TYPE_SVN_ACTION))
#define TSP_SVN_ACTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSP_TYPE_SVN_ACTION, TspSvnActionClass))

GType      tsp_svn_action_get_type      (void) G_GNUC_CONST G_GNUC_INTERNAL;
void       tsp_svn_action_register_type (ThunarxProviderPlugin *) G_GNUC_INTERNAL;

GtkAction *tsp_svn_action_new           (const gchar*,
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

#endif /* !__TSP_SVN_ACTION_H__ */
