/*-
 * Copyright (c) 2006 Peter de Ridder <peter@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <thunarx/thunarx.h>

#include <thunar-vfs/thunar-vfs.h>

#include <thunar-vcs-plugin/tvp-git-action.h>

#include <string.h>

#include <sys/wait.h>



static void tsh_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data);



struct _TvpGitActionClass
{
    GtkActionClass __parent__;
};



struct _TvpGitAction
{
    GtkAction __parent__;

    struct {
        unsigned is_parent : 1;
    } property;

    GList *files;
    GtkWidget *window;
};



enum {
    PROPERTY_IS_PARENT = 1,
};



enum {
    SIGNAL_NEW_PROCESS = 0,
    SIGNAL_COUNT
};



static guint action_signal[SIGNAL_COUNT];



static GtkWidget *tvp_git_action_create_menu_item (GtkAction *action);



static void tvp_git_action_finalize (GObject*);

static void tvp_git_action_set_property (GObject*, guint, const GValue*, GParamSpec*);


static GQuark tvp_action_arg_quark = 0;


static void tvp_action_exec (GtkAction *item, TvpGitAction *tvp_action);

static void tvp_action_unimplemented (GtkAction *, const gchar *);



THUNARX_DEFINE_TYPE (TvpGitAction, tvp_git_action, GTK_TYPE_ACTION)



static void
tvp_git_action_class_init (TvpGitActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

    gobject_class->finalize = tvp_git_action_finalize;
    gobject_class->set_property = tvp_git_action_set_property;

    gtkaction_class->create_menu_item = tvp_git_action_create_menu_item;

    g_object_class_install_property (gobject_class, PROPERTY_IS_PARENT,
            g_param_spec_boolean ("is-parent", "", "", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

    action_signal[SIGNAL_NEW_PROCESS] = g_signal_new("new-process", G_OBJECT_CLASS_TYPE(gobject_class), G_SIGNAL_RUN_FIRST,
            0, NULL, NULL, tsh_cclosure_marshal_VOID__POINTER_STRING, G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_STRING);

    tvp_action_arg_quark = g_quark_from_static_string ("tvp-action-arg");
}



static void
tvp_git_action_init (TvpGitAction *self)
{
    self->property.is_parent = 0;
    self->files = NULL;
    self->window = NULL;
}



GtkAction *
tvp_git_action_new (const gchar *name,
        const gchar *label,
        GList *files,
        GtkWidget *window,
        gboolean is_parent)
{
    g_return_val_if_fail(name, NULL);
    g_return_val_if_fail(label, NULL);

    GtkAction *action = g_object_new (TVP_TYPE_GIT_ACTION,
            "hide-if-empty", FALSE,
            "name", name,
            "label", label,
            "is-parent", is_parent,
#if !GTK_CHECK_VERSION(2,9,0)
            "stock-id", "git",
#else
            "icon-name", "git",
#endif
            NULL);
    TVP_GIT_ACTION (action)->files = thunarx_file_info_list_copy (files);
    //  TVP_GIT_ACTION (action)->window = gtk_widget_ref (window);
    TVP_GIT_ACTION (action)->window = window;
    return action;
}



static void
tvp_git_action_finalize (GObject *object)
{
    thunarx_file_info_list_free (TVP_GIT_ACTION (object)->files);
    TVP_GIT_ACTION (object)->files = NULL;
    //  gtk_widget_unref (TVP_GIT_ACTION (object)->window);
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
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void
add_subaction(GtkAction *action, GtkMenuShell *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *stock, gchar *arg)
{
    GtkAction *subaction;
    GtkWidget *subitem;

    subaction = gtk_action_new (name, text, tooltip, stock);
    g_object_set_qdata (G_OBJECT (subaction), tvp_action_arg_quark, arg);
    g_signal_connect_after (subaction, "activate", G_CALLBACK (tvp_action_exec), action);

    subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
    gtk_menu_shell_append (menu, subitem);
    gtk_widget_show(subitem);
}


static void
add_subaction_u (GtkMenuShell *menu, const gchar *name, const gchar *text, const gchar *tooltip, const gchar *stock, gchar *arg)
{
    GtkAction *subaction;
    GtkWidget *subitem;

    subaction = gtk_action_new (name, text, tooltip, stock);
    g_signal_connect_after (subaction, "activate", G_CALLBACK (tvp_action_unimplemented), arg);

    subitem = gtk_action_create_menu_item (subaction);
    g_object_get (G_OBJECT (subaction), "tooltip", &tooltip, NULL);
    gtk_widget_set_tooltip_text(subitem, tooltip);
    gtk_menu_shell_append (menu, subitem);
    gtk_widget_show(subitem);
}


static GtkWidget *
tvp_git_action_create_menu_item (GtkAction *action)
{
    GtkWidget *item;
    GtkWidget *menu;

    item = GTK_ACTION_CLASS(tvp_git_action_parent_class)->create_menu_item (action);

    menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);

    add_subaction (action, GTK_MENU_SHELL(menu), "tvp::add", Q_("Menu|Add"), _("Add"), GTK_STOCK_ADD, "--add");
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::bisect", Q_("Menu|Bisect"), _("Bisect"), NULL, _("Bisect"));
    add_subaction (action, GTK_MENU_SHELL(menu), "tvp::branch", Q_("Menu|Branch"), _("Branch"), NULL, "--branch");
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::checkout", Q_("Menu|Checkout"), _("Checkout"), GTK_STOCK_CONNECT, _("Checkout"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::clone", Q_("Menu|Clone"), _("Clone"), GTK_STOCK_COPY, _("Clone"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::commit", Q_("Menu|Commit"), _("Commit"), GTK_STOCK_APPLY, _("Commit"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::diff", Q_("Menu|Diff"), _("Diff"), GTK_STOCK_FIND_AND_REPLACE, _("Diff"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::fetch", Q_("Menu|Fetch"), _("Fetch"), NULL, _("Fetch"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::grep", Q_("Menu|Grep"), _("Grep"), NULL, _("Grep"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::init", Q_("Menu|Init"), _("Init"), NULL, _("Init"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::log", Q_("Menu|Log"), _("Log"), GTK_STOCK_INDEX, _("Log"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::merge", Q_("Menu|Merge"), _("Merge"), NULL, _("Merge"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::move", Q_("Menu|Move"), _("Move"), GTK_STOCK_DND_MULTIPLE, _("Move"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::pull", Q_("Menu|Pull"), _("Pull"), NULL, _("Pull"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::push", Q_("Menu|Push"), _("Push"), NULL, _("Push"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::rebase", Q_("Menu|Rebase"), _("Rebase"), NULL, _("Rebase"));
    add_subaction (action, GTK_MENU_SHELL(menu), "tvp::reset", Q_("Menu|Reset"), _("Reset"), GTK_STOCK_UNDO, "--reset");
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::remove", Q_("Menu|Remove"), _("Remove"), GTK_STOCK_DELETE, _("Remove"));
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::show", Q_("Menu|Show"), _("Show"), NULL, _("Show"));
    add_subaction (action, GTK_MENU_SHELL(menu), "tvp::status", Q_("Menu|Status"), _("Status"), GTK_STOCK_DIALOG_INFO, "--status");
    add_subaction_u(GTK_MENU_SHELL(menu), "tvp::tag", Q_("Menu|Tag"), _("Tag"), NULL, _("Tag"));

    return item;
}



static void tvp_action_unimplemented (GtkAction *item, const gchar *tvp_action)
{
    GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Action %s is unimplemented"), tvp_action);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
}



static void tvp_action_exec (GtkAction *item, TvpGitAction *tvp_action)
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
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (tvp_action->window));

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
                if (file[strlen (file) - 1] == '/')
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
    if (!gdk_spawn_on_screen (screen, NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &error))
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

