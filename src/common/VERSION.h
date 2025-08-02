/* Version information for WaoN
 * Copyright (C) 2024 WaoN Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _VERSION_H_
#define _VERSION_H_

/* Version string */
#define WAON_VERSION "0.11"

/* Version components */
#define WAON_VERSION_MAJOR 0
#define WAON_VERSION_MINOR 11
#define WAON_VERSION_PATCH 0

/* Build information */
#define WAON_BUILD_DATE __DATE__
#define WAON_BUILD_TIME __TIME__

/* Version as integer for comparisons */
#define WAON_VERSION_NUMBER (WAON_VERSION_MAJOR * 10000 + \
                            WAON_VERSION_MINOR * 100 + \
                            WAON_VERSION_PATCH)

/* Get full version string with build info */
#define WAON_VERSION_FULL WAON_VERSION " (built " WAON_BUILD_DATE " " WAON_BUILD_TIME ")"

#endif /* !_VERSION_H_ */
