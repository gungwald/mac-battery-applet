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

/* TODO - Format all files so there are no tabs. */
/* TODO - Move TODOs to Mylin. */
/* TODO - Add drop-down menu. */
/* TODO - Add link to power setup to drop-down menu. */
/* TODO - Add a special icon/animation for the plug-in event. */
/* TODO - Make it easy to enable/disable standard battery applet. */
/* TODO - Consider C version for basic PPC applet and Java version for extended features. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "strings.h"
#include "files.h"
#include "mac-powerpc-pmu.h"

/* Constants */
#define UPDATE_INTERVAL_MILLISECONDS 5000

/* Macros */

/* Prototypes for private functions */
gboolean update_icon(gpointer data);
char *build_icon_file_name(pmu_battery_status_t *battery);
char *build_missing_battery_icon_file_name();
char *build_tooltip(pmu_info_t *i, pmu_battery_status_t *battery);
char *find_data_dir();
char *find_icon_themes_dir();
char *find_icon_theme_dir(const char *name);
bool set_icon_theme(const char *name);

/* Globulars */
const char *program_name = NULL;
const char *prefix_dir_search_list[] = { "/usr/local", "/opt", "/usr", NULL };
char *icon_theme_dir = NULL;
bool debug = false;

int main(int argc, char *argv[])
{
    GtkStatusIcon *battery_indicator;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    /* Set our program name */
    program_name = get_short_name(argv[0]);
    set_icon_theme("default");

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
    static char file_name[STR_CAPACITY];
    char *icon_dir;

    icon_dir = find_icon_theme_dir("default");
    snprintf(file_name, STR_CAPACITY, "%s/battery-missing.xpm", icon_dir);
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
    static char tooltip[STR_CAPACITY];
    static char debug_tooltip[STR_CAPACITY];
    short int charge_percent;
    short int remaining_minutes;
    char formatted_info[STR_CAPACITY];
    char formatted_status[STR_CAPACITY];

    charge_percent = calc_pmu_battery_charge_percent(battery_status);
    remaining_minutes = calc_pmu_battery_remaining_minutes(battery_status);
    if (is_pmu_battery_plugged_in_and_fully_charged(battery_info, battery_status)) {
        snprintf(tooltip, STR_CAPACITY, "Power at %d%%.\nFully charged.", charge_percent);
    }
    else if (is_pmu_battery_charging(battery_status)) {
        snprintf(tooltip, STR_CAPACITY, "Power at %d%% and charging.\n%d minutes remaining until fully charged.", charge_percent, remaining_minutes);
    }
    else {
        snprintf(tooltip, STR_CAPACITY, "Power at %d%% and draining.\n%d minutes remaining until completely empty.", charge_percent, remaining_minutes);
    }
    if (debug) {
        snprintf(debug_tooltip, STR_CAPACITY, "%s\n%s\n%s", tooltip, format_pmu_info(battery_info, formatted_info, STR_CAPACITY), format_pmu_battery_status(battery_status, formatted_status, STR_CAPACITY));
        return debug_tooltip;
    }
    return tooltip;
}

char *build_icon_file_name(pmu_battery_status_t *battery)
{
    static char path[STR_CAPACITY];
    char *file_name;
    int charge_percent;
    char *charging = "";
    char *icon_dir;

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
    icon_dir = find_icon_theme_dir("default");
    snprintf(path, STR_CAPACITY, "%s/%s%s.xpm", icon_dir, file_name, charging);
    return path;
}

/**
 * Searches all the usual suspects for a data directory matching the program
 * name.
 * <p>
 * Returns the data directory found as a pointer to a static string or exits
 * with an error if no directory was found.
 */
char *find_data_dir()
{
    /* Initialize data_dir to an empty string. */
    static char data_dir[STR_CAPACITY] = { '\0' };

    if (IS_EMPTY_STRING(data_dir)) {
        if (! find_dir(prefix_dir_search_list, "share/mac-battery-applet", data_dir, STR_CAPACITY)) {
            fprintf(stderr, "%s: data directory was not found\n", program_name);
            exit(EXIT_FAILURE);
        }
    }
    return data_dir;
}

/**
 * Looks for a directory named 'icon-themes' inside the data directory
 * and exits with an error if it is not found.
 */
char *find_icon_themes_dir()
{
    static char themes_dir[STR_CAPACITY];
    char *data_dir;

    data_dir = find_data_dir();
    snprintf(themes_dir, STR_CAPACITY, "%s/icon-themes", data_dir);
    if (! file_exists(themes_dir)) {
        fprintf(stderr, "%s: icon themes directory was not found in %s\n", program_name, data_dir);
        exit(EXIT_FAILURE);
    }
    return themes_dir;
}

/**
 * Looks for the directory by the given name inside the icon themes directory
 * as returned by find_icon_themes_dir. Returns an empty string if it is not
 * found.
 */
char *find_icon_theme_dir(const char *name)
{
    static char theme_dir[STR_CAPACITY];
    char *themes_dir;

    themes_dir = find_icon_themes_dir();
    snprintf(theme_dir, STR_CAPACITY, "%s/%s", themes_dir, name);
    if (! file_exists(theme_dir)) {
        CLEAR_STRING(theme_dir);
    }
    return theme_dir;
}

bool set_icon_theme(const char *name)
{
    char *theme_dir;

    theme_dir = find_icon_theme_dir(name);
    if (CONTAINS_A_VALUE(theme_dir)) {
        icon_theme_dir = theme_dir;
        return true;
    }
    return false;
}
