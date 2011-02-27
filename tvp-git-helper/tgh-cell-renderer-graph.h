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

#ifndef __TGH_CELL_RENDERER_GRAPH_H__
#define __TGH_CELL_RENDERER_GRAPH_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TghCellRendererGraphClass TghCellRendererGraphClass;
typedef struct _TghCellRendererGraph      TghCellRendererGraph;
  
#define TGH_TYPE_CELL_RENDERER_GRAPH            (tgh_cell_renderer_graph_get_type ())
#define TGH_CELL_RENDERER_GRAPH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TGH_TYPE_CELL_RENDERER_GRAPH, TghCellRendererGraph))
#define TGH_CELL_RENDERER_GRAPH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TGH_TYPE_CELL_RENDERER_GRAPH, TghCellRendererGraphClass))
#define TGH_IS_CELL_RENDERER_GRAPH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TGH_TYPE_CELL_RENDERER_GRAPH))
#define TGH_IS_CELL_RENDERER_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TGH_TYPE_CELL_RENDERER_GRAPH))
#define TGH_CELL_RENDERER_GRAPH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TGH_TYPE_CELL_RENDERER_GRAPH, TghCellRendererGraphClass))

typedef struct _TghGraphNode TghGraphNode;

struct  _TghGraphNode
{
  TghGraphNode *next;
  enum {TGH_GRAPH_LINE, TGH_GRAPH_JUNCTION} type;
  gchar *name;
  gchar **junction;
};

GType               tgh_cell_renderer_graph_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkCellRenderer*    tgh_cell_renderer_graph_new      (void) G_GNUC_MALLOC G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__TGH_CELL_RENDERER_GRAPH_H__ */
