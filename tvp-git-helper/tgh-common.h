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

#ifndef __TGH_COMMON_H__
#define __TGH_COMMON_H__

#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#if LIBXFCE4UI_CHECK_VERSION(4, 21, 0)
#define tvp_gtk_dialog_get_action_area(dialog) xfce_gtk_dialog_get_action_area (dialog)
#else
#include <exo/exo.h>
#define tvp_gtk_dialog_get_action_area(dialog) exo_gtk_dialog_get_action_area (dialog)
#endif

G_BEGIN_DECLS

void tgh_replace_child  (gboolean, GPid);
void tgh_cancel         (void);
void tgh_child_exit     (GPid, gint, gpointer);

gchar* tgh_common_prefix (gchar **files);
gchar** tgh_strip_prefix (gchar **files, const gchar *prefix);

#define TGH_OUTPUT_PARSER(x) ((TghOutputParser*)(x))
#define TGH_OUTPUT_PARSER_FUNC(x) ((TghOutputParserFunc)(x))

typedef struct _TghOutputParser TghOutputParser;

typedef void (*TghOutputParserFunc) (TghOutputParser *, gchar *);

struct _TghOutputParser {
  TghOutputParserFunc parse;
};

TghOutputParser* tgh_error_parser_new      (GtkWidget *);

TghOutputParser* tgh_notify_parser_new     (GtkWidget *);

TghOutputParser* tgh_status_parser_new     (GtkWidget *);

TghOutputParser* tgh_log_parser_new        (GtkWidget *);

TghOutputParser* tgh_branch_parser_new     (GtkWidget *);

TghOutputParser* tgh_stash_list_parser_new (GtkWidget *);
TghOutputParser* tgh_stash_show_parser_new (GtkWidget *);

TghOutputParser* tgh_blame_parser_new      (GtkWidget *);

TghOutputParser* tgh_clean_parser_new      (GtkWidget *);

gboolean tgh_parse_output_func  (GIOChannel *, GIOCondition, gpointer);

G_END_DECLS

#endif /*__TGH_COMMON_H__*/
