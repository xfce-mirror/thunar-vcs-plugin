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

#include <apr_lib.h>

#include <subversion-1/svn_cmdline.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_fs.h>

#include "tsh-dialog-common.h"
#include "tsh-login-dialog.h"
#include "tsh-file-dialog.h"
#include "tsh-trust-dialog.h"
#include "tsh-notify-dialog.h"

#include "tsh-common.h"

static svn_error_t* tsh_auth_simple_prompt(svn_auth_cred_simple_t**, void*, const char*, const char*, svn_boolean_t, apr_pool_t*);
static svn_error_t* tsh_auth_username_prompt(svn_auth_cred_username_t**, void*, const char*, svn_boolean_t, apr_pool_t*);
static svn_error_t* tsh_auth_ssl_server_trust_prompt(svn_auth_cred_ssl_server_trust_t**, void*, const char*, apr_uint32_t, const svn_auth_ssl_server_cert_info_t*, svn_boolean_t, apr_pool_t*);
static svn_error_t* tsh_auth_ssl_client_cert_prompt(svn_auth_cred_ssl_client_cert_t**, void*, const char*, svn_boolean_t, apr_pool_t*);
static svn_error_t* tsh_auth_ssl_client_cert_pw_prompt(svn_auth_cred_ssl_client_cert_pw_t**, void*, const char*, svn_boolean_t, apr_pool_t*);

static svn_error_t* tsh_check_cancel(void*);

static gboolean cancelled = FALSE;

