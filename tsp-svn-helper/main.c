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
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>

#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>

#include "tsh-common.h"
#include "tsh-update.h"

int main (int argc, char *argv[])
{
	GThread *thread = NULL;

	/* SVN variables */
	apr_pool_t *pool;
	svn_error_t *err;
	svn_client_ctx_t *svn_ctx;

	/* CMD-line options */
	gboolean print_version = FALSE;
	gboolean checkout = FALSE;
	gboolean update = FALSE;
	gchar **files = NULL;

	GOptionEntry general_options_table[] =
	{
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &print_version, N_("Print version information"), NULL },
		{ G_OPTION_REMAINING, '\0', G_OPTION_ARG_FILENAME, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry checkout_options_table[] =
	{
		{ "checkout", '\0', 0, G_OPTION_ARG_NONE, &checkout, N_("Execute checkout action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionEntry update_options_table[] =
	{
		{ "update", '\0', 0, G_OPTION_ARG_NONE, &update, N_("Execute update action"), NULL },
		{ NULL, '\0', 0, 0, NULL, NULL, NULL }
	};

	GOptionGroup *option_group;
	GOptionContext *option_context = g_option_context_new("<action> [options] [args]");

	g_option_context_add_main_entries(option_context, general_options_table, GETTEXT_PACKAGE);
	g_option_context_add_group(option_context, gtk_get_option_group(TRUE));

	option_group = g_option_group_new("checkout", "Checkout Related Opions:", "Checkout", NULL, NULL);
	g_option_group_add_entries(option_group, checkout_options_table);
	g_option_context_add_group(option_context, option_group);

	option_group = g_option_group_new("update", "Update Related Opions:", "Update", NULL, NULL);
	g_option_group_add_entries(option_group, update_options_table);
	g_option_context_add_group(option_context, option_group);

	g_thread_init (NULL);
	gdk_threads_init ();

	g_option_context_parse(option_context, &argc, &argv, NULL);

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

	return EXIT_SUCCESS;
}

