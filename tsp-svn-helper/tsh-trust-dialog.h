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

#ifndef __TSH_TRUST_DIALOG_H__
#define __TSH_TRUST_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _TshTrustDialogClass TshTrustDialogClass;
typedef struct _TshTrustDialog      TshTrustDialog;

#define TSH_TYPE_TRUST_DIALOG             (tsh_trust_dialog_get_type ())
#define TSH_TRUST_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSH_TYPE_TRUST_DIALOG, TshTrustDialog))
#define TSH_TRUST_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSH_TYPE_TRUST_DIALOG, TshTrustDialogClass))
#define TSH_IS_TRUST_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSH_TYPE_TRUST_DIALOG))
#define TSH_IS_TRUST_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSH_TYPE_TRUST_DIALOG))
#define TSH_TRUST_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSH_TYPE_TRUST_DIALOG, TshTrustDialogClass))

GType      tsh_trust_dialog_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

GtkWidget* tsh_trust_dialog_new      (const gchar *title,
                                      GtkWindow *parent,
                                      GtkDialogFlags flags,
                                      guint32 failures,
                                      gboolean may_save) G_GNUC_MALLOC G_GNUC_INTERNAL;

guint32      tsh_trust_dialog_get_accepted (TshTrustDialog *dialog);

gboolean     tsh_trust_dialog_get_may_save (TshTrustDialog*);

G_END_DECLS;

#endif /* !__TSH_TRUST_DIALOG_H__ */
