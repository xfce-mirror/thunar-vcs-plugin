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

#include "tsh-lock-dialog.h"

struct _TshLockDialog
{
	GtkDialog dialog;

	GtkWidget *text_view;
  GtkWidget *steal;
};

struct _TshLockDialogClass
{
	GtkDialogClass dialog_class;
};

G_DEFINE_TYPE (TshLockDialog, tsh_lock_dialog, GTK_TYPE_DIALOG)

static void
tsh_lock_dialog_class_init (TshLockDialogClass *klass)
{
}

static void
tsh_lock_dialog_init (TshLockDialog *dialog)
{
	GtkWidget *text_view;
	GtkWidget *scroll_window;
  GtkWidget *steal;

	scroll_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  dialog->text_view = text_view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD_CHAR);

	gtk_container_add (GTK_CONTAINER (scroll_window), text_view);
	gtk_widget_show (text_view);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scroll_window, TRUE, TRUE, 0);
	gtk_widget_show (scroll_window);

  dialog->steal = steal = gtk_check_button_new_with_label("Steal Lock");
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), steal, FALSE, FALSE, 0);
	gtk_widget_show (steal);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Lock"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                          NULL);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
}

GtkWidget*
tsh_lock_dialog_new (const gchar *title, GtkWindow *parent, GtkDialogFlags flags)
{
	TshLockDialog *dialog = g_object_new (TSH_TYPE_LOCK_DIALOG, NULL);

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

	return GTK_WIDGET(dialog);
}

gchar *
tsh_lock_dialog_get_message (TshLockDialog *dialog)
{
  GtkTextIter start, end;

  g_return_val_if_fail (TSH_IS_LOCK_DIALOG (dialog), NULL);

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

gboolean
tsh_lock_dialog_get_steal (TshLockDialog *dialog)
{
  g_return_val_if_fail (TSH_IS_LOCK_DIALOG (dialog), FALSE);

  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->steal));
}

