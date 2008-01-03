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
#include <dirent.h>

#ifndef USE_FILE_ENTRY_REPLACEMENT
//#include "tsh-file-chooser-entry.h"
#include "gtkfilechooserentry.h"
//#include <gtk/gtkfilechooserentry.h>
#endif

#include "tsh-transfer-dialog.h"

#ifdef USE_FILE_ENTRY_REPLACEMENT
static void browse_callback(GtkButton *, TshTransferDialog *);
#endif

struct _TshTransferDialog
{
	GtkDialog dialog;

	GtkWidget *repository;
	GtkWidget *path;
#ifdef USE_FILE_ENTRY_REPLACEMENT
  GtkWidget *filechooser;
#endif
};

struct _TshTransferDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshTransferDialog, tsh_transfer_dialog, GTK_TYPE_DIALOG)

static void
tsh_transfer_dialog_class_init (TshTransferDialogClass *klass)
{
}

static void
tsh_transfer_dialog_init (TshTransferDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *label;
#ifdef USE_FILE_ENTRY_REPLACEMENT
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *image;
#endif

	table = gtk_table_new (2, 2, FALSE);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, FALSE, TRUE, 0);
	gtk_widget_show (table);

	label = gtk_label_new_with_mnemonic (_("_Repository:"));
	gtk_table_attach (GTK_TABLE (table), label,
	                  0, 1, 0, 1,
	                  GTK_FILL,
	                  GTK_FILL,
	                  0, 0);

#ifdef USE_FILE_ENTRY_REPLACEMENT
  box = gtk_hbox_new(FALSE, 0);
	dialog->repository = gtk_entry_new();
  dialog->filechooser = gtk_file_chooser_dialog_new(_("Select a folder"), GTK_WINDOW(dialog),
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                    GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                    NULL);
#else
	dialog->repository = gtk_file_chooser_entry_new(_("Select a folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);//tsh_file_chooser_entry_new ();
	//dialog->repository = _gtk_file_chooser_entry_new(FALSE);
#endif

#ifdef USE_FILE_ENTRY_REPLACEMENT
  image = gtk_image_new_from_stock (GTK_STOCK_OPEN,
                                    GTK_ICON_SIZE_MENU);
  button = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(button), image);
  g_signal_connect(button, "clicked", G_CALLBACK(browse_callback), dialog);

  gtk_box_pack_start(GTK_BOX(box), dialog->repository, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, TRUE, 0);

	gtk_widget_show(dialog->repository);
	gtk_widget_show(button);
  
  gtk_table_attach (GTK_TABLE (table), box,
	                  1, 2, 0, 1,
	                  GTK_EXPAND | GTK_FILL,
	                  GTK_FILL,
	                  0, 0);
#else
  gtk_table_attach (GTK_TABLE (table), dialog->repository,
	                  1, 2, 0, 1,
	                  GTK_EXPAND | GTK_FILL,
	                  GTK_FILL,
	                  0, 0);
#endif

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->repository);
	gtk_widget_show(label);
#ifdef USE_FILE_ENTRY_REPLACEMENT
	gtk_widget_show(box);
#else
	gtk_widget_show(dialog->repository);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog->repository), FALSE);
#endif

	label = gtk_label_new_with_mnemonic (_("_Directory:"));
	gtk_table_attach (GTK_TABLE (table), label,
	                  0, 1, 1, 2,
	                  GTK_FILL,
	                  GTK_FILL,
	                  0, 0);

	dialog->path = gtk_file_chooser_button_new (_("Select a folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	//dialog->path = gtk_file_chooser_entry_new(_("Select a folder"), GTK_FILE_CHOOSER_ACTION_OPEN);//GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);//tsh_file_chooser_entry_new ();
	gtk_table_attach (GTK_TABLE (table), dialog->path,
	                  1, 2, 1, 2,
	                  GTK_EXPAND | GTK_FILL,
	                  GTK_FILL,
	                  0, 0);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->path);
	gtk_widget_show(label);
	gtk_widget_show(dialog->path);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Transfer"));

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
	                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                        GTK_STOCK_OK, GTK_RESPONSE_OK,
	                        NULL);
	gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tsh_transfer_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, const gchar *repo_dir, const gchar *local_dir)
{
	TshTransferDialog *dialog = g_object_new (TSH_TYPE_TRANSFER_DIALOG, NULL);

	if(title)
		gtk_window_set_title (GTK_WINDOW(dialog), title);

	if(parent)
		gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	if(flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_NO_SEPARATOR)
		gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);

	if(repo_dir)
  {
    gchar *absolute = NULL;
    if(!g_path_is_absolute (repo_dir))
    {
      //TODO: ".."
      gchar *currdir = g_get_current_dir();
      absolute = g_build_filename(currdir, (repo_dir[0] == '.' && (!repo_dir[1] || repo_dir[1] == G_DIR_SEPARATOR || repo_dir[1] == '/'))?&repo_dir[1]:repo_dir, NULL);
      g_free (currdir);
    }
#ifndef USE_FILE_ENTRY_REPLACEMENT
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog->repository), absolute?absolute:repo_dir);
#endif
    g_free (absolute);
  }

	if(local_dir)
  {
    gboolean isdir = TRUE;
    gchar *absolute = NULL;
    if(!g_path_is_absolute (local_dir))
    {
      //TODO: ".."
      gchar *currdir = g_get_current_dir();
      absolute = g_build_filename(currdir, (local_dir[0] == '.' && (!local_dir[1] || local_dir[1] == G_DIR_SEPARATOR || local_dir[1] == '/'))?&local_dir[1]:local_dir, NULL);
      g_free (currdir);
    }
    DIR *dir = opendir(absolute?absolute:local_dir);
    FILE *fp;
    if(dir)
      closedir(dir);
    else if((fp = fopen(absolute?absolute:local_dir, "r")))
    {
      fclose(fp);
      isdir = FALSE;
    }
    if(isdir)
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog->path), absolute?absolute:local_dir);
    else
    {
      gtk_file_chooser_set_action (GTK_FILE_CHOOSER(dialog->path), GTK_FILE_CHOOSER_ACTION_OPEN);
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(dialog->path), absolute?absolute:local_dir);
    }
    g_free (absolute);
  }

	return GTK_WIDGET(dialog);
}

gchar* tsh_transfer_dialog_get_reposetory (TshTransferDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_TRANSFER_DIALOG (dialog), NULL);

#ifdef USE_FILE_ENTRY_REPLACEMENT
	return g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog->repository)));
#else
	return gtk_file_chooser_entry_get_uri(GTK_FILE_CHOOSER_ENTRY(dialog->repository));
#endif
}

gchar* tsh_transfer_dialog_get_directory (TshTransferDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_TRANSFER_DIALOG (dialog), NULL);

	return gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog->path));
}

#ifdef USE_FILE_ENTRY_REPLACEMENT
static void
browse_callback(GtkButton *button, TshTransferDialog *dialog)
{
  gtk_widget_show(dialog->filechooser);
  if(gtk_dialog_run(GTK_DIALOG(dialog->filechooser)) == GTK_RESPONSE_OK)
  {
    gchar *url = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog->filechooser));
    gtk_entry_set_text(GTK_ENTRY(dialog->repository), url);
    g_free(url);
  }
  gtk_widget_hide(dialog->filechooser);
}
#endif

