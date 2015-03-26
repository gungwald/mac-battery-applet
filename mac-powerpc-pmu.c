/* Copyright (c) 2014 Bill Chatfield <bill_chatfield@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>     /* strstr, strtok */
#include <strings.h>    /* strcasecmp */
#include <stdarg.h>     /* va_list, va_start, va_end */
#include <stdbool.h>	/* bool, true, false */
#include <errno.h>		/* errno */
#include <malloc.h>		/* malloc, free */
#include <sys/types.h>	/* stat */
#include <sys/stat.h>	/* stat */
#include <unistd.h>		/* stat */
#include <linux/pmu.h>

#include "mac-powerpc-pmu.h"

#define PMU_DIR "/proc/pmu"
#define PMU_INFO_FILE "/proc/pmu/info"
#define PMU_BATTERY_FILE "/proc/pmu/battery_"
#define PMU_MAX_LINE_LEN 255

#define PMU_DRIVER_VER_KEY "PMU driver version"
#define PMU_FIRMWARE_VER_KEY "PMU firmware version"
#define PMU_AC_PWR_KEY "AC Power"
#define PMU_BATTERY_COUNT_KEY "Battery count"

#define PMU_FLAGS_KEY "flags"
#define PMU_CHARGE_KEY "charge"
#define PMU_MAX_CHARGE_KEY "max_charge"
#define PMU_CURRENT_KEY "current"
#define PMU_VOLTAGE_KEY "voltage"
#define PMU_TIME_REMAINING_KEY "time rem."

/* Values for pmu_battery_battery_status_t.flags */
#define PMU_BATT_PRESENT     0x00000001
#define PMU_BATT_CHARGING    0x00000002
#define PMU_BATT_TYPE_MASK   0x000000f0
#define PMU_BATT_TYPE_SMART  0x00000010
#define PMU_BATT_TYPE_HOOPER 0x00000020
#define PMU_BATT_TYPE_COMET  0x00000030

static void init_pmu_info(pmu_info_t *p);
static char *read_file(const char *file_name);
static FILE *open_file(const char *file_name);
static bool read_line(FILE *f, const char *file_name, char *line);
static bool parse_line(char *line, char **key, char **value);
static bool close_file(FILE *f, const char *file_name);
static void report_errno(const char *name_of_object_that_caused_the_error);
static void report_error(const char *message, ...);
static char *trim(char *s);

bool is_pmu_present()
{
    struct stat pmu_dir;
    bool is_present = false;

    if (stat(PMU_DIR, &pmu_dir) == 0 && S_ISDIR(pmu_dir.st_mode)) {
        is_present = true;
    }
    else if (errno != ENOENT) {
        /* ENOENT means that the directory does not exist. That just means
           we return false. But any other error is a real error. */
        report_errno(PMU_DIR);
    }
    return is_present;
}

void init_pmu_info(pmu_info_t *p) 
{
    p->driver_version[0] = '\0';
    p->firmware_version[0] = '\0';
    p->ac_power = false;
    p->battery_count = 0;
}

void init_pmu_battery_status(pmu_battery_status_t *p) 
{
    p->flags = 0;
    p->charge = 0;
    p->max_charge = 0;
    p->voltage = 0;
    p->time_remaining = 0;
}

bool read_pmu_info(pmu_info_t *p) 
{
    FILE *pmu_info_file;
    bool matched_something = false;
    char line[PMU_MAX_LINE_LEN + 1];
    char *key;
    char *value;

    if ((pmu_info_file = open_file(PMU_INFO_FILE)) != NULL) {

        init_pmu_info(p);

        while (read_line(pmu_info_file, PMU_INFO_FILE, line)) {

            if (parse_line(line, &key, &value)) {

                if (strcasecmp(key, PMU_DRIVER_VER_KEY) == 0) {
                    strncpy(p->driver_version, value, PMU_MAX_STR_LEN);
                    p->driver_version[PMU_MAX_STR_LEN] = '\0';
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_FIRMWARE_VER_KEY) == 0) {
                    strncpy(p->firmware_version, value, PMU_MAX_STR_LEN);
                    p->firmware_version[PMU_MAX_STR_LEN] = '\0';
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_AC_PWR_KEY) == 0) {
                    p->ac_power = value[0] == '1' ? true : false;
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_BATTERY_COUNT_KEY) == 0) {
                    p->battery_count = atoi(value);
                    matched_something = true;
                }
                else {
                    report_error("%s: unrecognized key: %s, value: %s\n", PMU_INFO_FILE, key, value);
                }
            }
        }
        close_file(pmu_info_file, PMU_INFO_FILE);
    }
    return matched_something;
}

bool read_pmu_battery_status(int battery_num, pmu_battery_status_t *p)
{
    FILE *battery_file;
    bool matched_something = false;
    char line[PMU_MAX_LINE_LEN + 1];
    char *key;
    char *value;
    char battery_file_name[PMU_MAX_STR_LEN+1];
    int count;

    /* The snprintf size argument must count the string termainator char.
       The glibc implementation adds the terminator even when truncation
       occurs. */
    count = snprintf(battery_file_name, PMU_MAX_STR_LEN+1, "%s%d", PMU_BATTERY_FILE, battery_num);

    /* Check for truncation. */
    if (count >= PMU_MAX_STR_LEN+1) {
        report_error("Battery file name %s%d is too large for buffer.\n", PMU_BATTERY_FILE, battery_num);
        return false;
    }

    battery_file = open_file(battery_file_name);
    if (battery_file != NULL) {

        init_pmu_battery_status(p);

        while (read_line(battery_file, battery_file_name, line)) {

            if (parse_line(line, &key, &value)) {

                if (strcasecmp(key, PMU_FLAGS_KEY) == 0) {
                    char *end_ptr;
                    // Convert the flags hex string value to a long int.
                    p->flags = strtol(value, &end_ptr, 16);
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_CHARGE_KEY) == 0) {
                    p->charge = atoi(value);
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_MAX_CHARGE_KEY) == 0) {
                    p->max_charge = atoi(value);
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_VOLTAGE_KEY) == 0) {
                    p->voltage = atoi(value);
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_CURRENT_KEY) == 0) {
                    p->current = atoi(value);
                    matched_something = true;
                }
                else if (strcasecmp(key, PMU_TIME_REMAINING_KEY) == 0) {
                    p->time_remaining = atoi(value);
                    matched_something = true;
                }
                else {
                    report_error("%s: unrecognized key: '%s', with value: '%s'\n", battery_file_name, key, value);
                }
            }
        }
        close_file(battery_file, battery_file_name);
    }
    return matched_something;
}