gboolean tsh_init (apr_pool_t **ppool, svn_error_t **perr)
{
	apr_pool_t *pool;
	svn_error_t *err;

	if (perr)
		*perr = NULL;

	if (!ppool)
		return FALSE;

	/* Initialize svn stuff */
	if (svn_cmdline_init (G_LOG_DOMAIN, stderr) != EXIT_SUCCESS)
	{
		*ppool = NULL;
		return FALSE;
	}

	*ppool = pool = svn_pool_create (NULL);

	/* Initialize utf routines */
	svn_utf_initialize (pool);

	/* Initialize FS lib */
	if ((err = svn_fs_initialize (pool)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}

	if ((err = svn_config_ensure (NULL, pool)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}

#ifdef G_OS_WIN32
	/* Set the working copy administrative directory name */
	if (getenv ("SVN_ASP_DOT_NET_HACK"))
	{
		if(err = svn_wc_set_adm_dir ("_svn", pool))
		{
			if (perr)
				*perr = err;
			return FALSE;
		}
	}
#endif

	return TRUE;
}

gboolean tsh_create_context (svn_client_ctx_t **pctx, apr_pool_t *pool, svn_error_t **perr)
{
	svn_auth_provider_object_t *provider;
	apr_array_header_t *providers;
	svn_error_t *err;
	svn_boolean_t cache = TRUE;
	svn_auth_baton_t *ab;
	svn_config_t *cfg;
	svn_client_ctx_t *ctx;

	if (perr)
		*perr = NULL;

	if (!pctx)
		return FALSE;

	if (!pool)
		return FALSE;

	/* Create the client context */
	if ((err = svn_client_create_context (pctx, pool)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}
	ctx = *pctx;

	/* Get the config */
  if ((err = svn_config_get_config(&(ctx->config), NULL, pool)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}
  cfg = apr_hash_get(ctx->config, SVN_CONFIG_CATEGORY_CONFIG,
                     APR_HASH_KEY_STRING);

	/* Set cancel funvtion */
	ctx->cancel_func = tsh_check_cancel;

	/* Create an array to hold the providers */
	providers = apr_array_make (pool, 12, sizeof (svn_auth_provider_object_t *));

	/* Disk caching auth providers */
#ifdef G_OS_WIN32
	svn_auth_get_windows_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
#endif
#ifdef SVN_HAVE_KEYCHAIN_SERVICES
	svn_auth_get_keychain_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
#endif
	svn_auth_get_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_username_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* Cert auth providers */
	svn_auth_get_ssl_server_trust_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_pw_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* Prompt providers */
	svn_auth_get_simple_prompt_provider (&provider,
	                                     tsh_auth_simple_prompt,
	                                     NULL,
	                                     2,
	                                     pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_username_prompt_provider (&provider,
	                                       tsh_auth_username_prompt,
	                                       NULL,
	                                       2,
	                                       pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	svn_auth_get_ssl_server_trust_prompt_provider (&provider,
	                                               tsh_auth_ssl_server_trust_prompt,
	                                               NULL,
	                                               pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_prompt_provider (&provider,
	                                              tsh_auth_ssl_client_cert_prompt,
	                                              NULL,
																								2,
	                                              pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_pw_prompt_provider (&provider,
	                                                 tsh_auth_ssl_client_cert_pw_prompt,
	                                                 NULL,
																									 2,
	                                                 pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* open the auth baton */
  svn_auth_open(&ab, providers, pool);
	ctx->auth_baton = ab;

	/* Check config for cache settings */
	if ((err = svn_config_get_bool (cfg, &cache,
	                                SVN_CONFIG_SECTION_AUTH,
																	SVN_CONFIG_OPTION_STORE_PASSWORDS,
																	TRUE)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}

  if (!cache)
    svn_auth_set_parameter(ab, SVN_AUTH_PARAM_DONT_STORE_PASSWORDS, "");

	/* Check config for cache settings */
	if ((err = svn_config_get_bool (cfg, &cache,
	                                SVN_CONFIG_SECTION_AUTH,
																	SVN_CONFIG_OPTION_STORE_AUTH_CREDS,
																	TRUE)))
	{
		if (perr)
			*perr = err;
		return FALSE;
	}

  if (!cache)
    svn_auth_set_parameter(ab, SVN_AUTH_PARAM_NO_AUTH_CACHE, "");

	return TRUE;
}

static svn_error_t*
tsh_auth_simple_prompt(svn_auth_cred_simple_t **cred,
                       void *baton,
                       const char *realm,
                       const char *username,
                       svn_boolean_t may_save,
                       apr_pool_t *pool)
{
	if(!username)
		username = "";

	gdk_threads_enter();

	GtkWidget *dialog = tsh_login_dialog_new(NULL, NULL, 0, username, TRUE, may_save);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gdk_threads_leave();

		*cred = NULL;

		gtk_widget_destroy(dialog);

		cancelled = TRUE;
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	}

	gdk_threads_leave();

  svn_auth_cred_simple_t *ret = apr_pcalloc(pool, sizeof(svn_auth_cred_simple_t));
	TshLoginDialog *login_dialog = TSH_LOGIN_DIALOG(dialog);
	ret->username = apr_pstrdup(pool, tsh_login_dialog_get_username(login_dialog));
	ret->password = apr_pstrdup(pool, tsh_login_dialog_get_password(login_dialog));
	ret->may_save = tsh_login_dialog_get_may_save(login_dialog);
	*cred = ret;

	gtk_widget_destroy(dialog);

	return SVN_NO_ERROR;
}

static svn_error_t*
tsh_auth_username_prompt(svn_auth_cred_username_t **cred,
                                             void *baton,
                                             const char *realm,
                                             svn_boolean_t may_save,
                                             apr_pool_t *pool)
{
	gdk_threads_enter();

	GtkWidget *dialog = tsh_login_dialog_new(NULL, NULL, 0, "", FALSE, may_save);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gdk_threads_leave();

		*cred = NULL;

		gtk_widget_destroy(dialog);

		cancelled = TRUE;
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	}

	gdk_threads_leave();

  svn_auth_cred_username_t *ret = apr_pcalloc(pool, sizeof(svn_auth_cred_username_t));
	TshLoginDialog *login_dialog = TSH_LOGIN_DIALOG(dialog);
	ret->username = apr_pstrdup(pool, tsh_login_dialog_get_username(login_dialog));
	ret->may_save = tsh_login_dialog_get_may_save(login_dialog);
	*cred = ret;

	gtk_widget_destroy(dialog);

	return SVN_NO_ERROR;
}

static svn_error_t*
tsh_auth_ssl_server_trust_prompt(svn_auth_cred_ssl_server_trust_t **cred,
                                 void *baton,
                                 const char *realm,
                                 apr_uint32_t failures,
                                 const svn_auth_ssl_server_cert_info_t *cert_info,
                                 svn_boolean_t may_save,
                                 apr_pool_t *pool)
{
	gdk_threads_enter();

	GtkWidget *dialog = tsh_trust_dialog_new(NULL, NULL, 0, failures, may_save);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gdk_threads_leave();

		*cred = NULL;

		gtk_widget_destroy(dialog);

		cancelled = TRUE;
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	}

	gdk_threads_leave();

  svn_auth_cred_ssl_server_trust_t *ret = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_server_trust_t));
	TshTrustDialog *trust_dialog = TSH_TRUST_DIALOG(dialog);
	ret->may_save = tsh_trust_dialog_get_may_save(trust_dialog);
	ret->accepted_failures = tsh_trust_dialog_get_accepted(trust_dialog);
	*cred = ret;

	gtk_widget_destroy(dialog);

	return SVN_NO_ERROR;
}

static svn_error_t*
tsh_auth_ssl_client_cert_prompt(svn_auth_cred_ssl_client_cert_t **cred,
                                void *baton,
                                const char *realm,
                                svn_boolean_t may_save,
                                apr_pool_t *pool)
{
	gdk_threads_enter();

	GtkWidget *dialog = tsh_file_dialog_new(NULL, NULL, 0, may_save);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gdk_threads_leave();

		*cred = NULL;

		gtk_widget_destroy(dialog);

		cancelled = TRUE;
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	}

	gdk_threads_leave();

  svn_auth_cred_ssl_client_cert_t *ret = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_client_cert_t));
	TshFileDialog *file_dialog = TSH_FILE_DIALOG(dialog);
	ret->cert_file = apr_pstrdup(pool, tsh_file_dialog_get_filename(file_dialog));
	ret->may_save = tsh_file_dialog_get_may_save(file_dialog);
	*cred = ret;

	gtk_widget_destroy(dialog);

	return SVN_NO_ERROR;
}

static svn_error_t*
tsh_auth_ssl_client_cert_pw_prompt(svn_auth_cred_ssl_client_cert_pw_t **cred,
                                   void *baton,
                                   const char *realm,
                                   svn_boolean_t may_save,
                                   apr_pool_t *pool)
{
	gdk_threads_enter();

	GtkWidget *dialog = tsh_login_dialog_new(NULL, NULL, 0, NULL, TRUE, may_save);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gdk_threads_leave();

		*cred = NULL;

		gtk_widget_destroy(dialog);

		cancelled = TRUE;
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	}

	gdk_threads_leave();

  svn_auth_cred_ssl_client_cert_pw_t *ret = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_client_cert_pw_t));
	TshLoginDialog *login_dialog = TSH_LOGIN_DIALOG(dialog);
	ret->password = apr_pstrdup(pool, tsh_login_dialog_get_password(login_dialog));
	ret->may_save = tsh_login_dialog_get_may_save(login_dialog);
	*cred = ret;

	gtk_widget_destroy(dialog);

	return SVN_NO_ERROR;
}

