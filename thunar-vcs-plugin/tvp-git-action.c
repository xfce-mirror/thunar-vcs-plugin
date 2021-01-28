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

#include <string.h>
#include <sys/wait.h>
#include <libxfce4util/libxfce4util.h>
#include <thunar-vcs-plugin/tvp-git-action.h>



static void tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data);



struct _TvpGitActionClass
{
    ThunarxMenuItemClass __parent__;
};



struct _TvpGitAction
{
    ThunarxMenuItem __parent__;

    struct {
        unsigned is_parent : 1;
        unsigned is_directory : 1;
        unsigned is_file : 1;
    } property;

    GList *files;
    GtkWidget *window;
};



enum {
    PROPERTY_IS_PARENT = 1,
    PROPERTY_IS_DIRECTORY,
    PROPERTY_IS_FILE
};



enum {
    SIGNAL_NEW_PROCESS = 0,
    SIGNAL_COUNT
};



static guint action_signal[SIGNAL_COUNT];



static void tvp_git_action_create_menu_item (ThunarxMenuItem *item);

static void tvp_git_action_finalize (GObject*);

static void tvp_git_action_set_property (GObject*, guint, const GValue*, GParamSpec*);


static GQuark tvp_action_arg_quark = 0;


static void tvp_action_exec (ThunarxMenuItem *item, TvpGitAction *tvp_action);

/* remove G_GNUC_UNUSED if this function is ever used, or the function itself if it is useless */
static void tvp_action_unimplemented (ThunarxMenuItem *, const gchar *) G_GNUC_UNUSED;



THUNARX_DEFINE_TYPE (TvpGitAction, tvp_git_action, THUNARX_TYPE_MENU_ITEM)



