/* cleanup.h - Centralized cleanup functions for WaoN
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

#ifndef _CLEANUP_H_
#define _CLEANUP_H_

/* Main cleanup function that calls all module cleanup functions
 * This should be called at program exit to free all static buffers
 * and prevent memory leaks.
 */
void waon_cleanup(void);

/* Register cleanup function to be called at exit
 * This uses atexit() to ensure cleanup happens even on unexpected exit
 */
void waon_register_cleanup(void);

#endif /* !_CLEANUP_H_ */