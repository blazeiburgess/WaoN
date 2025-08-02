/* cli.c - Command Line Interface handling for WaoN
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
#include <errno.h>
#include <unistd.h>
#include "cli.h"
#include "VERSION.h"
#include "memory-check.h"

/* Long options mapping for getopt_long */
static struct option long_options[] = {
    /* Standard options with short equivalents */
    {"help",                no_argument,       0, OPT_HELP},
    {"version",             no_argument,       0, OPT_VERSION},
    {"input",               required_argument, 0, OPT_INPUT},
    {"output",              required_argument, 0, OPT_OUTPUT},
    {"patch",               required_argument, 0, OPT_PATCH},
    {"window",              required_argument, 0, OPT_WINDOW},
    {"shift",               required_argument, 0, OPT_HOP_SIZE},
    {"hop-size",            required_argument, 0, OPT_HOP_SIZE},
    {"cutoff",              required_argument, 0, OPT_CUTOFF},
    {"relative",            required_argument, 0, OPT_RELATIVE},
    {"peak",                required_argument, 0, OPT_PEAK},
    {"top",                 required_argument, 0, OPT_TOP},
    {"top-note",            required_argument, 0, OPT_TOP},
    {"bottom",              required_argument, 0, OPT_BOTTOM},
    {"bottom-note",         required_argument, 0, OPT_BOTTOM},
    {"adjust",              required_argument, 0, OPT_ADJUST},
    {"quiet",               no_argument,       0, OPT_QUIET},
    {"progress",            no_argument,       0, OPT_PROGRESS},
    
    /* Long-only options for backward compatibility */
    {"nophase",             no_argument,       0, OPT_NOPHASE},
    {"no-phase",            no_argument,       0, OPT_NO_PHASE},
    {"psub-n",              required_argument, 0, OPT_PSUB_N},
    {"drum-removal-bins",   required_argument, 0, OPT_DRUM_REMOVAL_BINS},
    {"psub-f",              required_argument, 0, OPT_PSUB_F},
    {"drum-removal-factor", required_argument, 0, OPT_DRUM_REMOVAL_FACTOR},
    {"oct",                 required_argument, 0, OPT_OCT},
    {"octave-removal",      required_argument, 0, OPT_OCTAVE_REMOVAL},
    
    /* Modern long-only options */
    {"fft-size",            required_argument, 0, OPT_FFT_SIZE_LONG},
    {"config",              required_argument, 0, OPT_CONFIG},
    {"no-config",           no_argument,       0, OPT_NO_CONFIG},
    {"dry-run",             no_argument,       0, OPT_DRY_RUN},
    {"batch",               no_argument,       0, OPT_BATCH},
    {"json",                no_argument,       0, OPT_JSON},
    {"threads",             required_argument, 0, OPT_THREADS},
    {"verbose",             no_argument,       0, OPT_VERBOSE},
    {"help-all",            no_argument,       0, OPT_HELP_ALL},
    {0, 0, 0, 0}
};

/* Short options string for getopt_long */
static const char *short_options = "hvi:o:p:n:s:w:c:r:k:t:b:a:qP";

void waon_options_init(waon_options_t *opts)
{
    memset(opts, 0, sizeof(waon_options_t));
    
    /* Set default values */
    opts->fft_size = 2048;
    opts->hop_size = 0;  /* Will be set to fft_size/4 if not specified */
    opts->window_type = 3;  /* Hanning window */
    opts->cutoff_ratio = -5.0;
    opts->relative_cutoff_ratio = 1.0;
    opts->use_relative_cutoff = 0;
    opts->peak_threshold = 128;  /* No peak search */
    opts->top_note = 103;  /* G8 */
    opts->bottom_note = 28;  /* E2 */
    opts->pitch_adjust = 0.0;
    opts->use_phase_vocoder = 1;
    opts->drum_removal_bins = 0;
    opts->drum_removal_factor = 0.0;
    opts->octave_removal_factor = 0.0;
    opts->num_threads = 1;
}