static void
tvp_git_action_class_init (TvpGitActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = tvp_git_action_finalize;
    gobject_class->set_property = tvp_git_action_set_property;

    g_object_class_install_property (gobject_class, PROPERTY_IS_PARENT,
            g_param_spec_boolean ("is-parent", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class, PROPERTY_IS_DIRECTORY,
            g_param_spec_boolean ("is-directory", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class, PROPERTY_IS_FILE,
            g_param_spec_boolean ("is-file", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

    action_signal[SIGNAL_NEW_PROCESS] = g_signal_new("new-process", G_OBJECT_CLASS_TYPE(gobject_class), G_SIGNAL_RUN_FIRST,
            0, NULL, NULL, tsh_cclosure_marshal_VOID__POINTER_STRING, G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_STRING);

    tvp_action_arg_quark = g_quark_from_string ("tvp-action-arg");
}



static void
tvp_git_action_init (TvpGitAction *self)
{
    self->property.is_parent = 0;
    self->files = NULL;
    self->window = NULL;
}



ThunarxMenuItem *
tvp_git_action_new (const gchar *name,
        const gchar *label,
        GList *files,
        GtkWidget *window,
        gboolean is_parent,
        gboolean is_direcotry,
        gboolean is_file)
{
    ThunarxMenuItem *item;

    g_return_val_if_fail(name, NULL);
    g_return_val_if_fail(label, NULL);

    item = g_object_new (TVP_TYPE_GIT_ACTION,
            "name", name,
            "label", label,
            "is-parent", is_parent,
            "is-directory", is_direcotry,
            "is-file", is_file,
            "icon", "git",
            NULL);
    TVP_GIT_ACTION (item)->files = thunarx_file_info_list_copy (files);
    TVP_GIT_ACTION (item)->window = window;

    tvp_git_action_create_menu_item (item);

    return item;
}



static void
tvp_git_action_finalize (GObject *object)
{
    thunarx_file_info_list_free (TVP_GIT_ACTION (object)->files);
    TVP_GIT_ACTION (object)->files = NULL;
    TVP_GIT_ACTION (object)->window = NULL;

    G_OBJECT_CLASS (tvp_git_action_parent_class)->finalize (object);
}



static void
tvp_git_action_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    switch (property_id)
    {
        case PROPERTY_IS_PARENT:
            TVP_GIT_ACTION (object)->property.is_parent = g_value_get_boolean (value)?1:0;
            break;
        case PROPERTY_IS_DIRECTORY:
            TVP_GIT_ACTION (object)->property.is_directory = g_value_get_boolean (value)?1:0;
            break;
        case PROPERTY_IS_FILE:
            TVP_GIT_ACTION (object)->property.is_file = g_value_get_boolean (value)?1:0;
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void
add_subaction(ThunarxMenuItem *item, ThunarxMenu *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *icon, gchar *arg)
{
    ThunarxMenuItem *subitem;
    subitem = thunarx_menu_item_new (name, text, tooltip, icon);
    thunarx_menu_append_item (menu, subitem);
    g_object_set_qdata (G_OBJECT (subitem), tvp_action_arg_quark, arg);
    g_signal_connect_after (subitem, "activate", G_CALLBACK (tvp_action_exec), item);
    g_object_unref (subitem);
}


static void
tvp_git_action_create_menu_item (ThunarxMenuItem *item)
{
    ThunarxMenu *menu;
    TvpGitAction *tvp_action = TVP_GIT_ACTION (item);

    menu = thunarx_menu_new ();
    thunarx_menu_item_set_menu (item, menu);

    add_subaction (item, menu, "tvp::git::add", _("Add"), _("Add file contents to the index"), "list-add", "--add");
    /* unimplemented: add_subaction(item, menu, "tvp::git::bisect", _("Bisect"), _("Bisect"), NULL, _("Bisect"));*/
    if (tvp_action->property.is_file)
        add_subaction (item, menu, "tvp::git::blame", _("Blame"), _("Show what revision and author last modified each line of a file"), "gtk-index", "--blame");
    if (tvp_action->property.is_parent)
        add_subaction (item, menu, "tvp::git::branch", _("Branch"), _("List, create or switch branches"), "media-playlist-shuffle", "--branch");
    /* unimplemented: add_subaction(item, menu, "tvp::git::checkout", _("Checkout"), _("Checkout"), "gtk-connect", _("Checkout"));*/
    add_subaction (item, menu, "tvp::git::clean", _("Clean"), _("Remove untracked files from the working tree"), "edit-clear", "--clean");
    if (tvp_action->property.is_parent)
        add_subaction (item, menu, "tvp::git::clone", _("Clone"), _("Clone a repository into a new directory"), "edit-copy", "--clone");
    /* unimplemented: add_subaction(item, menu, "tvp::git::commit", _("Commit"), _("Commit"), "gtk-apply", _("Commit"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::diff", _("Diff"), _("Diff"), "edit-find-replace", _("Diff"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::fetch", _("Fetch"), _("Fetch"), NULL, _("Fetch"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::grep", _("Grep"), _("Grep"), NULL, _("Grep"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::init", _("Init"), _("Init"), NULL, _("Init"));*/
    add_subaction (item, menu, "tvp::git::log", _("Log"), _("Show commit logs"), "gtk-index", "--log");
    /* unimplemented: add_subaction(item, menu, "tvp::git::merge", _("Merge"), _("Merge"), NULL, _("Merge"));*/
    if (!tvp_action->property.is_parent)
        add_subaction (item, menu, "tvp::git::move", _("Move"), _("Move or rename a file, a directory, or a symlink"), "gtk-dnd-multiple", "--move");
    /* unimplemented: add_subaction(item, menu, "tvp::git::pull", _("Pull"), _("Pull"), NULL, _("Pull"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::push", _("Push"), _("Push"), NULL, _("Push"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::rebase", _("Rebase"), _("Rebase"), NULL, _("Rebase"));*/
    add_subaction (item, menu, "tvp::git::reset", _("Reset"), _("Reset current HEAD to the specified state"), "edit-undo", "--reset");
    /* unimplemented: add_subaction(item, menu, "tvp::git::remove", _("Remove"), _("Remove"), "edit-delete", _("Remove"));*/
    /* unimplemented: add_subaction(item, menu, "tvp::git::show", _("Show"), _("Show"), NULL, _("Show"));*/
    if (tvp_action->property.is_parent)
        add_subaction (item, menu, "tvp::git::stash", _("Stash"), _("Stash the changes in a dirty working directory away"), "document-save", "--stash");
    if (tvp_action->property.is_parent)
        add_subaction (item, menu, "tvp::git::status", _("Status"), _("Show the working tree status"), "dialog-information", "--status");
    /* unimplemented: add_subaction(item, menu, "tvp::git::tag", _("Tag"), _("Tag"), NULL, _("Tag"));*/
}



static void tvp_action_unimplemented (ThunarxMenuItem *item, const gchar *action)
{
    GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), action);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
}


static void
tvp_setup_display_cb (gpointer data)
{
    g_setenv ("DISPLAY", (char *) data, TRUE);
}


static void tvp_action_exec (ThunarxMenuItem *item, TvpGitAction *tvp_action)
{
    guint size, i;
    gchar **argv;
    GList *iter;
    gchar *uri;
    gchar *filename;
    gchar *file;
    gchar *watch_path = NULL;
    gint pid;
    GError *error = NULL;
    char *display_name = NULL;
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tvp_action->window));
    GdkDisplay *display = gdk_screen_get_display (screen);

    iter = tvp_action->files;

    size = g_list_length (iter);

    argv = g_new (gchar *, size + 3);

    argv[0] = g_strdup (TVP_GIT_HELPER);
    argv[1] = g_strdup (g_object_get_qdata (G_OBJECT (item), tvp_action_arg_quark));
    argv[size + 2] = NULL;

    if(iter)
    {
        if(tvp_action->property.is_parent)
        {
            uri = thunarx_file_info_get_uri (iter->data);
            watch_path = g_filename_from_uri (uri, NULL, NULL);
            g_free (uri);
        }
        else
        {
            uri = thunarx_file_info_get_parent_uri (iter->data);
            watch_path = g_filename_from_uri (uri, NULL, NULL);
            g_free (uri);
        }
    }

    for (i = 0; i < size; i++)
    {
        /* determine the URI for the file info */
        uri = thunarx_file_info_get_uri (iter->data);
        if (G_LIKELY (uri != NULL))
        {
            /* determine the local filename for the URI */
            filename = g_filename_from_uri (uri, NULL, NULL);
            if (G_LIKELY (filename != NULL))
            {
                file = filename;
                /* strip the "file://" part of the uri */
                if (strncmp (file, "file://", 7) == 0)
                {
                    file += 7;
                }

                file = g_strdup (file);

                /* remove trailing '/' cause git can't handle that */
                if (strlen (file) > 1 && file[strlen (file) - 1] == '/')
                {
                    file[strlen (file) - 1] = '\0';
                }

                argv[i+2] = file;

                /* release the filename */
                g_free (filename);
            }

            /* release the URI */
            g_free (uri);
        }

        iter = g_list_next (iter);
    }

    pid = 0;
    if (screen != NULL)
        display_name = g_strdup (gdk_display_get_name (display));

    if (!g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, tvp_setup_display_cb, display_name, &pid, &error))
    {
        GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (tvp_action->window), GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Could not spawn \'" TVP_GIT_HELPER "\'");
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        g_error_free (error);
    }
    else
    {
        g_signal_emit(tvp_action, action_signal[SIGNAL_NEW_PROCESS], 0, &pid, watch_path);
    }

    g_free (display_name);
    g_free (watch_path);
    g_strfreev (argv);
}



static void
tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
        GValue       *return_value G_GNUC_UNUSED,
        guint         n_param_values,
        const GValue *param_values,
        gpointer      invocation_hint G_GNUC_UNUSED,
        gpointer      marshal_data)
{
    typedef void (*TshMarshalFunc_VOID__POINTER_STRING) (gpointer       data1,
            gconstpointer  arg_1,
            gconstpointer  arg_2,
            gpointer       data2);
    register TshMarshalFunc_VOID__POINTER_STRING callback;
    register GCClosure *cc = (GCClosure*) closure;
    register gpointer data1, data2;

    g_return_if_fail (n_param_values == 3);

    if (G_CCLOSURE_SWAP_DATA (closure))
    {
        data1 = closure->data;
        data2 = g_value_peek_pointer (param_values + 0);
    }
    else
    {
        data1 = g_value_peek_pointer (param_values + 0);
        data2 = closure->data;
    }
    callback = (TshMarshalFunc_VOID__POINTER_STRING) (marshal_data ? marshal_data : cc->callback);

    callback (data1,
            g_value_get_pointer (param_values + 1),
            g_value_get_string (param_values + 2),
            data2);
}

