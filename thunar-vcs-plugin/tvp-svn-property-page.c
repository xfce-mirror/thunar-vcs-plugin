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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <thunarx/thunarx.h>

#include <subversion-1/svn_types.h>

#include <libxfce4util/libxfce4util.h>

#include <thunar-vcs-plugin/tvp-svn-backend.h>
#include <thunar-vcs-plugin/tvp-svn-property-page.h>

#include <string.h>



struct _TvpSvnPropertyPageClass
{
	ThunarxPropertyPageClass __parent__;
};



struct _TvpSvnPropertyPage
{
	ThunarxPropertyPage __parent__;

	ThunarxFileInfo *file;
	GtkWidget *url;
	GtkWidget *revision;
	GtkWidget *repository;
	GtkWidget *modrev;
	GtkWidget *moddate;
	GtkWidget *modauthor;
	GtkWidget *changelist;
	GtkWidget *depth;
};



enum {
	PROPERTY_FILE = 1
};



static void tvp_svn_property_page_finalize (GObject*);

static void tvp_svn_property_page_set_property (GObject*, guint, const GValue*, GParamSpec*);

static void tvp_svn_property_page_get_property (GObject*, guint, GValue*, GParamSpec*);



THUNARX_DEFINE_TYPE (TvpSvnPropertyPage, tvp_svn_property_page, THUNARX_TYPE_PROPERTY_PAGE)



static void
tvp_svn_property_page_class_init (TvpSvnPropertyPageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = tvp_svn_property_page_finalize;
	gobject_class->set_property = tvp_svn_property_page_set_property;
	gobject_class->get_property = tvp_svn_property_page_get_property;

	g_object_class_install_property (gobject_class, PROPERTY_FILE,
		g_param_spec_object ("file", "file", "file", THUNARX_TYPE_FILE_INFO, G_PARAM_READWRITE));
}



static void
tvp_svn_property_page_init (TvpSvnPropertyPage *self)
{
  GtkWidget *grid;
  GtkWidget *label;
  GtkWidget *spacer;
  PangoAttrList *attr_list;
  PangoAttribute *attribute;

  attribute = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  /* allocate a new attribute list */
  attr_list = pango_attr_list_new ();

  /* add all specified attributes */
  attribute->start_index = 0;
  attribute->end_index = -1;
  pango_attr_list_insert (attr_list, attribute);

  gtk_container_set_border_width (GTK_CONTAINER (self), 12);

  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);

  label = gtk_label_new (_("URL:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_widget_show (label);

  self->url = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_START);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);
  gtk_widget_show (label);

  label = gtk_label_new (_("Revision:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
  gtk_widget_show (label);

  self->revision = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);
  gtk_widget_show (label);

  label = gtk_label_new (_("Repository:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);
  gtk_widget_show (label);

  self->repository = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 2, 1, 1);
  gtk_widget_show (label);

  /* Alignment in the most simple widget to find, for just doing a size request */
  spacer = g_object_new (GTK_TYPE_BOX,
                         "orientation", GTK_ORIENTATION_VERTICAL,
                         "height-request", 12,
                         "hexpand", TRUE,
                         "vexpand", TRUE,
                         NULL);
  gtk_grid_attach (GTK_GRID (grid), spacer, 0, 3, 2, 1);
  gtk_widget_show (spacer);

  label = gtk_label_new (_("Modified revision:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);
  gtk_widget_show (label);

  self->modrev = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1);
  gtk_widget_show (label);

  label = gtk_label_new (_("Modified date:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 5, 1, 1);
  gtk_widget_show (label);

  self->moddate = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 5, 1, 1);
  gtk_widget_show (label);

  label = gtk_label_new (_("Author:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 6, 1, 1);
  gtk_widget_show (label);

  self->modauthor = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 6, 1, 1);
  gtk_widget_show (label);

  /* Alignment in the most simple widget to find, for just doing a size request */
  spacer = g_object_new (GTK_TYPE_BOX,
                         "orientation", GTK_ORIENTATION_VERTICAL,
                         "height-request", 12,
                         "hexpand", TRUE,
                         "vexpand", TRUE,
                         NULL);
  gtk_grid_attach (GTK_GRID (grid), spacer, 0, 7, 2, 1);
  gtk_widget_show (spacer);

  label = gtk_label_new (_("Changelist:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 8, 1, 1);
  gtk_widget_show (label);

  self->changelist = label = gtk_label_new("");
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 8, 1, 1);
  gtk_widget_show (label);

  /* Translators: Depth as in depth of recursion */
  label = gtk_label_new (_("Depth:"));
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 9, 1, 1);
  gtk_widget_show (label);

  self->depth = label = gtk_label_new(_("Unknown"));
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_yalign (GTK_LABEL (label), 0.5);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 9, 1, 1);
  gtk_widget_show (label);

  /*TODO: kind, repos UUID, lock
   * wc info: size, schedule, copy from, text time, prop time, checksum, confilct, prejfile, working size */

  gtk_container_add (GTK_CONTAINER (self), grid);
  gtk_widget_show (GTK_WIDGET (grid));
}



GtkAction *
tvp_svn_property_page_new (ThunarxFileInfo *file)
{
	GtkAction *action = g_object_new (TVP_TYPE_SVN_PROPERTY_PAGE,
                                    "label", "Subversion",
                                    "file", file,
																		NULL);
	return action;
}



static void
tvp_svn_property_page_finalize (GObject *object)
{
	tvp_svn_property_page_set_file (TVP_SVN_PROPERTY_PAGE (object), NULL);

	G_OBJECT_CLASS (tvp_svn_property_page_parent_class)->finalize (object);
}



static void
tvp_svn_property_page_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
		case PROPERTY_FILE:
      tvp_svn_property_page_set_file (TVP_SVN_PROPERTY_PAGE (object), g_value_get_object (value));
		break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
	}
}



static void
tvp_svn_property_page_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
		case PROPERTY_FILE:
      g_value_set_object (value, tvp_svn_property_page_get_file (TVP_SVN_PROPERTY_PAGE (object)));
		break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
	}
}



