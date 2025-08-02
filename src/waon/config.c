/* config.c - Configuration file support for WaoN
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
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include "config.h"
#include "memory-check.h"

#define MAX_LINE_LENGTH 1024

char* expand_tilde_path(const char *path)
{
    if (!path || path[0] != '~') {
        return strdup(path);
    }
    
    const char *home = NULL;
    if (path[1] == '/' || path[1] == '\0') {
        /* ~/... or just ~ */
        home = getenv("HOME");
        if (!home) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                home = pw->pw_dir;
            }
        }
        if (!home) {
            return strdup(path); /* Can't expand, return as-is */
        }
        
        size_t len = strlen(home) + strlen(path);
        char *expanded = (char *)malloc(len);
        CHECK_MALLOC(expanded, "expand_tilde_path");
        snprintf(expanded, len, "%s%s", home, path + 1);
        return expanded;
    }
    
    return strdup(path);
}

static char* trim_whitespace(char *str)
{
    char *start = str;
    char *end;
    
    /* Trim leading whitespace */
    while (isspace(*start)) start++;
    
    /* All spaces? */
    if (*start == '\0') return start;
    
    /* Trim trailing whitespace */
    end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    /* Write new null terminator */
    *(end + 1) = '\0';
    
    return start;
}

static config_section_t parse_section(const char *section_name)
{
    if (strcasecmp(section_name, "general") == 0) {
        return CONFIG_SECTION_GENERAL;
    } else if (strcasecmp(section_name, "analysis") == 0) {
        return CONFIG_SECTION_ANALYSIS;
    } else if (strcasecmp(section_name, "note-detection") == 0 ||
               strcasecmp(section_name, "note_detection") == 0) {
        return CONFIG_SECTION_NOTE_DETECTION;
    } else if (strcasecmp(section_name, "range") == 0) {
        return CONFIG_SECTION_RANGE;
    } else if (strcasecmp(section_name, "processing") == 0) {
        return CONFIG_SECTION_PROCESSING;
    }
    return CONFIG_SECTION_GENERAL;
}

static void apply_config_value(waon_options_t *opts, config_section_t section,
                              const char *key, const char *value)
{
    switch (section) {
        case CONFIG_SECTION_GENERAL:
            if (strcasecmp(key, "verbose") == 0) {
                opts->verbose = atoi(value);
            } else if (strcasecmp(key, "quiet") == 0) {
                opts->quiet = atoi(value);
            } else if (strcasecmp(key, "progress") == 0) {
                opts->show_progress = atoi(value);
            }
            break;
            
        case CONFIG_SECTION_ANALYSIS:
            if (strcasecmp(key, "fft-size") == 0 || strcasecmp(key, "fft_size") == 0) {
                opts->fft_size = atol(value);
            } else if (strcasecmp(key, "hop-size") == 0 || strcasecmp(key, "hop_size") == 0) {
                opts->hop_size = atol(value);
            } else if (strcasecmp(key, "window") == 0) {
                opts->window_type = atoi(value);
            } else if (strcasecmp(key, "use-phase") == 0 || strcasecmp(key, "use_phase") == 0) {
                opts->use_phase_vocoder = atoi(value);
            }
            break;
            
        case CONFIG_SECTION_NOTE_DETECTION:
            if (strcasecmp(key, "cutoff") == 0) {
                opts->cutoff_ratio = atof(value);
            } else if (strcasecmp(key, "relative-cutoff") == 0 || strcasecmp(key, "relative_cutoff") == 0) {
                opts->relative_cutoff_ratio = atof(value);
                opts->use_relative_cutoff = 1;
            } else if (strcasecmp(key, "peak-threshold") == 0 || strcasecmp(key, "peak_threshold") == 0) {
                opts->peak_threshold = atoi(value);
            } else if (strcasecmp(key, "pitch-adjust") == 0 || strcasecmp(key, "pitch_adjust") == 0) {
                opts->pitch_adjust = atof(value);
            }
            break;
            
        case CONFIG_SECTION_RANGE:
            if (strcasecmp(key, "top-note") == 0 || strcasecmp(key, "top_note") == 0) {
                opts->top_note = atoi(value);
            } else if (strcasecmp(key, "bottom-note") == 0 || strcasecmp(key, "bottom_note") == 0) {
                opts->bottom_note = atoi(value);
            }
            break;
            
        case CONFIG_SECTION_PROCESSING:
            if (strcasecmp(key, "drum-removal-bins") == 0 || strcasecmp(key, "drum_removal_bins") == 0) {
                opts->drum_removal_bins = atoi(value);
            } else if (strcasecmp(key, "drum-removal-factor") == 0 || strcasecmp(key, "drum_removal_factor") == 0) {
                opts->drum_removal_factor = atof(value);
            } else if (strcasecmp(key, "octave-removal") == 0 || strcasecmp(key, "octave_removal") == 0) {
                opts->octave_removal_factor = atof(value);
            } else if (strcasecmp(key, "threads") == 0) {
                opts->num_threads = atoi(value);
            }
            break;
    }
}

