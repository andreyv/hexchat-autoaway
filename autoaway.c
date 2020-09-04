/*
 * AutoAway -- a HexChat plugin to set away on idle
 *
 * Copyright (C) 2013-2016 Andrey Vihrov <andrey.vihrov@gmail.com>
 *
 * Based on ideas from
 * https://robots.org.uk/src/xchat/idle.c (Copyright 2004 Sam Morris)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <hexchat-plugin.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

/* Plugin identification */

#define PNAME    "AutoAway"
#define PDESC    "Set away on idle"
#define PVERSION "2.0"

/* Plugin settings */

#define MAX_LEN 511 /* Maximum IRC message length (RFC 2812 section 2.3) plus one */
#define MAX_NET 50  /* Maximum number of network */

static char away_msg[MAX_LEN]   = "Idle";
static char away_extra[MAX_LEN] = "";
static char back_extra[MAX_LEN] = "";
static int  polling_timeout     = 10;      /* Seconds */
static int  idle_time           = 10 * 60; /* Seconds */
static char  away_nick_suffix[MAX_LEN] = "";
static char  away_nick_net[MAX_LEN]    = "";
static char *away_nick_net_list[MAX_NET];
static char  orig_nick_list[MAX_NET][MAX_LEN];

#define PREF_AWAY_MSG           "away_msg"
#define PREF_AWAY_EXTRA         "away_extra"
#define PREF_BACK_EXTRA         "back_extra"
#define PREF_POLLING_TIMEOUT    "polling_timeout"
#define PREF_IDLE_TIME          "idle_time"

/* Server specific away nick change
 * Some servers perfer you change the nick name when away
 * other servers (e.g. freenode) hate it.
 *
 * away_nick_suffix: The nick suffix to append to nick
 * away_nick_net: ',' seperated networks that allows away nick change
 */
#define PREF_AWAY_NICK_SUFFIX "away_nick_suffix"
#define PREF_AWAY_NICK_NET    "away_nick_net"

/* Old settings that are not used anymore */
#define PREF_AWAY_CMD           "away_cmd"
#define PREF_BACK_CMD           "back_cmd"

/* Global variables */

static hexchat_plugin   *ph;

static Display          *display;
static XScreenSaverInfo *ssinfo;
static int               away_nick_net_count = 0;

/* RFC 2812 and HexChat constants */

#define RPL_UNAWAY           "305"
#define RPL_NOWAWAY          "306"

#define HC_TYPE_SERVER       1
#define HC_FLAGS_CONNECTED   0x1
#define HC_FLAGS_MARKED_AWAY 0x4

/* Logging */

#define LOG_ERR  "error"
#define LOG_WARN "warning"
#define LOG_DBG  "debug"
#define LOG(level, ...) (hexchat_printf(ph, PNAME ": " level ": " __VA_ARGS__))
#ifndef NDEBUG
#  define DEBUG(...) (LOG(LOG_DBG, __VA_ARGS__))
#else
#  define DEBUG(...) ((void)0)
#endif

