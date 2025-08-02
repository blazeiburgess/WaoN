/* config.h - Configuration file support for WaoN
 * Copyright (C) 2024 WaoN Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef WAON_CONFIG_H
#define WAON_CONFIG_H

#include "cli.h"

/* Default config file paths */
#define WAON_SYSTEM_CONFIG "/etc/waon.conf"
#define WAON_USER_CONFIG   "~/.waonrc"

/* Config file sections */
typedef enum {
    CONFIG_SECTION_GENERAL,
    CONFIG_SECTION_ANALYSIS,
    CONFIG_SECTION_NOTE_DETECTION,
    CONFIG_SECTION_RANGE,
    CONFIG_SECTION_PROCESSING
} config_section_t;

/* Load configuration from file */
int load_config_file(const char *filename, waon_options_t *opts);

/* Load default configuration files (system and user) */
int load_default_configs(waon_options_t *opts);

/* Save current options to config file */
int save_config_file(const char *filename, const waon_options_t *opts);

/* Expand ~ in filepath to home directory */
char* expand_tilde_path(const char *path);

#endif /* WAON_CONFIG_H */