int load_config_file(const char *filename, waon_options_t *opts)
{
    char *expanded_path = expand_tilde_path(filename);
    FILE *fp = fopen(expanded_path, "r");
    
    if (!fp) {
        free(expanded_path);
        return -1;
    }
    
    char line[MAX_LINE_LENGTH];
    config_section_t current_section = CONFIG_SECTION_GENERAL;
    int line_num = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        char *trimmed = trim_whitespace(line);
        
        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }
        
        /* Check for section header */
        if (trimmed[0] == '[') {
            char *end_bracket = strchr(trimmed, ']');
            if (end_bracket) {
                *end_bracket = '\0';
                current_section = parse_section(trimmed + 1);
            } else {
                fprintf(stderr, "Warning: Invalid section header at line %d in %s\n",
                        line_num, expanded_path);
            }
            continue;
        }
        
        /* Parse key = value */
        char *equals = strchr(trimmed, '=');
        if (!equals) {
            fprintf(stderr, "Warning: Invalid line %d in %s (no '=' found)\n",
                    line_num, expanded_path);
            continue;
        }
        
        *equals = '\0';
        char *key = trim_whitespace(trimmed);
        char *value = trim_whitespace(equals + 1);
        
        /* Remove quotes from value if present */
        if ((value[0] == '"' && value[strlen(value)-1] == '"') ||
            (value[0] == '\'' && value[strlen(value)-1] == '\'')) {
            value[strlen(value)-1] = '\0';
            value++;
        }
        
        apply_config_value(opts, current_section, key, value);
    }
    
    fclose(fp);
    free(expanded_path);
    return 0;
}

int load_default_configs(waon_options_t *opts)
{
    int loaded = 0;
    
    /* Try to load system config */
    if (access(WAON_SYSTEM_CONFIG, R_OK) == 0) {
        if (load_config_file(WAON_SYSTEM_CONFIG, opts) == 0) {
            loaded++;
            if (opts->verbose) {
                fprintf(stderr, "Loaded system config from %s\n", WAON_SYSTEM_CONFIG);
            }
        }
    }
    
    /* Try to load user config */
    char *user_config = expand_tilde_path(WAON_USER_CONFIG);
    if (access(user_config, R_OK) == 0) {
        if (load_config_file(user_config, opts) == 0) {
            loaded++;
            if (opts->verbose) {
                fprintf(stderr, "Loaded user config from %s\n", user_config);
            }
        }
    }
    free(user_config);
    
    return loaded;
}

int save_config_file(const char *filename, const waon_options_t *opts)
{
    char *expanded_path = expand_tilde_path(filename);
    FILE *fp = fopen(expanded_path, "w");
    
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", expanded_path);
        free(expanded_path);
        return -1;
    }
    
    fprintf(fp, "# WaoN configuration file\n");
    fprintf(fp, "# Generated automatically\n\n");
    
    fprintf(fp, "[general]\n");
    fprintf(fp, "verbose = %d\n", opts->verbose);
    fprintf(fp, "quiet = %d\n", opts->quiet);
    fprintf(fp, "progress = %d\n", opts->show_progress);
    fprintf(fp, "\n");
    
    fprintf(fp, "[analysis]\n");
    fprintf(fp, "fft-size = %ld\n", opts->fft_size);
    fprintf(fp, "hop-size = %ld\n", opts->hop_size);
    fprintf(fp, "window = %d\n", opts->window_type);
    fprintf(fp, "use-phase = %d\n", opts->use_phase_vocoder);
    fprintf(fp, "\n");
    
    fprintf(fp, "[note-detection]\n");
    fprintf(fp, "cutoff = %f\n", opts->cutoff_ratio);
    if (opts->use_relative_cutoff) {
        fprintf(fp, "relative-cutoff = %f\n", opts->relative_cutoff_ratio);
    }
    fprintf(fp, "peak-threshold = %d\n", opts->peak_threshold);
    fprintf(fp, "pitch-adjust = %f\n", opts->pitch_adjust);
    fprintf(fp, "\n");
    
    fprintf(fp, "[range]\n");
    fprintf(fp, "top-note = %d\n", opts->top_note);
    fprintf(fp, "bottom-note = %d\n", opts->bottom_note);
    fprintf(fp, "\n");
    
    fprintf(fp, "[processing]\n");
    if (opts->drum_removal_bins > 0) {
        fprintf(fp, "drum-removal-bins = %d\n", opts->drum_removal_bins);
        fprintf(fp, "drum-removal-factor = %f\n", opts->drum_removal_factor);
    }
    if (opts->octave_removal_factor > 0.0) {
        fprintf(fp, "octave-removal = %f\n", opts->octave_removal_factor);
    }
    fprintf(fp, "threads = %d\n", opts->num_threads);
    fprintf(fp, "\n");
    
    fclose(fp);
    free(expanded_path);
    return 0;
}