static int
away_nick_net_find(hexchat_list *ch)
{
    for (int i = 0; i < away_nick_net_count; i++)

    {
        const char *network = hexchat_list_str(ph, ch, "network");
        DEBUG("away_nick_net_find: current:%s away_nick_net_list[%d]=%s", network, i,
              away_nick_net_list[i]);
        if (strcmp(network, away_nick_net_list[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void
set_away (bool away)
{
    hexchat_list *ch = hexchat_list_get(ph, "channels");
    assert(ch);

    /* Loop over all channels, find servers, and update state for each server */
    while (hexchat_list_next(ph, ch))
    {
        if (hexchat_list_int(ph, ch, "type") == HC_TYPE_SERVER)
        {
            int flags = hexchat_list_int(ph, ch, "flags");
            DEBUG("server: %s, flags: 0x%x", hexchat_list_str(ph, ch, "server"), flags);
            if (!(flags & HC_FLAGS_CONNECTED)
                || away == (bool)(flags & HC_FLAGS_MARKED_AWAY))
            {
                continue;
            }

            hexchat_context *ctx = (hexchat_context *)hexchat_list_str(ph, ch, "context");
            assert(ctx);

            if (!hexchat_set_context(ph, ctx))
            {
                LOG(LOG_WARN, "failed to set context for server %s",
                    hexchat_list_str(ph, ch, "server"));
                continue;
            }

            int net_index = away_nick_net_find(ch);
            if (away)
            {
                DEBUG("set away network:%s", hexchat_list_str(ph, ch, "network"));
                /* Change nick if it is in away_nick_net */
                if (*away_nick_suffix && (net_index >= 0))
                {
                    strncpy(orig_nick_list[net_index], hexchat_get_info(ph, "nick"), MAX_LEN);
                    DEBUG("orig_nick: %s", orig_nick_list[net_index]);
                    hexchat_commandf(ph, "NICK %s%s", orig_nick_list[net_index], away_nick_suffix);
                }
                hexchat_commandf(ph, "AWAY %s", away_msg);
            }
            else
            {
                DEBUG("set back network:%s", hexchat_list_str(ph, ch, "network"));
                /* Change nick if it is in away_nick_net */
                if (*away_nick_suffix && (net_index >= 0))
                {
                    hexchat_commandf(ph, "NICK %s", orig_nick_list[net_index]);
                }
                hexchat_command(ph, "BACK");
            }
        }
    }

    hexchat_list_free(ph, ch);
}

static int
check_idle (void *unused)
{
    (void)unused;

    if (!XScreenSaverQueryInfo(display, DefaultRootWindow(display), ssinfo))
    {
        LOG(LOG_ERR, "XScreenSaverQueryInfo() failed");
        return 1;
    }

    if (ssinfo->state == ScreenSaverOn
        || ssinfo->idle / 1000 >= (unsigned long)idle_time)
    {
        DEBUG("idle");
        set_away(true);
    }
    else
    {
        DEBUG("not idle");
        set_away(false);
    }

    return 1;
}

static int
send_extra_cmd (char *word[], char *word_eol[], void *user_data)
{
    (void)word; (void)word_eol;

    hexchat_command(ph, user_data);

    return HEXCHAT_EAT_NONE;
}

void
hexchat_plugin_get_info (char **name, char **desc,
                         char **version, void **reserved)
{
    *name    = PNAME;
    *desc    = PDESC;
    *version = PVERSION;

    (void)reserved;
}

int
hexchat_plugin_init (hexchat_plugin *plugin_handle,
                     char **plugin_name, char **plugin_desc,
                     char **plugin_version, char *arg)
{
    /* Init variables */

    ph = plugin_handle;

    *plugin_name    = PNAME;
    *plugin_desc    = PDESC;
    *plugin_version = PVERSION;

    (void)arg;

    /* Load settings */

    (void)hexchat_pluginpref_get_str(ph, PREF_AWAY_MSG, away_msg);
    (void)hexchat_pluginpref_get_str(ph, PREF_AWAY_EXTRA, away_extra);
    (void)hexchat_pluginpref_get_str(ph, PREF_BACK_EXTRA, back_extra);
    (void)hexchat_pluginpref_get_str(ph, PREF_AWAY_NICK_SUFFIX, away_nick_suffix);
    (void)hexchat_pluginpref_get_str(ph, PREF_AWAY_NICK_NET, away_nick_net);

    /* Parse the server list */
    bool string_mode = false;
    for (int i = 0; i < MAX_LEN && away_nick_net[i] != '\0'; i++)
    {
        switch (away_nick_net[i])
        {
        case ' ':
            if (!string_mode)
            {
                away_nick_net[i] = '\0';
            }
            break;
        case ',':
            string_mode      = false;
            away_nick_net[i] = '\0';
            break;
        default:
            if (!string_mode)
            {
                string_mode                               = true;
                away_nick_net_list[away_nick_net_count++] = &(away_nick_net[i]);
            }
            break;
        }
    }

    if (*away_nick_suffix)
    {
        // strncpy(orig_nick, hexchat_get_info(ph, "network"), MAX_LEN);
        for (int i = 0; i < away_nick_net_count; i++)
        {
#ifndef NDEBUG
            DEBUG("network[%d]:%s", i, away_nick_net_list[i]);
#endif
            /* Reset orig_nick_list */
            orig_nick_list[i][0] = '\0';
        }
    }

    int res;
    if ((res = hexchat_pluginpref_get_int(ph, PREF_POLLING_TIMEOUT)) > 0)
    {
        polling_timeout = res;
    }
    if ((res = hexchat_pluginpref_get_int(ph, PREF_IDLE_TIME)) > 0)
    {
        idle_time = res;
    }

    if (!hexchat_pluginpref_delete(ph, PREF_AWAY_CMD)
        || !hexchat_pluginpref_delete(ph, PREF_BACK_CMD))
    {
        LOG(LOG_WARN, "failed to delete old settings");
    }

    /* Set up X display */

    GtkWindow *win = (GtkWindow *)hexchat_get_info(ph, "gtkwin_ptr");
    assert(win);
    display = GDK_WINDOW_XDISPLAY(gtk_widget_get_window(GTK_WIDGET(win)));

    int event_base, error_base;
    if (!XScreenSaverQueryExtension(display, &event_base, &error_base))
    {
        LOG(LOG_ERR, "XScreenSaver extension not available");
        return 0;
    }
    if (!(ssinfo = XScreenSaverAllocInfo()))
    {
        LOG(LOG_ERR, "failed to allocate a XScreenSaverInfo structure");
        return 0;
    }

    /* Set up hooks */

    hexchat_hook_timer(ph, polling_timeout * 1000, check_idle, NULL);

    /* Ensure that extra commands are executed on any away state change */
    if (*away_extra)
    {
        hexchat_hook_server(ph, RPL_NOWAWAY, HEXCHAT_PRI_NORM, send_extra_cmd, away_extra);
    }
    if (*back_extra)
    {
        hexchat_hook_server(ph, RPL_UNAWAY, HEXCHAT_PRI_NORM, send_extra_cmd, back_extra);
    }

    /* All done */

    hexchat_print(ph, PNAME " plugin loaded\n");

    return 1;
}

int
hexchat_plugin_deinit (void)
{
    set_away(false);

    /* Save settings */

    if (!hexchat_pluginpref_set_str(ph, PREF_AWAY_MSG, away_msg)
        || !hexchat_pluginpref_set_str(ph, PREF_AWAY_EXTRA, away_extra)
        || !hexchat_pluginpref_set_str(ph, PREF_BACK_EXTRA, back_extra)
        || !hexchat_pluginpref_set_str(ph, PREF_AWAY_NICK_SUFFIX, away_nick_suffix)
        || !hexchat_pluginpref_set_str(ph, PREF_AWAY_NICK_NET, away_nick_net)
        || !hexchat_pluginpref_set_int(ph, PREF_POLLING_TIMEOUT, polling_timeout)
        || !hexchat_pluginpref_set_int(ph, PREF_IDLE_TIME, idle_time))
    {
        LOG(LOG_WARN, "failed to save settings");
    }

    /* Free resources */

    XFree(ssinfo);

    return 1;
}
