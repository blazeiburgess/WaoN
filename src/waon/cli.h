/* cli.h - Command Line Interface handling for WaoN
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

#ifndef WAON_CLI_H
#define WAON_CLI_H

#include <getopt.h>

/* Structure to hold all command line options */
typedef struct {
    /* File options */
    char *input_file;
    char *output_file;
    char *patch_file;
    char *config_file;
    
    /* FFT options */
    long fft_size;
    long hop_size;
    int window_type;
    
    /* Note detection options */
    double cutoff_ratio;
    double relative_cutoff_ratio;
    int use_relative_cutoff;
    int peak_threshold;
    int top_note;
    int bottom_note;
    double pitch_adjust;
    
    /* Phase vocoder options */
    int use_phase_vocoder;
    
    /* Drum removal options */
    int drum_removal_bins;
    double drum_removal_factor;
    
    /* Octave removal options */
    double octave_removal_factor;
    
    /* New modern options */
    int show_progress;
    int verbose;
    int quiet;
    int dry_run;
    int batch_mode;
    int json_output;
    int num_threads;
    
    /* Help and version */
    int show_help;
    int show_version;
    char *help_topic;
} waon_options_t;

/* Option IDs for getopt_long */
enum {
    OPT_HELP = 'h',
    OPT_VERSION = 'v',
    OPT_INPUT = 'i',
    OPT_OUTPUT = 'o',
    OPT_PATCH = 'p',
    OPT_FFT_SIZE = 'n',
    OPT_HOP_SIZE = 's',
    OPT_WINDOW = 'w',
    OPT_CUTOFF = 'c',
    OPT_RELATIVE = 'r',
    OPT_PEAK = 'k',
    OPT_TOP = 't',
    OPT_BOTTOM = 'b',
    OPT_ADJUST = 'a',
    OPT_QUIET = 'q',
    OPT_PROGRESS = 'P',
    
    /* Long-only options start at 256 to avoid conflicts */
    OPT_NOPHASE = 256,
    OPT_PSUB_N,
    OPT_PSUB_F,
    OPT_OCT,
    OPT_CONFIG,
    OPT_NO_CONFIG,
    OPT_DRY_RUN,
    OPT_BATCH,
    OPT_JSON,
    OPT_THREADS,
    OPT_VERBOSE,
    OPT_HELP_ALL,
    OPT_FFT_SIZE_LONG,
    OPT_HOP_SIZE_LONG,
    OPT_DRUM_REMOVAL_BINS,
    OPT_DRUM_REMOVAL_FACTOR,
    OPT_OCTAVE_REMOVAL,
    OPT_NO_PHASE,
    OPT_TOP_NOTE,
    OPT_BOTTOM_NOTE
};

/* Function declarations */
void waon_options_init(waon_options_t *opts);
int waon_parse_args(int argc, char **argv, waon_options_t *opts);
void waon_options_validate(waon_options_t *opts);
void waon_options_free(waon_options_t *opts);

void print_version(void);
void print_usage(const char *program_name);
void print_help_topic(const char *topic);
void print_help_all(void);

#endif /* WAON_CLI_H */