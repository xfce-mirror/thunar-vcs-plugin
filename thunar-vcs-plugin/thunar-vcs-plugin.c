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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include <thunar-vcs-plugin/tvp-provider.h>

#ifdef HAVE_SUBVERSION
#include <thunar-vcs-plugin/tvp-svn-action.h>
#include <thunar-vcs-plugin/tvp-svn-property-page.h>
#endif

#ifdef HAVE_GIT
#include <thunar-vcs-plugin/tvp-git-action.h>
#endif



static GType type_list[1];



/* delcare it here to make the compiler happy */
G_MODULE_EXPORT void thunar_extension_initialize (ThunarxProviderPlugin *plugin);

G_MODULE_EXPORT void
thunar_extension_initialize (ThunarxProviderPlugin *plugin)
{
  const gchar *mismatch;

  /* verify that the thunarx versions are compatible */
  mismatch = thunarx_check_version (THUNARX_MAJOR_VERSION, THUNARX_MINOR_VERSION, THUNARX_MICRO_VERSION);
  if (G_UNLIKELY (mismatch != NULL))
    {
      g_warning ("Version mismatch: %s", mismatch);
      return;
    }

  /* setup i18n support */
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

#ifdef G_ENABLE_DEBUG
  g_message ("Initializing thunar-vcs-plugin extension");
#endif

  /* register the types provided by this plugin */
  tvp_provider_register_type (plugin);
#ifdef HAVE_SUBVERSION
  tvp_svn_action_register_type (plugin);
  tvp_svn_property_page_register_type (plugin);
#endif
#ifdef HAVE_GIT
  tvp_git_action_register_type (plugin);
#endif

  /* setup the plugin provider type list */
  type_list[0] = TVP_TYPE_PROVIDER;
}



/* delcare it here to make the compiler happy */
G_MODULE_EXPORT void thunar_extension_shutdown (void);

G_MODULE_EXPORT void
thunar_extension_shutdown (void)
{
#ifdef G_ENABLE_DEBUG
  g_message ("Shutting down thunar-vcs-plugin extension");
#endif
}



/* delcare it here to make the compiler happy */
G_MODULE_EXPORT void thunar_extension_list_types (const GType **types, gint *n_types);

G_MODULE_EXPORT void
thunar_extension_list_types (const GType **types,
                             gint         *n_types)
{
  *types = type_list;
  *n_types = G_N_ELEMENTS (type_list);
}
