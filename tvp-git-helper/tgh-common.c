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
#include "tgh-notify-dialog.h"
#include "tgh-status-dialog.h"
#include "tgh-log-dialog.h"
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
} TghNotifyParser;

static void
notify_parser_func(TghNotifyParser *parser, gchar *line)
{
  TghNotifyDialog *dialog = TGH_NOTIFY_DIALOG(parser->dialog);
  if(line)
  {
    gchar *action, *file;

    file = strchr(line, '\'');
    if(file)
    {
      *file++ = '\0';
      *strrchr(file, '\'') = '\0';

      action = g_strstrip(line);

      tgh_notify_dialog_add(dialog, action, file);
    }
  }
  else
  {
    tgh_notify_dialog_done(dialog);
    g_free(parser);
  }
}

TghOutputParser*
tgh_notify_parser_new (GtkWidget *dialog)
{
  TghNotifyParser *parser = g_new(TghNotifyParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(notify_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
  enum {
    STATUS_COMMIT,
    STATUS_MODIFIED,
    STATUS_UNTRACKED
  } state;
} TghStatusParser;

static void
status_parser_func(TghStatusParser *parser, gchar *line)
{
  TghStatusDialog *dialog = TGH_STATUS_DIALOG(parser->dialog);
  if(line)
  {
    if(line[0] == '#' && line[1] == '\t')
    {
      gchar *file = strchr(line, ':');
      gchar *state = _("untracked");
      if(file && parser->state != STATUS_UNTRACKED)
      {
        *file = '\0';
        state = line+2;
        file = line+14;
      }
      else
        file = line+2;
      file[strlen(file)-1] = '\0';
      file = g_shell_unquote(file, NULL);

      tgh_status_dialog_add(dialog, file, state, parser->state == STATUS_COMMIT);

      g_free(file);
    }
    else if(strstr(line, "git reset"))
      parser->state = STATUS_COMMIT;
    else if(strstr(line, "git add"))
      parser->state = STATUS_UNTRACKED;
    else if(strstr(line, "git checkout"))
      parser->state = STATUS_MODIFIED;
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
  gchar *revision;
  gchar *author;
  gchar *author_date;
  gchar *commit;
  gchar *commit_date;
  gchar *message;
  GSList *files;
} TghLogParser;

static void
log_parser_add_entry(TghLogParser *parser, TghLogDialog *dialog)
{
  tgh_log_dialog_add(dialog,
      g_slist_reverse(parser->files),
      parser->revision,
      parser->author,
      parser->author_date,
      parser->commit,
      parser->commit_date,
      parser->message);

  parser->files = NULL;
  g_free(parser->revision);
  parser->revision = NULL;
  parser->author = NULL;
  g_free(parser->author_date);
  parser->author_date = NULL;
  g_free(parser->commit);
  parser->commit = NULL;
  g_free(parser->commit_date);
  parser->commit_date = NULL;
  g_free(parser->message);
  parser->message = NULL;
}

static void
log_parser_func(TghLogParser *parser, gchar *line)
{
  TghLogDialog *dialog = TGH_LOG_DIALOG(parser->dialog);
  if(line)
  {
    if(strncmp(line, "commit ", 7) == 0)
    {
      gchar *revision;

      if(parser->revision)
        log_parser_add_entry(parser, dialog);

      // read first 6 chars of hash?
      revision = g_strstrip(line+6);
      parser->revision = g_strndup(revision, revision[0]=='-'?7:6);
    }
    else if(strncmp(line, "Author:", 7) == 0)
    {
      parser->author = g_strdup(g_strstrip(line+7));
    }
    else if(strncmp(line, "AuthorDate:", 11) == 0)
    {
      parser->author_date = g_strdup(g_strstrip(line+11));
    }
    else if(strncmp(line, "Commit:", 7) == 0)
    {
      parser->commit = g_strdup(g_strstrip(line+7));
    }
    else if(strncmp(line, "CommitDate:", 11) == 0)
    {
      parser->commit_date = g_strdup(g_strstrip(line+11));
    }
    else if(strncmp(line, "    ", 4) == 0)
    {
      if(parser->message)
        parser->message = g_strconcat(parser->message, line+4, NULL);
      else
        parser->message = g_strdup(line+4);
    }
    else if(g_ascii_isdigit(line[0]))
    {
      gchar *ptr, *path;
      TghLogFile *file;
      file = g_new(TghLogFile, 1);
      file->insertions = strtoul(line, &ptr, 10);
      file->deletions = strtoul(ptr, &path, 10);
      path++;
      file->file = g_strndup (path, strlen(path)-1);
      parser->files = g_slist_prepend (parser->files, file);
    }
  }
  else
  {
    if(parser->revision)
      log_parser_add_entry(parser, dialog);
    tgh_log_dialog_done(dialog);
    g_free(parser);
  }
}

TghOutputParser*
tgh_log_parser_new (GtkWidget *dialog)
{
  TghLogParser *parser = g_new0(TghLogParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(log_parser_func);

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

