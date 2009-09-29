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

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>

#include "tsh-common.h"
#include "tsh-add.h"
#include "tsh-blame.h"
//#include "tsh-changelist.h"
#include "tsh-checkout.h"
#include "tsh-cleanup.h"
#include "tsh-commit.h"
#include "tsh-copy.h"
#include "tsh-delete.h"
#include "tsh-export.h"
#include "tsh-import.h"
#include "tsh-lock.h"
#include "tsh-log.h"
#include "tsh-move.h"
#include "tsh-properties.h"
#include "tsh-resolved.h"
#include "tsh-relocate.h"
#include "tsh-revert.h"
#include "tsh-status.h"
#include "tsh-switch.h"
#include "tsh-unlock.h"
#include "tsh-update.h"

static GThread *thread = NULL;

void tsh_replace_thread (GThread *new_thread)
{
  if(thread)
    g_thread_join (thread);

  thread = new_thread;
}

int main (int argc, char *argv[])
{
	/* SVN variables */
	apr_pool_t *pool;
	svn_error_t *err;
	svn_client_ctx_t *svn_ctx;

	/* CMD-line options */
	gboolean print_version = FALSE;
	gboolean add = FALSE;
	gboolean blame = FALSE;
	gboolean changelist = FALSE;
	gboolean checkout = FALSE;
	gboolean cleanup = FALSE;
	gboolean commit = FALSE;
	gboolean copy = FALSE;
	gboolean delete = FALSE;
	gboolean export = FALSE;
	gboolean import = FALSE;
	gboolean lock = FALSE;
	gboolean log = FALSE;
	gboolean move = FALSE;
  gboolean properties = FALSE;
  gboolean resolved = FALSE;
  gboolean relocate = FALSE;
  gboolean revert = FALSE;
  gboolean status = FALSE;
  gboolean switch_ = FALSE;
	gboolean unlock = FALSE;
	gboolean update = FALSE;
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

	GOptionEntry blame_options_table[] =
	{
		{ "blame", '\0', 0, G_OPTION_ARG_NONE, &blame, N_("Execute blame action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry changelist_options_table[] =
	{
		{ "changelist", '\0', 0, G_OPTION_ARG_NONE, &changelist, N_("Execute changelist action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry checkout_options_table[] =
	{
		{ "checkout", '\0', 0, G_OPTION_ARG_NONE, &checkout, N_("Execute checkout action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry cleanup_options_table[] =
	{
		{ "cleanup", '\0', 0, G_OPTION_ARG_NONE, &cleanup, N_("Execute cleanup action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry commit_options_table[] =
	{
		{ "commit", '\0', 0, G_OPTION_ARG_NONE, &commit, N_("Execute commit action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry copy_options_table[] =
	{
		{ "copy", '\0', 0, G_OPTION_ARG_NONE, &copy, N_("Execute copy action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry delete_options_table[] =
	{
		{ "delete", '\0', 0, G_OPTION_ARG_NONE, &delete, N_("Execute delete action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry export_options_table[] =
	{
		{ "export", '\0', 0, G_OPTION_ARG_NONE, &export, N_("Execute export action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry import_options_table[] =
	{
		{ "import", '\0', 0, G_OPTION_ARG_NONE, &import, N_("Execute import action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry lock_options_table[] =
	{
		{ "lock", '\0', 0, G_OPTION_ARG_NONE, &lock, N_("Execute lock action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry log_options_table[] =
	{
		{ "log", '\0', 0, G_OPTION_ARG_NONE, &log, N_("Execute log action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry move_options_table[] =
	{
		{ "move", '\0', 0, G_OPTION_ARG_NONE, &move, N_("Execute move action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry properties_options_table[] =
	{
		{ "properties", '\0', 0, G_OPTION_ARG_NONE, &properties, N_("Execute properties action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry resolved_options_table[] =
	{
		{ "resolved", '\0', 0, G_OPTION_ARG_NONE, &resolved, N_("Execute resolved action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry relocate_options_table[] =
	{
		{ "relocate", '\0', 0, G_OPTION_ARG_NONE, &relocate, N_("Execute relocate action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry revert_options_table[] =
	{
		{ "revert", '\0', 0, G_OPTION_ARG_NONE, &revert, N_("Execute revert action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry status_options_table[] =
	{
		{ "status", '\0', 0, G_OPTION_ARG_NONE, &status, N_("Execute status action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry switch_options_table[] =
	{
		{ "switch", '\0', 0, G_OPTION_ARG_NONE, &switch_, N_("Execute switch action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry unlock_options_table[] =
	{
		{ "unlock", '\0', 0, G_OPTION_ARG_NONE, &unlock, N_("Execute unlock action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry update_options_table[] =
	{
		{ "update", '\0', 0, G_OPTION_ARG_NONE, &update, N_("Execute update action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

  /* setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  if (!g_thread_supported ())
    g_thread_init (NULL);
	gdk_threads_init ();
  gdk_threads_enter ();

	option_context = g_option_context_new("<action> [options] [args]");

	g_option_context_add_main_entries(option_context, general_options_table, GETTEXT_PACKAGE);
	g_option_context_add_group(option_context, gtk_get_option_group(TRUE));

	option_group = g_option_group_new("add", N_("Add Related Options:"), N_("Add"), NULL, NULL);
	g_option_group_add_entries(option_group, add_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("blame", N_("Blame Related Options:"), N_("Blame"), NULL, NULL);
	g_option_group_add_entries(option_group, blame_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("changelist", N_("Changelist Related Options:"), N_("Blame"), NULL, NULL);
	g_option_group_add_entries(option_group, changelist_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("checkout", N_("Checkout Related Options:"), N_("Checkout"), NULL, NULL);
	g_option_group_add_entries(option_group, checkout_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("cleanup", N_("Cleanup Related Options:"), N_("Cleanup"), NULL, NULL);
	g_option_group_add_entries(option_group, cleanup_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("commit", N_("Commit Related Options:"), N_("Commit"), NULL, NULL);
	g_option_group_add_entries(option_group, commit_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("copy", N_("Copy Related Options:"), N_("Copy"), NULL, NULL);
	g_option_group_add_entries(option_group, copy_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("delete", N_("Delete Related Options:"), N_("Delete"), NULL, NULL);
	g_option_group_add_entries(option_group, delete_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("export", N_("Export Related Options:"), N_("Export"), NULL, NULL);
	g_option_group_add_entries(option_group, export_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("import", N_("Import Related Options:"), N_("Import"), NULL, NULL);
	g_option_group_add_entries(option_group, import_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("lock", N_("Lock Related Options:"), N_("Lock"), NULL, NULL);
	g_option_group_add_entries(option_group, lock_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("log", N_("Log Related Options:"), N_("Log"), NULL, NULL);
	g_option_group_add_entries(option_group, log_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("move", N_("Move Related Options:"), N_("Move"), NULL, NULL);
	g_option_group_add_entries(option_group, move_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("properties", N_("Properties Related Options:"), N_("Properties"), NULL, NULL);
	g_option_group_add_entries(option_group, properties_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("resolved", N_("Resolved Related Options:"), N_("Resolved"), NULL, NULL);
	g_option_group_add_entries(option_group, resolved_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("relocate", N_("Relocate Related Options:"), N_("Relocate"), NULL, NULL);
	g_option_group_add_entries(option_group, relocate_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("revert", N_("Revert Related Options:"), N_("Revert"), NULL, NULL);
	g_option_group_add_entries(option_group, revert_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("status", N_("Status Related Options:"), N_("Status"), NULL, NULL);
	g_option_group_add_entries(option_group, status_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("switch", N_("Switch Related Options:"), N_("Switch"), NULL, NULL);
	g_option_group_add_entries(option_group, switch_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("unlock", N_("Unlock Related Options:"), N_("Unlock"), NULL, NULL);
	g_option_group_add_entries(option_group, unlock_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("update", N_("Update Related Options:"), N_("Update"), NULL, NULL);
	g_option_group_add_entries(option_group, update_options_table);
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

	if(!tsh_init(&pool, &err))
	{
		if(err)
		{
			svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
			svn_error_clear(err);
		}
		if(pool)
			svn_pool_destroy(pool);
		return EXIT_FAILURE;
	}

	if(!tsh_create_context(&svn_ctx, pool, &err))
	{
		if(err)
		{
			svn_handle_error2(err, stderr, FALSE, G_LOG_DOMAIN ": ");
			svn_error_clear(err);
		}
		svn_pool_destroy(pool);
		return EXIT_FAILURE;
	}

  if(add || blame || delete || revert || resolved || changelist)
  {
    if(!g_strv_length(files))
    {
      g_fprintf(stderr, "%s: %s\n\tTry --help-all\n", g_get_prgname(), _("Not enough arguments provided"));
      svn_pool_destroy(pool);
      return EXIT_FAILURE;
    }
  }

	if(add)
	{
		thread = tsh_add(files, svn_ctx, pool);
	}

	if(blame)
	{
		thread = tsh_blame(files, svn_ctx, pool);
	}

	if(changelist)
	{
		//thread = tsh_changelist(files, svn_ctx, pool);
	}

	if(checkout)
	{
		thread = tsh_checkout(files, svn_ctx, pool);
	}

	if(cleanup)
	{
		thread = tsh_cleanup(files, svn_ctx, pool);
	}

	if(commit)
	{
		thread = tsh_commit(files, svn_ctx, pool);
	}

	if(copy)
	{
		thread = tsh_copy(files, svn_ctx, pool);
	}

	if(delete)
	{
		thread = tsh_delete(files, svn_ctx, pool);
	}

	if(export)
	{
		thread = tsh_export(files, svn_ctx, pool);
	}

	if(import)
	{
		thread = tsh_import(files, svn_ctx, pool);
	}

	if(lock)
	{
		thread = tsh_lock(files, svn_ctx, pool);
	}

	if(log)
	{
		thread = tsh_log(files, svn_ctx, pool);
	}

	if(move)
	{
		thread = tsh_move(files, svn_ctx, pool);
	}

	if(properties)
	{
		thread = tsh_properties(files, svn_ctx, pool);
	}

	if(resolved)
	{
		thread = tsh_resolved(files, svn_ctx, pool);
	}

	if(relocate)
	{
		thread = tsh_relocate(files, svn_ctx, pool);
	}

	if(revert)
	{
		thread = tsh_revert(files, svn_ctx, pool);
	}

  if(status)
  {
    thread = tsh_status(files, svn_ctx, pool);
  }

  if(switch_)
  {
    thread = tsh_switch(files, svn_ctx, pool);
  }

	if(unlock)
	{
		thread = tsh_unlock(files, svn_ctx, pool);
	}

	if(update)
	{
		thread = tsh_update(files, svn_ctx, pool);
	}

	if(thread)
	{
		gtk_main ();

		g_thread_join (thread);
	}

	svn_pool_destroy(pool);

  gdk_threads_leave ();

	return EXIT_SUCCESS;
}

