/* progress.c - Progress bar functionality for WaoN
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "progress.h"
#include "memory-check.h"

static int get_terminal_width(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; /* Default width */
}

progress_bar_t* progress_bar_init(long total_steps, const char *label)
{
    progress_bar_t *pb = (progress_bar_t *)malloc(sizeof(progress_bar_t));
    CHECK_MALLOC(pb, "progress_bar_init");
    
    pb->total_steps = total_steps;
    pb->current_step = 0;
    pb->bar_width = 50; /* Default bar width */
    pb->start_time = time(NULL);
    pb->enabled = isatty(STDOUT_FILENO) ? 1 : 0; /* Auto-disable if not a terminal */
    pb->last_percent = -1;
    
    if (label) {
        pb->label = strdup(label);
    } else {
        pb->label = strdup("Processing");
    }
    
    return pb;
}

void progress_bar_update(progress_bar_t *pb, long current_step)
{
    if (!pb || !pb->enabled) return;
    
    pb->current_step = current_step;
    
    /* Calculate percentage */
    int percent = (int)((pb->current_step * 100) / pb->total_steps);
    
    /* Only update if percentage changed */
    if (percent == pb->last_percent) return;
    pb->last_percent = percent;
    
    /* Calculate elapsed time and ETA */
    time_t elapsed = time(NULL) - pb->start_time;
    time_t eta = 0;
    if (pb->current_step > 0) {
        eta = (elapsed * pb->total_steps / pb->current_step) - elapsed;
    }
    
    /* Get terminal width and adjust bar width */
    int term_width = get_terminal_width();
    int available_width = term_width - strlen(pb->label) - 30; /* Reserve space for percentage, time */
    if (available_width < 20) available_width = 20;
    if (available_width < pb->bar_width) pb->bar_width = available_width;
    
    /* Calculate filled portion */
    int filled = (pb->bar_width * pb->current_step) / pb->total_steps;
    
    /* Clear line and print progress bar */
    fprintf(stderr, "\r%s: [", pb->label);
    
    /* Draw progress bar */
    int i;
    for (i = 0; i < pb->bar_width; i++) {
        if (i < filled) {
            fprintf(stderr, "=");
        } else if (i == filled) {
            fprintf(stderr, ">");
        } else {
            fprintf(stderr, " ");
        }
    }
    
    /* Print percentage and time */
    fprintf(stderr, "] %3d%% ", percent);
    
    if (elapsed > 0) {
        fprintf(stderr, "(%lds", elapsed);
        if (eta > 0 && percent < 100) {
            fprintf(stderr, ", ETA: %lds", eta);
        }
        fprintf(stderr, ")");
    }
    
    /* Flush output */
    fflush(stderr);
}

void progress_bar_finish(progress_bar_t *pb)
{
    if (!pb || !pb->enabled) return;
    
    /* Update to 100% */
    pb->current_step = pb->total_steps;
    progress_bar_update(pb, pb->total_steps);
    
    /* Print newline to move to next line */
    fprintf(stderr, "\n");
    fflush(stderr);
}

void progress_bar_free(progress_bar_t *pb)
{
    if (!pb) return;
    
    if (pb->label) {
        free(pb->label);
    }
    free(pb);
}

void progress_bar_enable(progress_bar_t *pb, int enable)
{
    if (pb) {
        pb->enabled = enable;
    }
}