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

#include <libxfce4util/libxfce4util.h>

#include "tgh-dialog-common.h"
#include "tgh-notify-dialog.h"
#include "tgh-status-dialog.h"
#include "tgh-log-dialog.h"
#include "tgh-branch-dialog.h"
#include "tgh-stash-dialog.h"
#include "tgh-blame-dialog.h"

#include "tgh-common.h"

static void
create_error_dialog (GtkWindow *parent, gchar *message)
{
  if (TGH_IS_NOTIFY_DIALOG (parent))
  {
    gchar **lines, **iter;
    lines = g_strsplit_set (message, "\r\n", -1);

    for (iter = lines; *iter; iter++)
    {
      gchar *action, *text;

      if ((*iter)[1])
      {
        text = *iter;
        action = strchr (text, ':');
        if (action)
        {
          *action = '\0';
          text = action+2;
          action = *iter;
        }

        tgh_notify_dialog_add (TGH_NOTIFY_DIALOG (parent), action, text);
      }
    }

    g_strfreev (lines);
  }
  else
  {
    GtkWidget *error;
    error = gtk_message_dialog_new (parent?GTK_WINDOW (parent):NULL, parent?GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL:0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Failed"));
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (error), "%s", g_strstrip (message));
    tgh_dialog_start (GTK_DIALOG (error), !parent);
  }
}

void
tgh_cancel(void)
{
}

static guint
path_compare (const gchar *path1, const gchar *path2)
{
  const gchar *i1, *i2;
  gchar c1, c2;
  gint count = 0;
  gint depth = 0;
  i1 = path1;
  i2 = path2;
  while ((c1 = *i1++))
  {
    c2 = *i2++;

    if (c1 != c2)
    {
      if (c1 == '/'&& !c2)
        depth = count;

      return depth;
    }

    if (c1 == '/')
      depth = count;

    count++;
  }

  if (*i2 == '/')
      depth = count;

  return depth;
}

gchar*
tgh_common_prefix (gchar **files)
{
  gchar **iter;
  gchar *prefix;
  guint prefix_len, match;

  if (files == NULL || files[0] == NULL)
    return NULL;

  prefix = g_strdup (files[0]);
  prefix_len = strlen (prefix);

  for (iter = &files[1]; *iter; iter++)
  {
    match = path_compare (prefix, *iter);
    prefix[match] = '\0';
    if (match == 0)
      break;
    prefix_len = match;
  }

  return prefix;
}

gchar**
tgh_strip_prefix (gchar **files, const gchar *prefix)
{
  gchar **stripped;
  guint len, i;
  guint prefix_len, start;

  if (files == NULL)
    return NULL;

  len = g_strv_length (files);
  stripped = g_new (gchar*, len + 1);
  stripped[len] = NULL;

  prefix_len = strlen (prefix);

  for (i = 0; i < len; i++)
  {
    if (G_LIKELY (g_str_has_prefix (files[i], prefix)))
    {
      start = prefix_len;
      while (files[i][start] == '/')
	start++;
      /* prefix is support to be a directory, so if the file fully matches is the current directory */
      if (G_UNLIKELY (files[i][start] == '\0'))
	stripped[i] = g_strdup (".");
      else
	stripped[i] = g_strdup (files[i] + start);
    }
    else
      stripped[i] = g_strdup (files[i]);
  }

  return stripped;
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
  if(WEXITSTATUS(status))
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
  gchar **parents;
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
      parser->parents,
      parser->author,
      parser->author_date,
      parser->commit,
      parser->commit_date,
      parser->message);

  parser->files = NULL;
  g_free(parser->revision);
  parser->revision = NULL;
  g_strfreev(parser->parents);
  parser->parents = NULL;
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
      gchar *revision, *parent;
      GSList *parent_list = NULL;
      guint parent_count = 0;

      if(parser->revision)
        log_parser_add_entry(parser, dialog);

      revision = g_strstrip (line+6);
      parent  = revision;

      while ((parent = strchr (parent, ' ')))
      {
        *parent++ = '\0';
        parent = g_strchug (parent);
        parent_list = g_slist_prepend (parent_list, parent);
        parent_count++;
      }

      parser->revision = g_strdup(revision);

      if (parent_count)
      {
        gchar **parents = g_new (char*, parent_count+1);
        parents[parent_count] = NULL;
        while (parent_list)
        {
          parents[--parent_count] = g_strdup (parent_list->data);
          parent_list = g_slist_delete_link (parent_list, parent_list);
        }
        parser->parents = parents;
      }
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

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
} TghStashListParser;

