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

#include <libxfce4util/libxfce4util.h>
#include <gtk/gtk.h>

#include "tsh-file-dialog.h"

struct _TshFileDialog
{
	GtkDialog dialog;

	GtkWidget *filename;
	GtkWidget *may_save;
};

struct _TshFileDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshFileDialog, tsh_file_dialog, GTK_TYPE_DIALOG)

static void
tsh_file_dialog_class_init (TshFileDialogClass *klass)
{
}

static void
tsh_file_dialog_init (TshFileDialog *dialog)
{
	GtkWidget *grid;
	GtkWidget *label;

	grid = gtk_grid_new ();

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), grid, FALSE, TRUE, 0);
	gtk_widget_show (grid);

	label = gtk_label_new_with_mnemonic (_("_Certificate:"));
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

	dialog->filename = gtk_file_chooser_button_new (_("Select a file"), GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_widget_set_hexpand (dialog->filename, TRUE);
	gtk_grid_attach (GTK_GRID (grid), dialog->filename, 1, 0, 1, 1);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->filename);
	gtk_widget_show(label);
	gtk_widget_show(dialog->filename);

	dialog->may_save = gtk_check_button_new_with_label(_("Remember"));
	gtk_widget_set_hexpand (dialog->may_save, TRUE);
	gtk_grid_attach (GTK_GRID (grid), dialog->may_save, 0, 1, 2, 1);
	gtk_widget_show(dialog->may_save);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Certificate"));

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
	                        _("_Cancel"), GTK_RESPONSE_CANCEL,
	                        _("_OK"), GTK_RESPONSE_OK,
	                        NULL);
	gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tsh_file_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, gboolean may_save)
{
	TshFileDialog *dialog = g_object_new (TSH_TYPE_FILE_DIALOG, NULL);

	if(title)
		gtk_window_set_title (GTK_WINDOW(dialog), title);

	if(parent)
		gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	if(flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	if(!may_save)
		gtk_widget_set_sensitive(dialog->may_save, FALSE);

	return GTK_WIDGET(dialog);
}

gchar*
tsh_file_dialog_get_filename (TshFileDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_FILE_DIALOG (dialog), NULL);

	return gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog->filename));
}

gboolean
tsh_file_dialog_get_may_save (TshFileDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_FILE_DIALOG (dialog), FALSE);

	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->may_save));
}

