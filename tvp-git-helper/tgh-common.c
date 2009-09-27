/*-
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>

#include "tgh-dialog-common.h"
#include "tgh-status-dialog.h"
#include "tgh-branch-dialog.h"

#include "tgh-common.h"

static void
create_error_dialog(GtkWindow *parent, gchar *message)
{
  GtkWidget *error;
  error = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Status failed"));
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error), "%s", g_strstrip(message));
  tgh_dialog_start(GTK_DIALOG(error), FALSE);
}

void
tgh_cancel(void)
{
}

typedef struct {
  TghOutputParser parent;
  gchar *error;
  GtkWidget *dialog;
  gboolean done, show_error;
} TghErrorParser;

void
tgh_child_exit(GPid pid, gint status, gpointer user_data)
{
  TghErrorParser *parser = user_data;
  if(WEXITSTATUS(status) > 1)
  {
    if(parser->done)
      create_error_dialog(GTK_WINDOW(parser->dialog), parser->error);
    else
      parser->show_error = TRUE;
  }
  tgh_replace_child(FALSE, 0);
}

static void
error_parser_func(TghErrorParser *parser, gchar *line)
{
  if(line)
    parser->error = g_strconcat(parser->error, line, NULL);
  else
    if(parser->show_error)
      create_error_dialog(GTK_WINDOW(parser->dialog), parser->error);
    else
      parser->done = TRUE;
}

TghOutputParser*
tgh_error_parser_new(GtkWidget *dialog)
{
  TghErrorParser *parser = g_new0(TghErrorParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(error_parser_func);

  parser->error = g_strdup("");
  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
  gboolean commit;
} TghStatusParser;

static void
status_parser_func(TghStatusParser *parser, gchar *line)
{
  TghStatusDialog *dialog = TGH_STATUS_DIALOG(parser->dialog);
  if(line)
  {
    if(strstr(line, "git reset"))
      parser->commit = TRUE;
    else if(strstr(line, "git add"))
      parser->commit = FALSE;
    if(line[0] == '#' && line[1] == '\t')
    {
      gchar *file = strchr(line, ':');
      gchar *state = _("untracked");
      if(file)
      {
        *file = '\0';
        state = line+2;
        file = g_strstrip(file+1);
      }
      else
        file = g_strstrip(line+2);

      tgh_status_dialog_add(dialog, file, state, parser->commit);
    }
  }
  else
  {
    tgh_status_dialog_done(dialog);
    g_free(parser);
  }
}

TghOutputParser*
tgh_status_parser_new (GtkWidget *dialog)
{
  TghStatusParser *parser = g_new(TghStatusParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(status_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
} TghBranchParser;

static void
branch_parser_func(TghStatusParser *parser, gchar *line)
{
  TghBranchDialog *dialog = TGH_BRANCH_DIALOG(parser->dialog);
  if(line)
  {
    gboolean active = line[0] == '*';
    gchar *branch = g_strstrip(line+2);
    tgh_branch_dialog_add(dialog, branch, active);
  }
  else
  {
    tgh_branch_dialog_done(dialog);
    g_free(parser);
  }
}

TghOutputParser*
tgh_branch_parser_new (GtkWidget *dialog)
{
  TghBranchParser *parser = g_new(TghBranchParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(branch_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

gboolean
tgh_parse_output_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
  TghOutputParser *parser = TGH_OUTPUT_PARSER (data);
  gchar *line;

  if(condition & G_IO_IN)
  {
    while(g_io_channel_read_line(source, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL)
    {
      parser->parse(parser, line);
      g_free(line);
    }
  }

  if(condition & G_IO_HUP)
  {
    parser->parse(parser, NULL);
    g_io_channel_unref(source);
    return FALSE;
  }
  return TRUE;
}

