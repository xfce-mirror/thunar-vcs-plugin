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
#include <dirent.h>

#include "tgh-transfer-dialog.h"

static void browse_callback(GtkButton *, TghTransferDialog *);

struct _TghTransferDialog
{
    GtkDialog dialog;

    GtkWidget *repository;
    GtkWidget *path;
    GtkWidget *filechooser;
};

struct _TghTransferDialogClass
{
    GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TghTransferDialog, tgh_transfer_dialog, GTK_TYPE_DIALOG)

static void
tgh_transfer_dialog_class_init (TghTransferDialogClass *klass)
{
}

static void
tgh_transfer_dialog_init (TghTransferDialog *dialog)
{
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *image;

    table = gtk_table_new (2, 2, FALSE);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), table, FALSE, TRUE, 0);
    gtk_widget_show (table);

    label = gtk_label_new_with_mnemonic (_("_Repository:"));
    gtk_table_attach (GTK_TABLE (table), label,
            0, 1, 0, 1,
            GTK_FILL,
            GTK_FILL,
            0, 0);

    box = gtk_hbox_new(FALSE, 0);
    dialog->repository = gtk_entry_new();
    dialog->filechooser = gtk_file_chooser_dialog_new(_("Select a folder"), GTK_WINDOW(dialog),
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            _("_Cancel"), GTK_RESPONSE_CANCEL,
            _("OK"), GTK_RESPONSE_OK,
            NULL);

    image = gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
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

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->repository);
    gtk_widget_show(label);
    gtk_widget_show(box);

    label = gtk_label_new_with_mnemonic (_("_Directory:"));
    gtk_table_attach (GTK_TABLE (table), label,
            0, 1, 1, 2,
            GTK_FILL,
            GTK_FILL,
            0, 0);

    dialog->path = gtk_file_chooser_button_new (_("Select a folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    //dialog->path = gtk_file_chooser_entry_new(_("Select a folder"), GTK_FILE_CHOOSER_ACTION_OPEN);//GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);//tgh_file_chooser_entry_new ();
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
            _("_Cancel"), GTK_RESPONSE_CANCEL,
            _("OK"), GTK_RESPONSE_OK,
            NULL);
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tgh_transfer_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, const gchar *repo_dir, const gchar *local_dir)
{
    TghTransferDialog *dialog = g_object_new (TGH_TYPE_TRANSFER_DIALOG, NULL);

    if(title)
        gtk_window_set_title (GTK_WINDOW(dialog), title);

    if(parent)
        gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

    if(flags & GTK_DIALOG_MODAL)
        gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

    if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
        gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

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
      g_free (absolute);
    }

    if(local_dir)
    {
        gboolean isdir = TRUE;
        gchar *absolute = NULL;
        DIR *dir;
        FILE *fp;
        if(!g_path_is_absolute (local_dir))
        {
            //TODO: ".."
            gchar *currdir = g_get_current_dir();
            absolute = g_build_filename(currdir, (local_dir[0] == '.' && (!local_dir[1] || local_dir[1] == G_DIR_SEPARATOR || local_dir[1] == '/'))?&local_dir[1]:local_dir, NULL);
            g_free (currdir);
        }
        dir = opendir(absolute?absolute:local_dir);
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

gchar* tgh_transfer_dialog_get_repository (TghTransferDialog *dialog)
{
    g_return_val_if_fail (TGH_IS_TRANSFER_DIALOG (dialog), NULL);

    return g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog->repository)));
}

gchar* tgh_transfer_dialog_get_directory (TghTransferDialog *dialog)
{
    g_return_val_if_fail (TGH_IS_TRANSFER_DIALOG (dialog), NULL);

    return gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog->path));
}

static void
browse_callback(GtkButton *button, TghTransferDialog *dialog)
{
    gtk_widget_show(dialog->filechooser);
    if(gtk_dialog_run(GTK_DIALOG(dialog->filechooser)) == GTK_RESPONSE_OK)
    {
        gchar *url = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog->filechooser));
        gtk_entry_set_text(GTK_ENTRY(dialog->repository), url);
        g_free(url);
    }
    gtk_widget_hide(dialog->filechooser);
}
