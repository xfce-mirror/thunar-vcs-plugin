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

#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>

#include "tgh-common.h"

#include "tgh-add.h"
#include "tgh-branch.h"
#include "tgh-clone.h"
#include "tgh-log.h"
#include "tgh-reset.h"
#include "tgh-stash.h"
#include "tgh-status.h"

static GPid pid;
static gboolean has_child = FALSE;

void tgh_replace_child (gboolean new_child, GPid new_pid)
{
  if(has_child)
    g_spawn_close_pid(pid);

  has_child = new_child;
  pid = new_pid;
}

int main (int argc, char *argv[])
{
  /* CMD-line options */
  gboolean print_version = FALSE;
  gboolean add = FALSE;
  gboolean branch = FALSE;
  gboolean clone = FALSE;
  gboolean log = FALSE;
  gboolean reset = FALSE;
  gboolean stash = FALSE;
  gboolean status = FALSE;
  gchar **files = NULL;
  GError *error = NULL;

  GOptionGroup *option_group;
  GOptionContext *option_context;

  GOptionEntry general_options_table[] =
  {
    { "version", 'v', 0, G_OPTION_ARG_NONE, &print_version, N_("Print version information"), NULL },
    { G_OPTION_REMAINING, '\0', G_OPTION_ARG_FILENAME, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry add_options_table[] =
  {
    { "add", '\0', 0, G_OPTION_ARG_NONE, &add, N_("Execute add action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry branch_options_table[] =
  {
    { "branch", '\0', 0, G_OPTION_ARG_NONE, &branch, N_("Execute branch action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry clone_options_table[] =
  {
    { "clone", '\0', 0, G_OPTION_ARG_NONE, &clone, N_("Execute clone action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry log_options_table[] =
  {
    { "log", '\0', 0, G_OPTION_ARG_NONE, &log, N_("Execute log action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry reset_options_table[] =
  {
    { "reset", '\0', 0, G_OPTION_ARG_NONE, &reset, N_("Execute reset action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry stash_options_table[] =
  {
    { "stash", '\0', 0, G_OPTION_ARG_NONE, &stash, N_("Execute stash action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionEntry status_options_table[] =
  {
    { "status", '\0', 0, G_OPTION_ARG_NONE, &status, N_("Execute status action"), NULL },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  /* setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  option_context = g_option_context_new("<action> [options] [args]");

  g_option_context_add_main_entries(option_context, general_options_table, GETTEXT_PACKAGE);
  g_option_context_add_group(option_context, gtk_get_option_group(TRUE));

  option_group = g_option_group_new("add", N_("Add Related Options:"), N_("Add"), NULL, NULL);
  g_option_group_add_entries(option_group, add_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("branch", N_("Branch Related Options:"), N_("Branch"), NULL, NULL);
  g_option_group_add_entries(option_group, branch_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("clone", N_("Clone Related Options:"), N_("Clone"), NULL, NULL);
  g_option_group_add_entries(option_group, clone_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("log", N_("Log Related Options:"), N_("Log"), NULL, NULL);
  g_option_group_add_entries(option_group, log_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("reset", N_("Reset Related Options:"), N_("Reset"), NULL, NULL);
  g_option_group_add_entries(option_group, reset_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("stash", N_("Stash Related Options:"), N_("Stash"), NULL, NULL);
  g_option_group_add_entries(option_group, stash_options_table);
  g_option_context_add_group(option_context, option_group);

  option_group = g_option_group_new("status", N_("Status Related Options:"), N_("Status"), NULL, NULL);
  g_option_group_add_entries(option_group, status_options_table);
  g_option_context_add_group(option_context, option_group);

  if(!g_option_context_parse(option_context, &argc, &argv, &error))
  {
    g_fprintf(stderr, "%s: %s\n\tTry --help-all\n", g_get_prgname(), error->message);
    g_error_free(error);
  }

  if(print_version)
  {
    g_print(PACKAGE_STRING "\n\tcompiled on " __DATE__ ", " __TIME__ "\n");
    return EXIT_SUCCESS;
  }

  if(add)
  {
    has_child = tgh_add(files, &pid);
  }

  if(branch)
  {
    has_child = tgh_branch(files, &pid);
  }

  if(clone)
  {
    has_child = tgh_clone(files, &pid);
  }

  if(log)
  {
    has_child = tgh_log(files, &pid);
  }

  if(reset)
  {
    has_child = tgh_reset(files, &pid);
  }

  if(stash)
  {
    has_child = tgh_stash(files, &pid);
  }

  if(status)
  {
    has_child = tgh_status(files, &pid);
  }

  if(has_child)
  {
    gtk_main ();

    tgh_replace_child(FALSE, 0);
  }

  return EXIT_SUCCESS;
}

