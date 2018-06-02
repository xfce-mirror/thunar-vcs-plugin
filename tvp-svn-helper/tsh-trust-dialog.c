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

#include <subversion-1/svn_auth.h>

#include "tsh-trust-dialog.h"

struct _TshTrustDialog
{
	GtkDialog dialog;

	GtkWidget *notyetvalid;
	GtkWidget *expired;
	GtkWidget *cnmismatch;
	GtkWidget *unknownca;
	GtkWidget *other;
	GtkWidget *may_save;
};

struct _TshTrustDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshTrustDialog, tsh_trust_dialog, GTK_TYPE_DIALOG)

static void
tsh_trust_dialog_class_init (TshTrustDialogClass *klass)
{
}

static void
tsh_trust_dialog_init (TshTrustDialog *dialog)
{
	GtkBox *content_area;
	content_area = GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog)));

	dialog->notyetvalid = gtk_check_button_new_with_label (_("Certificate is not yet valid."));
	gtk_box_pack_start (content_area, dialog->notyetvalid, FALSE, TRUE, 0);

	dialog->expired = gtk_check_button_new_with_label (_("Certificate has expired."));
	gtk_box_pack_start (content_area, dialog->expired, FALSE, TRUE, 0);

	dialog->cnmismatch = gtk_check_button_new_with_label (_("Certificate does not match the remote hostname."));
	gtk_box_pack_start (content_area, dialog->cnmismatch, FALSE, TRUE, 0);

	dialog->unknownca = gtk_check_button_new_with_label (_("Certificate authority is unknown."));
	gtk_box_pack_start (content_area, dialog->unknownca, FALSE, TRUE, 0);

	dialog->other = gtk_check_button_new_with_label (_("Other failure."));
	gtk_box_pack_start (content_area, dialog->other, FALSE, TRUE, 0);

	dialog->may_save = gtk_check_button_new_with_label(_("Remember"));
	gtk_box_pack_start (content_area, dialog->may_save, FALSE, TRUE, 0);
	gtk_widget_show(dialog->may_save);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Trust"));

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
	                        _("_Cancel"), GTK_RESPONSE_CANCEL,
	                        _("_OK"), GTK_RESPONSE_OK,
	                        NULL);
	gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

GtkWidget*
tsh_trust_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags, guint32 failures, gboolean may_save)
{
	TshTrustDialog *dialog = g_object_new (TSH_TYPE_TRUST_DIALOG, NULL);

	if(title)
		gtk_window_set_title (GTK_WINDOW(dialog), title);

	if(parent)
		gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	if(flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if(flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	if(failures & SVN_AUTH_SSL_NOTYETVALID)
		gtk_widget_show(dialog->notyetvalid);

	if(failures & SVN_AUTH_SSL_EXPIRED)
		gtk_widget_show(dialog->expired);

	if(failures & SVN_AUTH_SSL_CNMISMATCH)
		gtk_widget_show(dialog->cnmismatch);

	if(failures & SVN_AUTH_SSL_UNKNOWNCA)
		gtk_widget_show(dialog->unknownca);

	if(failures & SVN_AUTH_SSL_OTHER)
		gtk_widget_show(dialog->other);

	if(!may_save)
		gtk_widget_set_sensitive(dialog->may_save, FALSE);

	return GTK_WIDGET(dialog);
}

guint32
tsh_trust_dialog_get_accepted (TshTrustDialog *dialog)
{
	guint32 failures = 0;

  g_return_val_if_fail (TSH_IS_TRUST_DIALOG (dialog), failures);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->notyetvalid)))
		failures |= SVN_AUTH_SSL_NOTYETVALID;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->expired)))
		failures |= SVN_AUTH_SSL_EXPIRED;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->cnmismatch)))
		failures |= SVN_AUTH_SSL_CNMISMATCH;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->unknownca)))
		failures |= SVN_AUTH_SSL_UNKNOWNCA;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->other)))
		failures |= SVN_AUTH_SSL_OTHER;
	return failures;
}

gboolean
tsh_trust_dialog_get_may_save (TshTrustDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_TRUST_DIALOG (dialog), FALSE);

	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->may_save));
}

