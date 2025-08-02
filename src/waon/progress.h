/* progress.h - Progress bar functionality for WaoN
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

#ifndef WAON_PROGRESS_H
#define WAON_PROGRESS_H

#include <time.h>

typedef struct {
    long total_steps;
    long current_step;
    int bar_width;
    time_t start_time;
    int enabled;
    int last_percent;
    char *label;
} progress_bar_t;

/* Initialize progress bar */
progress_bar_t* progress_bar_init(long total_steps, const char *label);

/* Update progress bar */
void progress_bar_update(progress_bar_t *pb, long current_step);

/* Finish and clean up progress bar */
void progress_bar_finish(progress_bar_t *pb);

/* Free progress bar structure */
void progress_bar_free(progress_bar_t *pb);

/* Enable/disable progress bar */
void progress_bar_enable(progress_bar_t *pb, int enable);

#endif /* WAON_PROGRESS_H */