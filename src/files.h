#ifndef FILES_01081969_H
#define FILES_01081969_H

/* Copyright (c) 2015 Bill Chatfield <bill_chatfield@yahoo.com>
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
#include <string.h>

extern bool file_exists(const char *name);
extern bool dir_exists(const char *name);
extern bool find_dir(const char *path_search_list[], const char *name, char result_dir[], size_t capacity);
extern const char *get_short_name(const char *path);

#endif
