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

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-svn-plugin/tsp-svn-backend.h>
#include <thunar-svn-plugin/tsp-svn-property-page.h>

#include <string.h>

#include <subversion-1/svn_types.h>



struct _TspSvnPropertyPageClass
{
	ThunarxPropertyPageClass __parent__;
};



struct _TspSvnPropertyPage
{
	ThunarxPropertyPage __parent__;

	ThunarxFileInfo *file;
	GtkWidget *url;
	GtkWidget *revision;
	GtkWidget *repository;
	GtkWidget *modrev;
	GtkWidget *moddate;
	GtkWidget *modauthor;
};



enum {
	PROPERTY_FILE = 1
};



static void tsp_svn_property_page_finalize (GObject*);

static void tsp_svn_property_page_set_property (GObject*, guint, const GValue*, GParamSpec*);

static void tsp_svn_property_page_get_property (GObject*, guint, GValue*, GParamSpec*);



THUNARX_DEFINE_TYPE (TspSvnPropertyPage, tsp_svn_property_page, THUNARX_TYPE_PROPERTY_PAGE)



static void
tsp_svn_property_page_class_init (TspSvnPropertyPageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = tsp_svn_property_page_finalize;
	gobject_class->set_property = tsp_svn_property_page_set_property;
	gobject_class->get_property = tsp_svn_property_page_get_property;

	g_object_class_install_property (gobject_class, PROPERTY_FILE,
		g_param_spec_object ("file", "file", "file", THUNARX_TYPE_FILE_INFO, G_PARAM_READWRITE));
}



static void
tsp_svn_property_page_init (TspSvnPropertyPage *self)
{
  GtkWidget *table;
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

  table = gtk_table_new (7, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);

  label = gtk_label_new (_("URL:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->url = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_START);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_("Revision:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->revision = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_("Repository:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->repository = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  /* Alignment in the most simple widget to find, for just doing a size request */
  spacer = g_object_new (GTK_TYPE_ALIGNMENT, "height-request", 12, NULL);
  gtk_table_attach (GTK_TABLE (table), spacer, 0, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (spacer);

  label = gtk_label_new (_("Modified revision:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->modrev = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_("Modified date:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->moddate = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_("Author:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_label_set_attributes (GTK_LABEL (label), attr_list);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  self->modauthor = label = gtk_label_new(_("Unknown"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 6, 7, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  /*TODO: lock*/

  /*TODO: wc info*/

  gtk_container_add (GTK_CONTAINER (self), table);
  gtk_widget_show (GTK_WIDGET (table));
}



GtkAction *
tsp_svn_property_page_new (ThunarxFileInfo *file)
{
	GtkAction *action = g_object_new (TSP_TYPE_SVN_PROPERTY_PAGE,
                                    "label", "Subversion",
                                    "file", file,
																		NULL);
	return action;
}



static void
tsp_svn_property_page_finalize (GObject *object)
{
	tsp_svn_property_page_set_file (TSP_SVN_PROPERTY_PAGE (object), NULL);

	G_OBJECT_CLASS (tsp_svn_property_page_parent_class)->finalize (object);
}



static void
tsp_svn_property_page_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
		case PROPERTY_FILE:
      tsp_svn_property_page_set_file (TSP_SVN_PROPERTY_PAGE (object), g_value_get_object (value));
		break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
	}
}



static void
tsp_svn_property_page_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
		case PROPERTY_FILE:
      g_value_set_object (value, tsp_svn_property_page_get_file (TSP_SVN_PROPERTY_PAGE (object)));
		break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
	}
}



static void
tsp_svn_property_page_file_changed (ThunarxFileInfo *file, TspSvnPropertyPage *page)
{
  TspSvnInfo *info = NULL;
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
      info = tsp_svn_backend_get_info (filename);

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

    tsp_svn_info_free (info);
  }
}



void
tsp_svn_property_page_set_file (TspSvnPropertyPage *page, ThunarxFileInfo *file)
{
  g_return_if_fail (TSP_IS_SVN_PROPERTY_PAGE (page));
  g_return_if_fail (file == NULL || THUNARX_IS_FILE_INFO (file));

  if (page->file != NULL)
  {
    g_signal_handlers_disconnect_by_func (page->file, tsp_svn_property_page_file_changed, page);
    g_object_unref (G_OBJECT (page->file));
  }

  page->file = file;

  if (file != NULL)
  {
    g_object_ref (file);
    tsp_svn_property_page_file_changed (file, page);
    g_signal_connect (file, "changed", G_CALLBACK (tsp_svn_property_page_file_changed), page);
  }

  g_object_notify (G_OBJECT (page), "file");
}



ThunarxFileInfo*
tsp_svn_property_page_get_file (TspSvnPropertyPage *page)
{
  g_return_val_if_fail (TSP_IS_SVN_PROPERTY_PAGE (page), NULL);
  return page->file;
}