static svn_error_t*
tsh_check_cancel(void *baton)
{
	if(cancelled)
		return svn_error_create(SVN_ERR_CANCELLED, NULL, NULL);
	return SVN_NO_ERROR;
}

void
tsh_notify_func2(void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool)
{
	TshNotifyDialog *dialog = TSH_NOTIFY_DIALOG (baton);
	char buffer[256];

	switch(notify->action)
	{
		case svn_wc_notify_update_delete:
			gdk_threads_enter();
			tsh_notify_dialog_add(dialog, _("Deleted"), notify->path, notify->mime_type);
			gdk_threads_leave();
			break;
		case svn_wc_notify_update_add:
			gdk_threads_enter();
			tsh_notify_dialog_add(dialog, _("Added"), notify->path, notify->mime_type);
			gdk_threads_leave();
			break;
		case svn_wc_notify_update_update:
			gdk_threads_enter();
			tsh_notify_dialog_add(dialog, _("Updated"), notify->path, notify->mime_type);
			gdk_threads_leave();
			break;
		case svn_wc_notify_update_completed:
			g_snprintf(buffer, 256, _("At revision: %li"), notify->revision);
			gdk_threads_enter();
			tsh_notify_dialog_add(dialog, _("Completed"), buffer, NULL);
			gdk_threads_leave();
			break;
		case svn_wc_notify_update_external:
			gdk_threads_enter();
			tsh_notify_dialog_add(dialog, _("External"), notify->path, notify->mime_type);
			gdk_threads_leave();
			break;
		default:
			break;
	}
}