/* Helper functions */

bool is_pmu_battery_charging(pmu_battery_status_t *p)
{
	return p->flags & PMU_BATT_CHARGING;
}

bool is_pmu_battery_plugged_in_and_fully_charged(pmu_info_t *i, pmu_battery_status_t *p) {
	return i->ac_power && p->time_remaining == 0;
}

unsigned short int calc_pmu_battery_charge_percent(pmu_battery_status_t *p)
{
	return ((float)p->charge) / ((float)p->max_charge) * 100;
}

unsigned int calc_pmu_battery_remaining_minutes(pmu_battery_status_t *p)
{
	return p->time_remaining / 60;
}

char *format_pmu_info(pmu_info_t *i, char *formatted_output, size_t capacity)
{
	snprintf(formatted_output, capacity, "%s = %s\n%s = %s\n%s = %s\n%s = %d",
			PMU_DRIVER_VER_KEY, i->driver_version,
			PMU_FIRMWARE_VER_KEY, i->firmware_version,
			PMU_AC_PWR_KEY, i->ac_power ? "true" : "false",
			PMU_BATTERY_COUNT_KEY, i->battery_count);
    return formatted_output;
}

char *format_pmu_battery_status(pmu_battery_status_t *s, char *formatted_output, size_t capacity)
{
	snprintf(formatted_output, capacity, "%s = %d\n%s = %d\n%s = %d\n%s = %d\n%s = %d\n%s = %d",
			PMU_FLAGS_KEY, s->flags,
			PMU_CHARGE_KEY, s->charge,
			PMU_MAX_CHARGE_KEY,s->max_charge,
			PMU_CURRENT_KEY,s->current,
			PMU_VOLTAGE_KEY,s->voltage,
			PMU_TIME_REMAINING_KEY,s->time_remaining);
    return formatted_output;
}

/* I/O functions with error handling */

FILE *open_file(const char *file_name)
{
    FILE *f;

    if ((f = fopen(file_name, "r")) == NULL) {
        report_errno(file_name);
    }
    return f;
}

bool read_line(FILE *f, const char *file_name, char *line)
{
    bool ok = false;

    if (fgets(line, PMU_MAX_LINE_LEN + 1, f)) {
        ok = true;
    }
    else if (ferror(f)) {
        report_errno(file_name);
    }
    return ok;
}

/**
 * Parses the given line variable into a key/value pair.
 * The line variable is modified by strtok which is used
 * to do the parsing.
 */
bool parse_line(char *line, char **key, char **value)
{
    char *delimiters = ":\r\n";
    bool found_key_and_value = false;

    *key = NULL;
    *value = NULL;

    if ((*key = strtok(line, delimiters)) != NULL) {
        if ((*value = strtok(NULL, delimiters)) != NULL) {
            found_key_and_value = true;
        }
        else {
            /* No value token on this line. */
            report_error("%s: missing value for key: %s\n", PMU_INFO_FILE, key);
        }
    }
    *key = trim(*key);
    *value = trim(*value);
    return found_key_and_value;
}

bool close_file(FILE *f, const char *file_name)
{
    bool successful = true;

    if (fclose(f) == EOF) {
        report_errno(file_name);
        successful = false;
    }
    return successful;
}

/* The return value must be free'd. */
char *read_file(const char *file_name)
{
    FILE *f;
    char *contents = NULL;
    struct stat f_info;
    off_t f_len;

    if ((f = fopen(file_name, "r")) != NULL) {
        if (fstat(fileno(f), &f_info) == 0) {
            /* fstat was successful. */
            f_len = f_info.st_size;
            contents = (char *) malloc((f_len + 1) * sizeof(char));
            if (fread(contents, f_len, 1, f) == 1) {
                /* One item of size f_len was successfully read. */
                contents[f_len] = '\0';
            }
            else if (ferror(f)) {
                report_errno(file_name);
                free(contents);
                contents = NULL;
            }
        }
        else {
            report_errno(file_name);
        }
        fclose(f);
    }
    return contents;
}

/* Error handling functions */

void report_errno(const char *name_of_object_that_caused_the_error)
{
    /* Something different could be done here depending on where errors need 
       to go, like displaying an error dialog. */
    perror(name_of_object_that_caused_the_error);
}

void report_error(const char *message, ...)
{
    va_list args;

    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
}

char *trim(char *s)
{
	char *last;
    size_t len;

    if (s != NULL) {
        len = strlen(s);
    	if (len > 0) {
    		last = s + len - 1;
    		while (s <= last && isspace(*s)) {
    			s++;
    		}
    		while (last >= s && isspace(*last)) {
    			*last = '\0';
    			last--;
    		}
		}
	}
    return s;
}