int waon_parse_args(int argc, char **argv, waon_options_t *opts)
{
    int c;
    int option_index = 0;
    
    /* Support both traditional parsing and getopt_long */
    /* First check if using old-style options without dashes */
    int i;
    for (i = 1; i < argc; i++) {
        /* Handle old-style options like -nophase, -psub-n, -psub-f, -oct */
        if (strcmp(argv[i], "-nophase") == 0) {
            opts->use_phase_vocoder = 0;
        }
        else if (strcmp(argv[i], "-psub-n") == 0) {
            if (i + 1 < argc) {
                opts->drum_removal_bins = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: -psub-n requires an argument\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-psub-f") == 0) {
            if (i + 1 < argc) {
                opts->drum_removal_factor = atof(argv[++i]);
            } else {
                fprintf(stderr, "Error: -psub-f requires an argument\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-oct") == 0) {
            if (i + 1 < argc) {
                opts->octave_removal_factor = atof(argv[++i]);
            } else {
                fprintf(stderr, "Error: -oct requires an argument\n");
                return -1;
            }
        }
    }
    
    /* Reset getopt for proper parsing */
    optind = 1;
    
    /* Now parse with getopt_long */
    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
            case OPT_HELP:
                opts->show_help = 1;
                break;
                
            case OPT_VERSION:
                opts->show_version = 1;
                break;
                
            case OPT_INPUT:
                opts->input_file = strdup(optarg);
                break;
                
            case OPT_OUTPUT:
                opts->output_file = strdup(optarg);
                break;
                
            case OPT_PATCH:
                opts->patch_file = strdup(optarg);
                break;
                
            case OPT_FFT_SIZE:
            case OPT_FFT_SIZE_LONG:
                opts->fft_size = atol(optarg);
                break;
                
            case OPT_HOP_SIZE:
                opts->hop_size = atol(optarg);
                break;
                
            case OPT_WINDOW:
                opts->window_type = atoi(optarg);
                break;
                
            case OPT_CUTOFF:
                opts->cutoff_ratio = atof(optarg);
                break;
                
            case OPT_RELATIVE:
                opts->relative_cutoff_ratio = atof(optarg);
                opts->use_relative_cutoff = 1;
                break;
                
            case OPT_PEAK:
                opts->peak_threshold = atoi(optarg);
                break;
                
            case OPT_TOP:
                opts->top_note = atoi(optarg);
                break;
                
            case OPT_BOTTOM:
                opts->bottom_note = atoi(optarg);
                break;
                
            case OPT_ADJUST:
                opts->pitch_adjust = atof(optarg);
                break;
                
            case OPT_QUIET:
                opts->quiet = 1;
                break;
                
            case OPT_PROGRESS:
                opts->show_progress = 1;
                break;
                
            case OPT_NOPHASE:
            case OPT_NO_PHASE:
                opts->use_phase_vocoder = 0;
                break;
                
            case OPT_PSUB_N:
            case OPT_DRUM_REMOVAL_BINS:
                opts->drum_removal_bins = atoi(optarg);
                break;
                
            case OPT_PSUB_F:
            case OPT_DRUM_REMOVAL_FACTOR:
                opts->drum_removal_factor = atof(optarg);
                break;
                
            case OPT_OCT:
            case OPT_OCTAVE_REMOVAL:
                opts->octave_removal_factor = atof(optarg);
                break;
                
            case OPT_CONFIG:
                opts->config_file = strdup(optarg);
                break;
                
            case OPT_NO_CONFIG:
                /* Flag to skip config file loading */
                break;
                
            case OPT_DRY_RUN:
                opts->dry_run = 1;
                break;
                
            case OPT_BATCH:
                opts->batch_mode = 1;
                break;
                
            case OPT_JSON:
                opts->json_output = 1;
                break;
                
            case OPT_THREADS:
                opts->num_threads = atoi(optarg);
                break;
                
            case OPT_VERBOSE:
                opts->verbose = 1;
                break;
                
            case OPT_HELP_ALL:
                opts->show_help = 2;  /* Special value for extended help */
                break;
                
            case '?':
                /* getopt_long already printed an error message */
                return -1;
                
            default:
                fprintf(stderr, "Unknown option: %c\n", c);
                return -1;
        }
    }
    
    /* Handle remaining non-option arguments */
    if (optind < argc) {
        /* Could be old-style usage without option flags */
        /* For now, we'll just warn */
        if (!opts->quiet) {
            fprintf(stderr, "Warning: unexpected argument(s): ");
            while (optind < argc) {
                fprintf(stderr, "%s ", argv[optind++]);
            }
            fprintf(stderr, "\n");
        }
    }
    
    return 0;
}

