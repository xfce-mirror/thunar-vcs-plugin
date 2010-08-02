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

#include <thunar-vfs/thunar-vfs.h>
#include <gtk/gtk.h>

#include "tgh-common.h"
#include "tgh-cell-renderer-graph.h"

struct _TghCellRendererGraph
{
  GtkCellRenderer cell;

  GList *graph_iter;
  guint junction_size;
  guint spacing;
};

struct _TghCellRendererGraphClass
{
  GtkCellRendererClass cell_class;
};

enum {
  PROPERTY_GRAPH_ITER = 1
};

static void tgh_cell_renderer_graph_get_property (GObject*, guint, GValue*, GParamSpec*);
static void tgh_cell_renderer_graph_set_property (GObject*, guint, const GValue*, GParamSpec*);

static void tgh_cell_renderer_graph_get_size (GtkCellRenderer*, GtkWidget*, GdkRectangle*, gint*, gint*, gint*, gint*);
static void tgh_cell_renderer_graph_render (GtkCellRenderer*, GdkDrawable*, GtkWidget*, GdkRectangle*, GdkRectangle*, GdkRectangle*, GtkCellRendererState);

G_DEFINE_TYPE (TghCellRendererGraph, tgh_cell_renderer_graph, GTK_TYPE_CELL_RENDERER)

