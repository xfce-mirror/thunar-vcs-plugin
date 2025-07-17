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

#include <libxfce4util/libxfce4util.h>

#include "tsh-login-dialog.h"

static void username_activate (GtkEntry*, gpointer);
static void password_activate (GtkEntry*, gpointer);

struct _TshLoginDialog
{
	GtkDialog dialog;

	GtkWidget *user_lbl;
	GtkWidget *username;
	GtkWidget *pass_lbl;
	GtkWidget *password;
	GtkWidget *may_save;
};

struct _TshLoginDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshLoginDialog, tsh_login_dialog, GTK_TYPE_DIALOG)

static void
tsh_login_dialog_class_init (TshLoginDialogClass *klass)
{
}

static void
tsh_login_dialog_init (TshLoginDialog *dialog)
{
	GtkWidget *grid;

	grid = gtk_grid_new ();

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), grid, FALSE, TRUE, 0);
	gtk_widget_show (grid);

	dialog->user_lbl = gtk_label_new_with_mnemonic (_("_Username:"));
	gtk_grid_attach (GTK_GRID (grid), dialog->user_lbl, 0, 0, 1, 1);

	dialog->username = gtk_entry_new();
	gtk_widget_set_hexpand (dialog->username, TRUE);
	gtk_grid_attach (GTK_GRID (grid), dialog->username, 1, 0, 1, 1);

	gtk_label_set_mnemonic_widget (GTK_LABEL (dialog->user_lbl), dialog->username);

	dialog->pass_lbl = gtk_label_new_with_mnemonic (_("_Password:"));
	gtk_grid_attach (GTK_GRID (grid), dialog->pass_lbl, 0, 1, 1, 1);

	dialog->password = gtk_entry_new();
	gtk_widget_set_hexpand (dialog->password, TRUE);
	gtk_grid_attach (GTK_GRID (grid), dialog->password, 1, 1, 1, 1);

	gtk_entry_set_visibility(GTK_ENTRY(dialog->password), FALSE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (dialog->pass_lbl), dialog->password);

	dialog->may_save = gtk_check_button_new_with_label(_("Remember"));
	gtk_widget_set_hexpand (dialog->may_save, TRUE);
	gtk_grid_attach (GTK_GRID (grid), dialog->may_save, 0, 2, 2, 1);
	gtk_widget_show(dialog->may_save);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Login"));

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
	                        _("_Cancel"), GTK_RESPONSE_CANCEL,
	                        _("_OK"), GTK_RESPONSE_OK,
	                        NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tsh_login_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, const gchar *username, gboolean password, gboolean may_save)
{
	TshLoginDialog *dialog = g_object_new (TSH_TYPE_LOGIN_DIALOG, NULL);

	if(title)
		gtk_window_set_title (GTK_WINDOW(dialog), title);

	if(parent)
		gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	if(flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	if(username)
	{
		gtk_widget_show(dialog->user_lbl);
		gtk_entry_set_text(GTK_ENTRY(dialog->username), username);
		gtk_widget_show(dialog->username);
		if(password)
			g_signal_connect (dialog->username, "activate", G_CALLBACK (username_activate), dialog);
		else
			g_signal_connect (dialog->username, "activate", G_CALLBACK (password_activate), dialog);
	}

	if(password)
	{
		gtk_widget_show(dialog->pass_lbl);
		gtk_widget_show(dialog->password);
		g_signal_connect (dialog->password, "activate", G_CALLBACK (password_activate), dialog);
	}

	if(!may_save)
		gtk_widget_set_sensitive(dialog->may_save, FALSE);

	return GTK_WIDGET(dialog);
}

gchar*
tsh_login_dialog_get_username (TshLoginDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_LOGIN_DIALOG (dialog), NULL);

	return g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog->username)));
}

gchar*
tsh_login_dialog_get_password (TshLoginDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_LOGIN_DIALOG (dialog), NULL);

	return g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog->password)));
}

gboolean
tsh_login_dialog_get_may_save (TshLoginDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_LOGIN_DIALOG (dialog), FALSE);

	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->may_save));
}

static void
username_activate (GtkEntry *entry, gpointer user_data)
{
	TshLoginDialog *dialog = TSH_LOGIN_DIALOG (user_data);

	gtk_widget_grab_focus (dialog->password);
}

static void
password_activate (GtkEntry *entry, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG (user_data), GTK_RESPONSE_OK);
}