void waon_options_validate(waon_options_t *opts)
{
    /* Set default hop size if not specified */
    if (opts->hop_size == 0) {
        opts->hop_size = opts->fft_size / 4;
    }
    
    /* Validate window type */
    if (opts->window_type < 0 || opts->window_type > 6) {
        opts->window_type = 0;
    }
    
    /* Clear drum removal if parameters are invalid */
    if (opts->drum_removal_bins == 0) opts->drum_removal_factor = 0.0;
    if (opts->drum_removal_factor == 0.0) opts->drum_removal_bins = 0;
    
    /* Set default output file if not specified */
    if (opts->output_file == NULL && !opts->dry_run) {
        opts->output_file = strdup("output.mid");
    }
    
    /* Set stdin indicator for input if not specified */
    if (opts->input_file == NULL) {
        opts->input_file = strdup("-");
    }
}

void waon_options_free(waon_options_t *opts)
{
    if (opts->input_file) free(opts->input_file);
    if (opts->output_file) free(opts->output_file);
    if (opts->patch_file) free(opts->patch_file);
    if (opts->config_file) free(opts->config_file);
    if (opts->help_topic) free(opts->help_topic);
}

void print_version(void)
{
    fprintf(stdout, "WaoN - a Wave-to-Notes transcriber, Version %s\n\n", WAON_VERSION);
    fprintf(stdout, "Copyright (C) 1998-2007 Kengo Ichiki <kichiki@users.sourceforge.net>\n");
    fprintf(stdout, "Web: http://waon.sourceforge.net/\n\n");
}