static void
tgh_cell_renderer_graph_class_init (TghCellRendererGraphClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

  object_class->get_property = tgh_cell_renderer_graph_get_property;
  object_class->set_property = tgh_cell_renderer_graph_set_property;

  cell_class->get_size = tgh_cell_renderer_graph_get_size;
  cell_class->render = tgh_cell_renderer_graph_render;

  g_object_class_install_property (object_class, PROPERTY_GRAPH_ITER,
      g_param_spec_pointer ("graph-iter", "", "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROPERTY_GRAPH_ITER,
      g_param_spec_uint ("junction-size", "", "", 1, G_MAXUINT, 5, G_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROPERTY_GRAPH_ITER,
      g_param_spec_uint ("spacing", "", "", 1, G_MAXUINT, 4, G_PARAM_READWRITE));
}

static void
tgh_cell_renderer_graph_init (TghCellRendererGraph *renderer)
{
  renderer->junction_size = 5;
  renderer->spacing = 4;
}

GtkCellRenderer*
tgh_cell_renderer_graph_new (void)
{
  return g_object_new (TGH_TYPE_CELL_RENDERER_GRAPH, NULL);
}

static void
tgh_cell_renderer_graph_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  TghCellRendererGraph *renderer = TGH_CELL_RENDERER_GRAPH (object);

  switch (property_id)
  {
    case PROPERTY_GRAPH_ITER:
      g_value_set_pointer (value, renderer->graph_iter);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
tgh_cell_renderer_graph_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  TghCellRendererGraph *renderer = TGH_CELL_RENDERER_GRAPH (object);
  
  switch (property_id)
  {
    case PROPERTY_GRAPH_ITER:
      renderer->graph_iter = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
tgh_cell_renderer_graph_get_size (GtkCellRenderer *cell, GtkWidget *widget, GdkRectangle *cell_area, gint *x_offset, gint *y_offset, gint *width, gint *height)
{
  TghCellRendererGraph *renderer = TGH_CELL_RENDERER_GRAPH (cell);
  gint graph_width  = 0;
  gint graph_height = 0;
  gint calc_width;
  gint calc_height;

  if (renderer->graph_iter)
  {
    gint count;
    TghGraphNode *node_iter;

    for (node_iter = renderer->graph_iter->data, count = 0; node_iter; node_iter = node_iter->next, count++)
    {
      switch (node_iter->type)
      {
        case TGH_GRAPH_LINE:
          break;
        case TGH_GRAPH_JUNCTION:
          graph_height = renderer->junction_size;
          break;
      }
    }
    
    if (count)
    {
      graph_width  = renderer->spacing + renderer->spacing * count + count;
    }
  }

  calc_width  = (gint) cell->xpad * 2 + graph_width;
  calc_height = (gint) cell->ypad * 2 + graph_height;

  if (cell_area && graph_width > 0 && graph_height > 0)
  {
    if (x_offset)
    {
      *x_offset = (((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
            (1.0 - cell->xalign) : cell->xalign) * (cell_area->width - calc_width));
      if (*x_offset < 0)
        *x_offset = 0;
    }
    if (y_offset)
    {
      *y_offset = (cell->yalign * (cell_area->height - calc_height));
      if (*y_offset < 0)
        *y_offset = 0;
    }
  }
  else
  {
    if (x_offset)
      *x_offset = 0;
    if (y_offset)
      *y_offset = 0;
  }

  if (width)
    *width = calc_width;

  if (height)
    *height = calc_height;
}

static gint
tgh_graph_node_length (TghGraphNode *list)
{
  TghGraphNode *iter;
  gint count;

  for (iter = list, count = 0; iter; iter = iter->next, count++);

  return count;
}

static gint
tgh_graph_node_index_of (TghGraphNode *list, const gchar *name)
{
  TghGraphNode *iter;
  gint index1;

  for (iter = list, index1 = 0; iter; iter = iter->next, index1++)
  {
    if (0 == strcmp (name, iter->name))
      return index1;
  }

  return -1;
}

static void
draw_node (gint index1, TghGraphNode *node_list, const gchar *name, gint x1_offset, gint x2_offset, gint y_offset, gint height, guint spacing, gboolean bottom, gboolean rtl, cairo_t *cr)
{
  gint index2;

  index2 = tgh_graph_node_index_of (node_list, name);

  if (index2 >= 0)
  {
    double x1, x2;
    if (rtl)
    {
      x1 = x1_offset - (spacing + spacing * index1 + index1) + 0.5;
      x2 = x2_offset - (spacing + spacing * index2 + index2) + 0.5;
    }
    else
    {
      x1 = x1_offset + spacing + spacing * index1 + index1 + 0.5;
      x2 = x2_offset + spacing + spacing * index2 + index2 + 0.5;
    }

    if (bottom)
      x2 = (x1+x2)/2;
    else
      x1 = (x1+x2)/2;

    cairo_move_to (cr, x1, y_offset);
    cairo_line_to (cr, x2, y_offset + height);
  }
}

static void
tgh_cell_renderer_graph_render (GtkCellRenderer *cell, GdkDrawable *window, GtkWidget *widget, GdkRectangle *background_area, GdkRectangle *cell_area, GdkRectangle *expose_area, GtkCellRendererState flags)
{
  TghCellRendererGraph *renderer = TGH_CELL_RENDERER_GRAPH (cell);
  gint x_offset;
  gint width;

  tgh_cell_renderer_graph_get_size (cell, widget, cell_area, &x_offset, NULL, &width, NULL);

  if (renderer->graph_iter)
  {
    cairo_t *cr;
    GdkGC *gc;
    GList *graph_iter;
    TghGraphNode *node_iter;
    TghGraphNode *node_list;
    gchar **junction_iter;
    gint index1;
    gint line_height, line_offset;
    guint spacing = renderer->spacing;
    guint junction_size = renderer->junction_size;
    gint y_offset;
    gint height;
    gint x2_offset;
    gint x;
    gboolean rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);
    GtkStateType state;

    if (flags & GTK_CELL_RENDERER_INSENSITIVE)
      state = GTK_STATE_INSENSITIVE;
    else if (flags & GTK_CELL_RENDERER_SELECTED)
      state = GTK_STATE_SELECTED;
    else if (flags & GTK_CELL_RENDERER_PRELIT)
      state = GTK_STATE_PRELIGHT;
    else
      state = GTK_STATE_NORMAL;

    x_offset += cell_area->x + cell->xpad;
    x2_offset = x_offset + width - cell->xpad * 2;
    y_offset = background_area->y;
    height = background_area->height;

    if (rtl)
    {
      gint swap = x_offset;
      x_offset = x2_offset;
      x2_offset = swap;
    }

    cr = gdk_cairo_create (window);
    cairo_set_line_width (cr, 1);
    gdk_cairo_set_source_color (cr, &widget->style->fg[state]);

    gc = widget->style->fg_gc[state];

    if (expose_area)
      gdk_gc_set_clip_rectangle (gc, expose_area);

    node_list = renderer->graph_iter->data;
    graph_iter = g_list_next (renderer->graph_iter);
    if (graph_iter)
    {
      line_height = (height - junction_size) / 2;
      line_offset = y_offset;

      x2_offset = tgh_graph_node_length (graph_iter->data);
      x2_offset = renderer->spacing + renderer->spacing * x2_offset + x2_offset;
      x2_offset = ((rtl ?  (1.0 - cell->xalign) : cell->xalign) * (cell_area->width - x2_offset)) + (rtl ? x2_offset : 0);
      if (x2_offset < 0)
        x2_offset = 0;
      x2_offset += cell_area->x;

      index1 = 0;
      for (node_iter = graph_iter->data; node_iter; node_iter = node_iter->next)
      {
        switch (node_iter->type)
        {
          case TGH_GRAPH_LINE:
            draw_node (index1, node_list, node_iter->name, x2_offset, x_offset, line_offset, line_height, spacing, FALSE, rtl, cr);
            index1++;
            break;
          case TGH_GRAPH_JUNCTION:
            if (node_iter->junction)
            {
              for (junction_iter = node_iter->junction; *junction_iter; junction_iter++)
              {
                draw_node (index1, node_list, *junction_iter, x2_offset, x_offset, line_offset, line_height, spacing, FALSE, rtl, cr);
              }
            }
            index1++;
            break;
        }
      }
    }

    graph_iter = g_list_previous (renderer->graph_iter);
    if (graph_iter)
    {
      node_list = graph_iter->data;

      x2_offset = tgh_graph_node_length (graph_iter->data);
      x2_offset = renderer->spacing + renderer->spacing * x2_offset + x2_offset;
      x2_offset = ((rtl ?  (1.0 - cell->xalign) : cell->xalign) * (cell_area->width - x2_offset)) + (rtl ? x2_offset : 0);
      if (x2_offset < 0)
        x2_offset = 0;
      x2_offset += cell_area->x;
    }
    else
      node_list = NULL;

    line_height = (height - junction_size) / 2;
    line_offset = y_offset + height - line_height;

    index1 = 0;
    for (node_iter = renderer->graph_iter->data; node_iter; node_iter = node_iter->next)
    {
      switch (node_iter->type)
      {
        case TGH_GRAPH_LINE:
          if (rtl)
            x = x_offset - (spacing + spacing * index1 + index1);
          else
            x = x_offset + spacing + spacing * index1 + index1;
          gdk_draw_line (window, gc, x, y_offset + line_height, x, line_offset);

          draw_node (index1, node_list, node_iter->name, x_offset, x2_offset, line_offset, line_height, spacing, TRUE, rtl, cr);
          index1++;
          break;
        case TGH_GRAPH_JUNCTION:
          if (rtl)
            x = x_offset - (spacing + spacing * index1 + index1);
          else
            x = x_offset + spacing + spacing * index1 + index1;
          gdk_draw_rectangle (window, widget->style->bg_gc[state], TRUE, 
              x - (junction_size/2),
              y_offset + line_height,
              junction_size, junction_size);
          gdk_draw_rectangle (window, gc, FALSE, 
              x - (junction_size/2),
              y_offset + line_height,
              junction_size - 1, junction_size - 1);

          if (node_iter->junction)
          {
            for (junction_iter = node_iter->junction; *junction_iter; junction_iter++)
            {
              draw_node (index1, node_list, *junction_iter, x_offset, x2_offset, line_offset, line_height, spacing, TRUE, rtl, cr);
            }
          }
          index1++;
          break;
      }
    }

    if (expose_area)
      gdk_gc_set_clip_rectangle (gc, NULL);

    cairo_stroke (cr);
    cairo_destroy (cr);
  }
}

