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

#ifndef __TSH_COMMON_H__
#define __TSH_COMMON_H__

#include <subversion-1/svn_client.h>
#include <subversion-1/svn_types.h>
#include <subversion-1/svn_version.h>

G_BEGIN_DECLS

/* typdef from tsh-blame-dialog.h */
typedef struct _TshBlameDialog      TshBlameDialog;
struct tsh_blame_baton
{
  TshBlameDialog *dialog;
  svn_revnum_t start_revnum;
  svn_revnum_t end_revnum;
};

gboolean tsh_init (apr_pool_t**, svn_error_t**);

void tsh_replace_thread (GThread *);
void tsh_cancel (void);
void tsh_reset_cancel(void);

gboolean tsh_create_context (svn_client_ctx_t**, apr_pool_t*, svn_error_t**);

void         tsh_notify_func2  (void *, const svn_wc_notify_t *, apr_pool_t *);
void         tsh_status_func2  (void *, const char *, svn_wc_status2_t *);
svn_error_t *tsh_status_func3  (void *, const char *, svn_wc_status2_t *, apr_pool_t *);
svn_error_t *tsh_status_func   (void *, const char *, const svn_client_status_t *, apr_pool_t *);
svn_error_t *tsh_log_msg_func2 (const char **, const char **, const apr_array_header_t *, void *, apr_pool_t *);
svn_error_t *tsh_log_func      (void *, svn_log_entry_t *, apr_pool_t *);
svn_error_t *tsh_blame_func2   (void *, apr_int64_t, svn_revnum_t, const char *, const char *, svn_revnum_t, const char *, const char *, const char *, const char *, apr_pool_t *);
svn_error_t *tsh_blame_func3   (void *, svn_revnum_t, svn_revnum_t, apr_int64_t, svn_revnum_t, apr_hash_t *, svn_revnum_t, apr_hash_t *, const char *, const char *, svn_boolean_t, apr_pool_t *);
svn_error_t *tsh_blame_func4   (void *, apr_int64_t, svn_revnum_t, apr_hash_t *, svn_revnum_t, apr_hash_t *, const char *, const svn_string_t *, svn_boolean_t, apr_pool_t *);
svn_error_t *tsh_proplist_func (void *, const char *, apr_hash_t *, apr_pool_t *);
svn_error_t *tsh_commit_func2  (const svn_commit_info_t *, void *, apr_pool_t *);

gchar       *tsh_strerror  (svn_error_t *);

const gchar *tsh_status_to_string(enum svn_wc_status_kind status);

gchar *tsh_is_working_copy (const gchar *, apr_pool_t *);

#define CHECK_SVN_VERSION(major, minor) \
    (SVN_VER_MAJOR == (major) && SVN_VER_MINOR == (minor))
#define CHECK_SVN_VERSION_G(major, minor) \
    (SVN_VER_MAJOR > (major) || \
    (SVN_VER_MAJOR == (major) && SVN_VER_MINOR > (minor)) || \
    (SVN_VER_MAJOR == (major) && SVN_VER_MINOR == (minor)))
#define CHECK_SVN_VERSION_S(major, minor) \
    ((major > SVN_VER_MAJOR) || \
    ((major == SVN_VER_MAJOR) && (minor >= SVN_VER_MINOR)))

G_END_DECLS

#endif /*__TSH_COMMON_H__*/

