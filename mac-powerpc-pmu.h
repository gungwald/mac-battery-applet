#ifndef MAC_POWERPC_PMU_H
#define MAC_POWERPC_PMU_H

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

#include <stdbool.h>

#define PMU_MAX_STR_LEN 63
#define PMU_MAX_BATTERIES 4

typedef struct
{
	char driver_version[PMU_MAX_STR_LEN+1];
	char firmware_version[PMU_MAX_STR_LEN+1];
	bool ac_power;
	int battery_count;
}
pmu_info_t;

typedef struct
{
	long int flags;
	int charge;	
	int max_charge;
	int current;
	int voltage;
	int time_remaining;
}
pmu_battery_status_t;

/* Experimental combined structure */
typedef struct
{
	pmu_info_t power;
	pmu_battery_status_t batteries[PMU_MAX_BATTERIES];
}
pmu_t;

/* Battery reading functions */
extern bool is_pmu_present();
extern bool read_pmu_info(pmu_info_t *p);
extern bool read_pmu_battery_status(int battery_num, pmu_battery_status_t *p);

/* Helper functions */
extern bool is_pmu_battery_charging(pmu_battery_status_t *p);
extern bool is_pmu_battery_plugged_in_and_fully_charged(pmu_info_t *i, pmu_battery_status_t *p);
extern unsigned short int calc_pmu_battery_charge_percent(pmu_battery_status_t *p);
extern unsigned int calc_pmu_battery_remaining_minutes(pmu_battery_status_t *p);
extern char *format_pmu_info(pmu_info_t *i, char *formatted_output, size_t capacity);
extern char *format_pmu_battery_status(pmu_battery_status_t *s, char *formatted_output, size_t capacity);

#endif

