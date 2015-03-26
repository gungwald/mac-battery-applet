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

#include <stdlib.h>
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

/* Prototypes for private functions */
gboolean update_icon(gpointer data);
char *build_icon_file_name(pmu_battery_status_t *battery);
char *build_missing_battery_icon_file_name();
char *build_tooltip(pmu_info_t *i, pmu_battery_status_t *battery);

int main (int argc, char *argv[])
{
	GtkStatusIcon *battery_indicator;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif
	
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

char *build_missing_battery_icon_file_name() {
	static char file_name[MAX_STR_LEN+1];
	strcpy(file_name, BATTERY_ICON_DIR);
	strcat(file_name, "/");
	strcat(file_name, "battery-missing.xpm");
	return file_name;
}

gboolean update_icon(gpointer data)
{
    pmu_info_t pmu_info;
	pmu_battery_status_t battery;
	int percent_charge;
    GtkStatusIcon *battery_icon;
	
    battery_icon = (GtkStatusIcon *) data;
    read_pmu_info(&pmu_info);
	read_pmu_battery_status(0, &battery);
	percent_charge = (float) battery.charge / battery.max_charge * 100;
	gtk_status_icon_set_from_file(battery_icon, build_icon_file_name(&battery));
    gtk_status_icon_set_tooltip_text(battery_icon, build_tooltip(&pmu_info, &battery));
	return TRUE;
}

char *build_tooltip(pmu_info_t *battery_info, pmu_battery_status_t *battery_status)
{
	static char tooltip[MAX_STR_LEN+1];
	static char debug_tooltip[MAX_STR_LEN+1];
	short int charge_percent;
    short int remaining_minutes;
    char formatted_info[MAX_STR_LEN+1];
    char formatted_status[MAX_STR_LEN+1];

	charge_percent = calc_pmu_battery_charge_percent(battery_status);
	remaining_minutes = calc_pmu_battery_remaining_minutes(battery_status);
    if (is_pmu_battery_plugged_in_and_fully_charged(battery_info, battery_status)) {
    	snprintf(tooltip, MAX_STR_LEN+1, "Power at %d%%.\nFully charged.", charge_percent);
    }
	else if (is_pmu_battery_charging(battery_status)) {
		snprintf(tooltip, MAX_STR_LEN+1, "Power at %d%% and charging.\n%d minutes remaining until fully charged.", charge_percent, remaining_minutes);
    }
    else {
    	snprintf(tooltip, MAX_STR_LEN+1, "Power at %d%% and draining.\n%d minutes remaining until completely empty.", charge_percent, remaining_minutes);
    }
    snprintf(debug_tooltip, MAX_STR_LEN+1, "%s\n%s\n%s",
    		tooltip,
    		format_pmu_info(battery_info, formatted_info, MAX_STR_LEN+1),
    		format_pmu_battery_status(battery_status, formatted_status, MAX_STR_LEN+1));
    return debug_tooltip;
}

char *build_icon_file_name(pmu_battery_status_t *battery)
{
	static char file_name[MAX_STR_LEN+1];
    bool is_charging;
    int charge_percent;

	strcpy(file_name, BATTERY_ICON_DIR);
	strcat(file_name, "/");
    charge_percent = calc_pmu_battery_charge_percent(battery);

    /* FIXME - Rework icon for Gnome 3 because it is scaling it down. */
    /* FIXME - Keep current icons for Gnome 2. */
    /* TODO - Make icon theme selectable via a configuration menu. */
    /* TODO - Create new icons for new themes. */
	if (charge_percent < 0) {
		strcat(file_name, "battery-missing.xpm");
	}
	else if (charge_percent >= 90) {
		strcat(file_name, "battery-90%-power");
	}
	else if (charge_percent >= 80) {
		strcat(file_name, "battery-80%-power");
	}
	else if (charge_percent >= 70) {
		strcat(file_name, "battery-70%-power");
	}
	else if (charge_percent >= 60) {
		strcat(file_name, "battery-60%-power");
	}
	else if (charge_percent >= 50) {
		strcat(file_name, "battery-50%-power");
	}
	else if (charge_percent >= 40) {
		strcat(file_name, "battery-40%-power");
	}
	else if (charge_percent >= 30) {
		strcat(file_name, "battery-30%-power");
	}
	else if (charge_percent >= 20) {
		strcat(file_name, "battery-20%-power");
	}
	else if (charge_percent >= 10) {
		strcat(file_name, "battery-10%-power");
	}
	else if (charge_percent >= 0) {
		strcat(file_name, "battery-0%-power");
	}

	if (is_pmu_battery_charging(battery)) {
		strcat(file_name, "-charging");
	}
	strcat(file_name, ".xpm");
	return file_name;
}
