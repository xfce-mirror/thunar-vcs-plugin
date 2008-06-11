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

#ifndef __TSH_COMMON_H__
#define __TSH_COMMON_H__

G_BEGIN_DECLS

gboolean tsh_init (apr_pool_t**, svn_error_t**);

void tsh_replace_thread (GThread *);
void tsh_cancel ();
void tsh_reset_cancel();

gboolean tsh_create_context (svn_client_ctx_t**, apr_pool_t*, svn_error_t**);

void         tsh_notify_func2  (void *, const svn_wc_notify_t *, apr_pool_t *);
void         tsh_status_func2  (void *, const char *, svn_wc_status2_t *);
svn_error_t *tsh_log_msg_func2 (const char **, const char **, const apr_array_header_t *, void *, apr_pool_t *);
svn_error_t *tsh_log_func      (void *, apr_hash_t *, svn_revnum_t, const char *, const char *, const char *, apr_pool_t *);
svn_error_t *tsh_blame_func    (void *, apr_int64_t, svn_revnum_t, const char *, const char *, const char *, apr_pool_t *);

gchar       *tsh_strerror  (svn_error_t *);

const gchar *tsh_status_to_string(enum svn_wc_status_kind status);

gchar *tsh_is_working_copy (const gchar *, apr_pool_t *);

G_END_DECLS

#endif /*__TSH_COMMON_H__*/