static void
stash_list_parser_func (TghStashListParser *parser, gchar *line)
{
  TghStashDialog *dialog = TGH_STASH_DIALOG (parser->dialog);
  if (line)
  {
    gchar *stash, *branch, *desc;
    branch = strchr (line, ':');
    *branch++ = '\0';
    stash = g_strstrip (line);
    desc = strchr (branch, ':');
    *desc++ = '\0';
    branch = g_strstrip (branch);
    desc = g_strstrip (desc);
    tgh_stash_dialog_add (dialog, stash, branch, desc);
  }
  else
  {
    tgh_stash_dialog_done (dialog);
    g_free (parser);
  }
}

TghOutputParser*
tgh_stash_list_parser_new (GtkWidget *dialog)
{
  TghStashListParser *parser = g_new (TghStashListParser,1);

  TGH_OUTPUT_PARSER (parser)->parse = TGH_OUTPUT_PARSER_FUNC (stash_list_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER (parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
} TghStashShowParser;

static void
stash_show_parser_func (TghStashShowParser *parser, gchar *line)
{
  TghStashDialog *dialog = TGH_STASH_DIALOG (parser->dialog);
  if (line)
  {
    gchar *ptr, *file;
    guint insertions = strtoul (line, &ptr, 10);
    guint deletions = strtoul (ptr, &file, 10);
    file++;
    file = g_strndup (file, strlen(file)-1);
    tgh_stash_dialog_add_file (dialog, insertions, deletions, file);
  }
  else
  {
    tgh_stash_dialog_done (dialog);
    g_free (parser);
  }
}

TghOutputParser*
tgh_stash_show_parser_new (GtkWidget *dialog)
{
  TghStashShowParser *parser = g_new (TghStashShowParser,1);

  TGH_OUTPUT_PARSER (parser)->parse = TGH_OUTPUT_PARSER_FUNC (stash_show_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER (parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
} TghBlameParser;

static void
blame_parser_func (TghBlameParser *parser, gchar *line)
{
  TghBlameDialog *dialog = TGH_BLAME_DIALOG (parser->dialog);
  if (line)
  {
    gchar *revision, *name, *date, *text, *ptr;
    guint64 line_no;

    name = strchr (line, '(');
    *name++ = '\0';

    revision = g_strstrip (line);

    text = strchr (name, ')');
    *text = '\0';
    text += 2;
    text[strlen (text)-1] = '\0';

    ptr = strrchr (name, ' ');
    line_no = g_ascii_strtoull (ptr, NULL, 10);

    while (*--ptr == ' ');
    ptr[1] = '\0';

    date = strrchr (name, ' ');
    *date = '\0';
    ptr = strrchr (name, ' ');
    *date = ' ';
    *ptr = '\0';
    date = strrchr (name, ' ');
    *ptr = ' ';
    *date++ = '\0';

    name = g_strstrip (name);

    tgh_blame_dialog_add (dialog, line_no, revision, name, date, text);
  }
  else
  {
    tgh_blame_dialog_done (dialog);
    g_free (parser);
  }
}

TghOutputParser*
tgh_blame_parser_new (GtkWidget *dialog)
{
  TghBlameParser *parser = g_new (TghBlameParser,1);

  TGH_OUTPUT_PARSER (parser)->parse = TGH_OUTPUT_PARSER_FUNC (blame_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER (parser);
}

typedef struct {
  TghOutputParser parent;
  GtkWidget *dialog;
} TghCleanParser;

static void
clean_parser_func(TghNotifyParser *parser, gchar *line)
{
  TghNotifyDialog *dialog = TGH_NOTIFY_DIALOG(parser->dialog);
  if(line)
  {
    gchar *action, *file;

    action = file = line;
    if (g_ascii_strncasecmp (line, "Would ", 6) == 0)
      file += 6;

    if (g_ascii_strncasecmp (file, "Not ", 4) == 0)
      file += 4;

    file = strchr (file, ' ');
    *file++ = '\0';
    file[strlen (file)-1] = '\0';

    tgh_notify_dialog_add(dialog, action, file);
  }
  else
  {
    tgh_notify_dialog_done(dialog);
    g_free(parser);
  }
}

TghOutputParser*
tgh_clean_parser_new (GtkWidget *dialog)
{
  TghCleanParser *parser = g_new(TghCleanParser,1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(clean_parser_func);

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