static const gchar *
depth_to_string(svn_depth_t depth)
{
  const gchar *depth_string;

	switch(depth)
	{
    default:
    case svn_depth_unknown:
      depth_string = _("Unknown");
      break;
    case svn_depth_exclude:
      /* Translators: svn recursion depth infotmation
       * Exclude should not apear client side
       */
      depth_string = _("Exclude");
      break;
    case svn_depth_empty:
      /* Translators: svn recursion depth infotmation
       * Empty depth means only this file/direcotry is checked out
       */
      depth_string = _("Empty");
      break;
    case svn_depth_files:
      /* Translators: svn recursion depth infotmation
       * Files depth means this file/direcotry and all of it's files are checked out
       */
      depth_string = _("Files");
      break;
    case svn_depth_immediates:
      /* Translators: svn recursion depth infotmation
       * Immediates depth means this file/direcotry and all of it's files and subdirectories are checked out
       */
      depth_string = _("Immediates");
      break;
    case svn_depth_infinity:
      /* Translators: svn recursion depth infotmation
       * Infinity depth means this file/direcotry is checked out with full recursion
       */
      depth_string = _("Infinity");
      break;
	}
  return depth_string;
}



static void
tvp_svn_property_page_file_changed (ThunarxFileInfo *file, TvpSvnPropertyPage *page)
{
  TvpSvnInfo *info = NULL;
  gchar  *filename;
  gchar  *uri;

  /* determine the parent URI for the file info */
  uri = thunarx_file_info_get_uri (file);
  if (G_LIKELY (uri != NULL))
  {
    /* determine the local filename for the URI */
    filename = g_filename_from_uri (uri, NULL, NULL);
    if (G_LIKELY (filename != NULL))
    {
      /* check if the folder is a working copy */
      info = tvp_svn_backend_get_info (filename);

      /* release the filename */
      g_free (filename);
    }

    /* release the URI */
    g_free (uri);
  }

  if (info)
  {
    gchar *tmpstr;
    gtk_label_set_text (GTK_LABEL (page->url), info->url);
    tmpstr = g_strdup_printf ("%"SVN_REVNUM_T_FMT, info->revision);
    gtk_label_set_text (GTK_LABEL (page->revision), tmpstr);
    g_free (tmpstr);
    gtk_label_set_text (GTK_LABEL (page->repository), info->repository);
    tmpstr = g_strdup_printf ("%"SVN_REVNUM_T_FMT, info->modrev);
    gtk_label_set_text (GTK_LABEL (page->modrev), tmpstr);
    g_free (tmpstr);
    gtk_label_set_text (GTK_LABEL (page->moddate), info->moddate);
    gtk_label_set_text (GTK_LABEL (page->modauthor), info->modauthor);
    if(info->has_wc_info)
    {
      if(info->changelist)
        gtk_label_set_text (GTK_LABEL (page->changelist), info->changelist);
      if(info->depth)
        gtk_label_set_text (GTK_LABEL (page->depth), depth_to_string(info->depth));
    }

    tvp_svn_info_free (info);
  }
}



void
tvp_svn_property_page_set_file (TvpSvnPropertyPage *page, ThunarxFileInfo *file)
{
  g_return_if_fail (TVP_IS_SVN_PROPERTY_PAGE (page));
  g_return_if_fail (file == NULL || THUNARX_IS_FILE_INFO (file));

  if (page->file != NULL)
  {
    g_signal_handlers_disconnect_by_func (page->file, tvp_svn_property_page_file_changed, page);
    g_object_unref (G_OBJECT (page->file));
  }

  page->file = file;

  if (file != NULL)
  {
    g_object_ref (file);
    tvp_svn_property_page_file_changed (file, page);
    g_signal_connect (file, "changed", G_CALLBACK (tvp_svn_property_page_file_changed), page);
  }

  g_object_notify (G_OBJECT (page), "file");
}



ThunarxFileInfo*
tvp_svn_property_page_get_file (TvpSvnPropertyPage *page)
{
  g_return_val_if_fail (TVP_IS_SVN_PROPERTY_PAGE (page), NULL);
  return page->file;
}

