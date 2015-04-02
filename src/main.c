/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) 2015 Bill Chatfield <bill_chatfield@yahoo.com>
 * 
 * battery-applet is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * battery-applet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* TODO - Add drop-down menu. */
/* TODO - Add link to power setup to drop-down menu. */
/* TODO - Add a special icon/animation for the plug-in event. */
/* TODO - Make it easy to enable/disable standard battery applet. */
/* TODO - Consider C version for basic PPC applet and Java version for extended features. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "mac-powerpc-pmu.h"

/* Constants */
#define MAX_STR_LEN 255
#define UPDATE_INTERVAL_MILLISECONDS 5000
/* FIXME - Icon directory needs to be configurable. Maybe Gnome has a standard
 way of retrieving data files. */
#define BATTERY_ICON_DIR "/home/bill/share/icons/battery"

/* Macros */
/* Calling strlen, letting it step through a possibly long string,
 and then comparing the result to 0 would take unnecessarily long.
 Checking the first character for a null byte is very fast. */
#define IS_EMPTY(s) (s[0]=='\0')

/* Prototypes for private functions */
gboolean update_icon(gpointer data);
char *build_icon_file_name(pmu_battery_status_t *battery);
char *build_missing_battery_icon_file_name();
char *build_tooltip(pmu_info_t *i, pmu_battery_status_t *battery);
char *find_data_dir();
char *find_icon_themes_dir();
char *find_icon_theme_dir(const char *name);
bool file_exists(const char *name);
const char *last_name(const char *path);

/* Globulars */
const char *program_name = NULL;
char *prefix_dir_search_list[] = { "/usr/local", "/opt", "/usr", NULL };

int main(int argc, char *argv[])
{
    GtkStatusIcon *battery_indicator;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    /* Set our program name */
    program_name = last_name(argv[0]);

    gtk_init(&argc, &argv);
    battery_indicator = gtk_status_icon_new();

    if (is_pmu_present()) {
        update_icon(battery_indicator);
        g_timeout_add(UPDATE_INTERVAL_MILLISECONDS, update_icon, battery_indicator);
    }
    else {
        gtk_status_icon_set_from_file(battery_indicator, build_missing_battery_icon_file_name());
    }
    gtk_main();
    return EXIT_SUCCESS;
}

char *build_missing_battery_icon_file_name()
{
    static char file_name[MAX_STR_LEN + 1];
    snprintf(file_name, MAX_STR_LEN + 1, "%s/battery-missing.xpm", BATTERY_ICON_DIR);
    return file_name;
}

gboolean update_icon(gpointer data)
{
    pmu_info_t pmu_info;
    pmu_battery_status_t battery;
    GtkStatusIcon *battery_icon;

    battery_icon = (GtkStatusIcon *) data;
    read_pmu_info(&pmu_info);
    read_pmu_battery_status(0, &battery);
    gtk_status_icon_set_from_file(battery_icon, build_icon_file_name(&battery));
    gtk_status_icon_set_tooltip_text(battery_icon, build_tooltip(&pmu_info, &battery));
    return TRUE;
}

char *build_tooltip(pmu_info_t *battery_info, pmu_battery_status_t *battery_status)
{
    static char tooltip[MAX_STR_LEN + 1];
    static char debug_tooltip[MAX_STR_LEN + 1];
    short int charge_percent;
    short int remaining_minutes;
    char formatted_info[MAX_STR_LEN + 1];
    char formatted_status[MAX_STR_LEN + 1];

    charge_percent = calc_pmu_battery_charge_percent(battery_status);
    remaining_minutes = calc_pmu_battery_remaining_minutes(battery_status);
    if (is_pmu_battery_plugged_in_and_fully_charged(battery_info, battery_status)) {
        snprintf(tooltip, MAX_STR_LEN + 1, "Power at %d%%.\nFully charged.", charge_percent);
    }
    else if (is_pmu_battery_charging(battery_status)) {
        snprintf(tooltip, MAX_STR_LEN + 1, "Power at %d%% and charging.\n%d minutes remaining until fully charged.", charge_percent, remaining_minutes);
    }
    else {
        snprintf(tooltip, MAX_STR_LEN + 1, "Power at %d%% and draining.\n%d minutes remaining until completely empty.", charge_percent, remaining_minutes);
    }
    snprintf(debug_tooltip, MAX_STR_LEN + 1, "%s\n%s\n%s", tooltip, format_pmu_info(battery_info, formatted_info, MAX_STR_LEN + 1), format_pmu_battery_status(battery_status, formatted_status, MAX_STR_LEN + 1));
    return debug_tooltip;
}

char *build_icon_file_name(pmu_battery_status_t *battery)
{
    static char path[MAX_STR_LEN + 1];
    char *file_name;
    int charge_percent;
    char *charging = "";

    charge_percent = calc_pmu_battery_charge_percent(battery);

    /* FIXME - Rework icon for Gnome 3 because it is scaling it down. */
    /* FIXME - Keep current icons for Gnome 2. */
    if (charge_percent < 0) {
        file_name = "battery-missing.xpm";
    }
    else if (charge_percent >= 90) {
        file_name = "battery-90%-power";
    }
    else if (charge_percent >= 80) {
        file_name = "battery-80%-power";
    }
    else if (charge_percent >= 70) {
        file_name = "battery-70%-power";
    }
    else if (charge_percent >= 60) {
        file_name = "battery-60%-power";
    }
    else if (charge_percent >= 50) {
        file_name = "battery-50%-power";
    }
    else if (charge_percent >= 40) {
        file_name = "battery-40%-power";
    }
    else if (charge_percent >= 30) {
        file_name = "battery-30%-power";
    }
    else if (charge_percent >= 20) {
        file_name = "battery-20%-power";
    }
    else if (charge_percent >= 10) {
        file_name = "battery-10%-power";
    }
    else if (charge_percent >= 0) {
        file_name = "battery-0%-power";
    }
    if (is_pmu_battery_charging(battery)) {
        charging = "-charging";
    }
    snprintf(path, MAX_STR_LEN+1, "%s/%s%s.xpm", BATTERY_ICON_DIR, file_name, charging);
    return path;
}

bool file_exists(const char *name)
{
    struct stat file_info;
    return stat(name, &file_info) == 0 || errno != ENOENT;
}

char *find_data_dir()
{
    /* Initialize data_dir to an empty string. */
    static char data_dir[MAX_STR_LEN + 1] = { '\0' };
    int i;

    if (IS_EMPTY(data_dir)) {
        for (i = 0; prefix_dir_search_list[i] != NULL; i++) {
            snprintf(data_dir, MAX_STR_LEN+1, "%s/share/%s", prefix_dir_search_list[i], program_name);
            if (file_exists(data_dir)) {
                break;
            }
        }
    }
    if (IS_EMPTY(data_dir)) {
        fprintf(stderr, "%s: data directory was not found\n", program_name);
        exit(EXIT_FAILURE);
    }
    return data_dir;
}

char *find_icon_themes_dir()
{
    static char themes_dir[MAX_STR_LEN + 1];
    return themes_dir;
}

char *find_icon_theme_dir(const char *name)
{
    return NULL;
}

/**
 * Gets the last component of a path. It returns an empty string if path
 * ends with '/'.
 */
const char *last_name(const char *path)
{
    const char *p;

    p = path + strlen(path) - 1;
    if (*p == '/') {
        p = "";
    }
    else {
        p--;
        while (*p != '/' && p > path) {
            p--;
        }
        if (*p != '/') {
            p = path;
        }
    }
    return p;
}
