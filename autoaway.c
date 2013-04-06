/*
 * AutoAway -- a HexChat plugin to set away on idle
 *
 * Copyright (C) 2013 Andrey Vihrov <andrey.vihrov@gmail.com>
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

#include <hexchat-plugin.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

/* Plugin identification */

#define PNAME    "AutoAway"
#define PDESC    "Set away on idle"
#define PVERSION "1.01"

/* Plugin settings */

#define CMD_LEN  512

static char away_cmd[CMD_LEN] = "ALLSERV AWAY Idle";
static char back_cmd[CMD_LEN] = "ALLSERV BACK";
static int  polling_timeout   = 10;      // Seconds
static int  idle_time         = 10 * 60; // Seconds

#define PREF_AWAY_CMD         "away_cmd"
#define PREF_BACK_CMD         "back_cmd"
#define PREF_POLLING_TIMEOUT  "polling_timeout"
#define PREF_IDLE_TIME        "idle_time"


static hexchat_plugin *ph;

static Display          *display;
static XScreenSaverInfo *saver_info;


static inline void perr(const char *msg)
{
    hexchat_printf(ph, PNAME ": error: %s\n", msg);
}

static void set_away()
{
    if (!hexchat_get_info(ph, "away"))
    {
        hexchat_command(ph, away_cmd);
    }
}

static void set_back()
{
    if (hexchat_get_info(ph, "away"))
    {
        hexchat_command(ph, back_cmd);
    }
}

static int check_idle(void *unused)
{
    (void)unused;

    if (!XScreenSaverQueryInfo(display, DefaultRootWindow(display), saver_info))
    {
        perr("XScreenSaverQueryInfo() failed");
        return 1;
    }

    if (saver_info->state == ScreenSaverOn
        || saver_info->idle / 1000 >= (unsigned long)idle_time) /* Idle */
    {
        set_away();
    }
    else /* Not idle */
    {
        set_back();
    }

    return 1;
}

void
hexchat_plugin_get_info(char **name, char **desc,
                        char **version, void **reserved)
{
    *name    = PNAME;    // TODO: Assignment of const char * to char *
    *desc    = PDESC;
    *version = PVERSION;

    (void)reserved;
}

int
hexchat_plugin_init(hexchat_plugin *plugin_handle,
                    char **plugin_name, char **plugin_desc,
                    char **plugin_version, char *arg)
{
    int res;
    int event_base, error_base;

    /* Init variables */

    ph = plugin_handle;

    *plugin_name    = PNAME;
    *plugin_desc    = PDESC;
    *plugin_version = PVERSION;

    (void)arg;


    /* Load settings */

    (void)hexchat_pluginpref_get_str(ph, PREF_AWAY_CMD, away_cmd);
    (void)hexchat_pluginpref_get_str(ph, PREF_BACK_CMD, back_cmd);

    if ((res = hexchat_pluginpref_get_int(ph, PREF_POLLING_TIMEOUT)) > 0)
    {
        polling_timeout = res;
    }
    if ((res = hexchat_pluginpref_get_int(ph, PREF_IDLE_TIME)) > 0)
    {
        idle_time = res;
    }


    /* Open X display */

    if (!(display = XOpenDisplay(NULL)))
    {
        perr("failed to open X display");
        return 0;
    }

    if (!XScreenSaverQueryExtension(display, &event_base, &error_base))
    {
        perr("XScreenSaver extension not available");
        return 0;
    }
    if (!(saver_info = XScreenSaverAllocInfo()))
    {
        perr("failed to allocate a XScreenSaverInfo structure");
        return 0;
    }


    /* Set up timer */

    hexchat_hook_timer(ph, polling_timeout * 1000, check_idle, NULL);

    /* All done */

    hexchat_print(ph, PNAME " plugin loaded\n");

    return 1;
}

int
hexchat_plugin_deinit(void)
{
    set_back();

    /* Save settings */

    if (!hexchat_pluginpref_set_str(ph, PREF_AWAY_CMD, away_cmd)
        || !hexchat_pluginpref_set_str(ph, PREF_BACK_CMD, back_cmd)
        || !hexchat_pluginpref_set_int(ph, PREF_POLLING_TIMEOUT, polling_timeout)
        || !hexchat_pluginpref_set_int(ph, PREF_IDLE_TIME, idle_time))
    {
        perr("failed to save settings");
        return 0;
    }

    /* Close X display */

    XFree(saver_info);
    XCloseDisplay(display);

    return 1;
}