void print_usage(const char *program_name)
{
    print_version();
    fprintf(stdout, "WaoN is a Wave-to-Notes transcriber,\n"
           "that is, a converter from sound file to midi file.\n\n");
    fprintf(stdout, "Usage: %s [option ...]\n\n", program_name);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -h --help\tprint this help.\n");
    fprintf(stdout, "  -v, --version\tprint version information.\n");
    fprintf(stdout, "OPTIONS FOR FILES\n");
    fprintf(stdout, "  -i --input\tinput wav file (default: stdin)\n");
    fprintf(stdout, "  -o --output\toutput mid file (default: 'output.mid')\n");
    fprintf(stdout, "\toptions -i and -o have argument '-' as stdin/stdout\n");
    fprintf(stdout, "  -p --patch\tpatch file (default: no patch)\n");
    fprintf(stdout, "FFT OPTIONS\n");
    fprintf(stdout, "  -n --fft-size\tsampling number from WAV in 1 step (default: 2048)\n");
    fprintf(stdout, "  -w --window\t0 no window\n");
    fprintf(stdout, "\t\t1 parzen window\n");
    fprintf(stdout, "\t\t2 welch window\n");
    fprintf(stdout, "\t\t3 hanning window (default)\n");
    fprintf(stdout, "\t\t4 hamming window\n");
    fprintf(stdout, "\t\t5 blackman window\n");
    fprintf(stdout, "\t\t6 steeper 30-dB/octave rolloff window\n");
    fprintf(stdout, "READING WAV OPTIONS\n");
    fprintf(stdout, "  -s --shift --hop-size\tshift number from WAV in 1 step\n");
    fprintf(stdout, "\t\t(default: 1/4 of the value in -n option)\n");
    fprintf(stdout, "PHASE-VOCODER OPTIONS\n");
    fprintf(stdout, "  -nophase --no-phase\tdon't use phase diff to improve freq estimation.\n"
           "\t\t(default: use the correction)\n");
    fprintf(stdout, "NOTE SELECTION OPTIONS\n");
    fprintf(stdout, "  -c --cutoff\tlog10 of cut-off ratio to scale velocity of note\n"
           "\t\t(default: -5.0)\n");
    fprintf(stdout, "  -r --relative\tlog10 of cut-off ratio relative to the average.\n"
           "\t\t(default: no relative cutoff\n"
           "\t\t= absolute cutoff with the value in -c option)\n");
    fprintf(stdout, "  -k --peak\tpeak threshold for note-on, which ranges [0,127]\n"
           "\t\t(default: 128 = no peak-search = search only first on-event)\n");
    fprintf(stdout, "  -t --top --top-note\ttop note [midi #] (default: 103 = G7)\n");
    fprintf(stdout, "  -b --bottom --bottom-note\tbottom note [midi #] (default: 28 = E1)\n");
    fprintf(stdout, "\tHere middle C (261 Hz) = C4 = midi 60. Midi # ranges [0,127].\n");
    fprintf(stdout, "  -a --adjust\tadjust-pitch param, which is suggested by WaoN after analysis.\n"
           "\t\tunit is half-note, that is, +1 is half-note up,\n"
           "\t\tand -0.5 is quater-note down. (default: 0)\n");
    fprintf(stdout, "DRUM-REMOVAL OPTIONS\n");
    fprintf(stdout, "  -psub-n --drum-removal-bins\tnumber of averaging bins in one side.\n"
           "\t\tthat is, for n, (i-n,...,i,...,i+n) are averaged\n"
           "\t\t(default: 0)\n");
    fprintf(stdout, "  -psub-f --drum-removal-factor\tfactor to the average,"
           " where the power is modified as\n"
           "\t\tp[i] = (sqrt(p[i]) - f * sqrt(ave[i]))^2\n"
           "\t\t(default: 0.0)\n");
    fprintf(stdout, "OCTAVE-REMOVAL OPTIONS\n");
    fprintf(stdout, "  -oct --octave-removal\tfactor to the octave removal,"
           " where the power is modified as\n"
           "\t\tp[i] = (sqrt(p[i]) - f * sqrt(oct[i]))^2\n"
           "\t\t(default: 0.0)\n");
    fprintf(stdout, "MODERN OPTIONS\n");
    fprintf(stdout, "  -P --progress\tshow progress bar during processing\n");
    fprintf(stdout, "  -q --quiet\tsuppress non-error output\n");
    fprintf(stdout, "  --verbose\tshow detailed processing information\n");
    fprintf(stdout, "  --dry-run\tshow what would be done without processing\n");
    fprintf(stdout, "  --config FILE\tread options from configuration file\n");
    fprintf(stdout, "  --batch\tenable batch processing mode\n");
    fprintf(stdout, "  --json\toutput results in JSON format\n");
    fprintf(stdout, "  --threads N\tnumber of threads for batch processing (default: 1)\n");
}

void print_help_topic(const char *topic)
{
    /* TODO: Implement topic-specific help */
    fprintf(stdout, "Help for topic '%s' not yet implemented.\n", topic);
}

void print_help_all(void)
{
    /* Print extended help with all options and examples */
    print_usage("waon");
    fprintf(stdout, "\nEXAMPLES:\n");
    fprintf(stdout, "  Basic conversion:\n");
    fprintf(stdout, "    waon -i input.wav -o output.mid\n\n");
    fprintf(stdout, "  With custom FFT settings:\n");
    fprintf(stdout, "    waon -i input.wav -o output.mid -n 4096 -s 1024 -w 3\n\n");
    fprintf(stdout, "  Real-time processing with timidity:\n");
    fprintf(stdout, "    cat input.wav | waon -i - -o - | timidity -id -\n\n");
    fprintf(stdout, "  With progress bar and verbose output:\n");
    fprintf(stdout, "    waon -i input.wav -o output.mid --progress --verbose\n\n");
    fprintf(stdout, "  Batch processing:\n");
    fprintf(stdout, "    waon -i \"*.wav\" --batch --threads 4 --progress\n\n");
}