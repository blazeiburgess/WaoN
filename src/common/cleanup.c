/* cleanup.c - Centralized cleanup functions for WaoN
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

#include <stdlib.h>
#include <stdio.h>
#include "cleanup.h"

/* Forward declarations for module cleanup functions */
extern void fft_cleanup(void);
extern void snd_cleanup(void);
extern void hc_cleanup(void);

/* Flag to ensure cleanup only happens once */
static int cleanup_done = 0;

/* Main cleanup function that calls all module cleanup functions
 * This should be called at program exit to free all static buffers
 * and prevent memory leaks.
 */
void waon_cleanup(void)
{
  if (cleanup_done) {
    return;  /* Already cleaned up */
  }
  
  /* Call all module cleanup functions */
  fft_cleanup();
  snd_cleanup();
  hc_cleanup();
  
  cleanup_done = 1;
  
#ifdef DEBUG_MEMORY
  /* Print memory statistics if debug mode is enabled */
  fprintf(stderr, "WaoN: All static buffers cleaned up\n");
#endif
}

/* Register cleanup function to be called at exit
 * This uses atexit() to ensure cleanup happens even on unexpected exit
 */
void waon_register_cleanup(void)
{
  static int registered = 0;
  
  if (!registered) {
    if (atexit(waon_cleanup) != 0) {
      fprintf(stderr, "Warning: Failed to register cleanup function\n");
    }
    registered = 1;
  }
}