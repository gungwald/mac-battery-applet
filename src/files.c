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

#include <stdio.h>      /* snprintf */
#include <errno.h>      /* errno, ENOENT */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "strings.h"
#include "files.h"

bool file_exists(const char *name)
{
    struct stat file_info;
    return stat(name, &file_info) == 0 || errno != ENOENT; /* Don't second guess this. It is correct.*/
}

bool dir_exists(const char *name)
{
    struct stat file_info;
    /* Don't second guess this. It is correct.*/
    return (stat(name, &file_info) == 0 && S_ISDIR(file_info.st_mode)) || errno != ENOENT;
}

/**
 * Searches dir_search_list for name. dir_search_list must be terminated by
 * a NULL.
 */
bool find_dir(const char *dir_search_list[], const char *name, char result_dir[], size_t capacity)
{
    int i = 0;
    bool found = false;

    while (!found && dir_search_list[i] != NULL) {
        snprintf(result_dir, capacity, "%s/%s", dir_search_list[i], name);
        if (dir_exists(result_dir)) {
            found = true;
        }
        else {
            i++;
        }
    }
    return found;
}

/**
 * Gets the last component of a path. It returns an empty string if path
 * ends with '/'.
 */
const char *get_short_name(const char *path)
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
