/* waon.c - WaoN library implementation
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#ifdef FFTW2
#include <rfftw.h>
#else
#include <fftw3.h>
#endif

#include <sndfile.h>

#include "waon.h"
#include "fft.h"
#include "hc.h"
#include "snd.h"
#include "midi.h"
#include "analyse.h"
#include "notes.h"
#include "memory-check.h"
#include "cleanup.h"

/* Global variables are defined in their respective source files */
/* We just include the headers that declare them as extern */

/* Internal context structure */
struct waon_context {
    waon_error_t last_error;
    waon_progress_callback_t progress_callback;
    void *progress_user_data;
    int initialized;
};

/* Internal options structure */
struct waon_options {
    /* Basic parameters */
    int fft_size;
    int hop_size;
    waon_window_t window_type;
    double cutoff_ratio;
    int note_bottom;
    int note_top;
    
    /* Advanced parameters */
    int use_phase_vocoder;
    int drum_removal_bins;
    double drum_removal_factor;
    double octave_removal_factor;
    double pitch_adjust;
    int peak_threshold;
    int use_relative_cutoff;
    double relative_cutoff_ratio;
};

/* Static initialization flag */
static int library_initialized = 0;

/* Initialize library */
waon_error_t waon_init(void)
{
    if (!library_initialized) {
        waon_register_cleanup();
        library_initialized = 1;
    }
    return WAON_SUCCESS;
}

/* Cleanup library resources - this is the public API function */
void waon_cleanup(void)
{
    /* No-op for now since cleanup is handled automatically via atexit */
    /* This function exists for API compatibility */
}

/* Create a new WaoN context */
waon_context_t* waon_create(void)
{
    waon_init();
    
    waon_context_t *ctx = (waon_context_t*)malloc(sizeof(waon_context_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->last_error = WAON_SUCCESS;
    ctx->progress_callback = NULL;
    ctx->progress_user_data = NULL;
    ctx->initialized = 1;
    
    return ctx;
}

/* Destroy a WaoN context */
void waon_destroy(waon_context_t *ctx)
{
    if (ctx) {
        free(ctx);
    }
}

/* Get last error code */
waon_error_t waon_get_error(waon_context_t *ctx)
{
    if (!ctx) {
        return WAON_ERROR_INVALID_PARAM;
    }
    return ctx->last_error;
}

/* Get error message for error code */
const char* waon_error_string(waon_error_t error)
{
    switch (error) {
        case WAON_SUCCESS:
            return "Success";
        case WAON_ERROR_MEMORY:
            return "Memory allocation failed";
        case WAON_ERROR_FILE_NOT_FOUND:
            return "File not found";
        case WAON_ERROR_FILE_FORMAT:
            return "Invalid file format";
        case WAON_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case WAON_ERROR_IO:
            return "I/O error";
        case WAON_ERROR_INTERNAL:
            return "Internal error";
        default:
            return "Unknown error";
    }
}

/* Create default options */
waon_options_t* waon_options_create(void)
{
    waon_options_t *opts = (waon_options_t*)malloc(sizeof(waon_options_t));
    if (!opts) {
        return NULL;
    }
    
    /* Set defaults matching waon's defaults */
    opts->fft_size = 2048;
    opts->hop_size = 512;  /* Default to fft_size/4 */
    opts->window_type = WAON_WINDOW_HANNING;
    opts->cutoff_ratio = -5.0;
    opts->note_bottom = 48;  /* C3 */
    opts->note_top = 72;     /* C5 */
    opts->use_phase_vocoder = 1;
    opts->drum_removal_bins = 0;
    opts->drum_removal_factor = 0.0;
    opts->octave_removal_factor = 0.0;
    opts->pitch_adjust = 0.0;
    opts->peak_threshold = 128;
    opts->use_relative_cutoff = 0;
    opts->relative_cutoff_ratio = 1.0;
    
    return opts;
}

/* Destroy options */
void waon_options_destroy(waon_options_t *opts)
{
    if (opts) {
        free(opts);
    }
}

/* Set FFT size */
waon_error_t waon_options_set_fft_size(waon_options_t *opts, int size)
{
    if (!opts) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    /* Check if power of 2 */
    if (size <= 0 || (size & (size - 1)) != 0) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->fft_size = size;
    /* Adjust hop size if it was default */
    if (opts->hop_size == opts->fft_size / 4) {
        opts->hop_size = size / 4;
    }
    
    return WAON_SUCCESS;
}

/* Set hop size */
waon_error_t waon_options_set_hop_size(waon_options_t *opts, int size)
{
    if (!opts || size < 0) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->hop_size = (size == 0) ? opts->fft_size / 4 : size;
    return WAON_SUCCESS;
}

/* Set window type */
waon_error_t waon_options_set_window(waon_options_t *opts, waon_window_t window)
{
    if (!opts || window < WAON_WINDOW_NONE || window > WAON_WINDOW_STEEPER) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->window_type = window;
    return WAON_SUCCESS;
}

/* Set cutoff ratio */
waon_error_t waon_options_set_cutoff(waon_options_t *opts, double cutoff)
{
    if (!opts) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->cutoff_ratio = cutoff;
    opts->use_relative_cutoff = 0;
    return WAON_SUCCESS;
}

/* Set note range */
waon_error_t waon_options_set_note_range(waon_options_t *opts, int bottom, int top)
{
    if (!opts || bottom < 0 || bottom > 127 || top < 0 || top > 127 || bottom > top) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->note_bottom = bottom;
    opts->note_top = top;
    return WAON_SUCCESS;
}

/* Enable/disable phase vocoder */
waon_error_t waon_options_set_phase_vocoder(waon_options_t *opts, int enable)
{
    if (!opts) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->use_phase_vocoder = enable ? 1 : 0;
    return WAON_SUCCESS;
}

/* Set drum removal parameters */
waon_error_t waon_options_set_drum_removal(waon_options_t *opts, int bins, double factor)
{
    if (!opts || bins < 0 || factor < 0.0) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->drum_removal_bins = bins;
    opts->drum_removal_factor = factor;
    return WAON_SUCCESS;
}

/* Set octave removal factor */
waon_error_t waon_options_set_octave_removal(waon_options_t *opts, double factor)
{
    if (!opts || factor < 0.0) {
        return WAON_ERROR_INVALID_PARAM;
    }
    
    opts->octave_removal_factor = factor;
    return WAON_SUCCESS;
}

/* Set progress callback */
void waon_set_progress_callback(waon_context_t *ctx,
                               waon_progress_callback_t callback,
                               void *user_data)
{
    if (ctx) {
        ctx->progress_callback = callback;
        ctx->progress_user_data = user_data;
    }
}

/* Internal function to perform transcription */
static waon_error_t waon_transcribe_internal(waon_context_t *ctx,
                                             SNDFILE *sf,
                                             SF_INFO *sfinfo,
                                             const char *output_file,
                                             const waon_options_t *opts)
{
    int i;
    
    /* Use default options if none provided */
    waon_options_t default_opts;
    const waon_options_t *options = opts;
    if (!options) {
        waon_options_t *tmp = waon_options_create();
        if (!tmp) {
            ctx->last_error = WAON_ERROR_MEMORY;
            return ctx->last_error;
        }
        default_opts = *tmp;
        waon_options_destroy(tmp);
        options = &default_opts;
    }
    
    /* Set global variables for compatibility */
    abs_flg = options->use_relative_cutoff ? 0 : 1;
    adj_pitch = options->pitch_adjust;
    
    /* Local variables from options */
    long len = options->fft_size;
    long hop = options->hop_size;
    int flag_window = options->window_type;
    int notelow = options->note_bottom;
    int notetop = options->note_top;
    double cut_ratio = options->cutoff_ratio;
    double rel_cut_ratio = options->relative_cutoff_ratio;
    int flag_phase = options->use_phase_vocoder;
    int psub_n = options->drum_removal_bins;
    double psub_f = options->drum_removal_factor;
    double oct_f = options->octave_removal_factor;
    int peak_threshold = options->peak_threshold;
    
    /* Check stereo or mono */
    if (sfinfo->channels != 2 && sfinfo->channels != 1) {
        ctx->last_error = WAON_ERROR_FILE_FORMAT;
        return ctx->last_error;
    }
    
    /* Initialize notes structure */
    struct WAON_notes *notes = WAON_notes_init();
    if (!notes) {
        ctx->last_error = WAON_ERROR_MEMORY;
        return ctx->last_error;
    }
    
    char vel[128];
    int on_event[128];
    for (i = 0; i < 128; i++) {
        vel[i] = 0;
        on_event[i] = -1;
    }
    
    /* Allocate buffers */
    double *left = (double *)malloc(sizeof(double) * len);
    double *right = (double *)malloc(sizeof(double) * len);
    if (!left || !right) {
        ctx->last_error = WAON_ERROR_MEMORY;
        WAON_notes_free(notes);
        if (left) free(left);
        if (right) free(right);
        return ctx->last_error;
    }
    
#ifdef FFTW2
    double *x = (double *)malloc(sizeof(double) * len);
    double *y = (double *)malloc(sizeof(double) * len);
#else
    double *x = (double *)fftw_malloc(sizeof(double) * len);
    double *y = (double *)fftw_malloc(sizeof(double) * len);
#endif
    
    if (!x || !y) {
        ctx->last_error = WAON_ERROR_MEMORY;
        WAON_notes_free(notes);
        free(left);
        free(right);
        if (x) free(x);
        if (y) free(y);
        return ctx->last_error;
    }
    
    /* Power spectrum */
    double *p = (double *)malloc(sizeof(double) * (len / 2 + 1));
    if (!p) {
        ctx->last_error = WAON_ERROR_MEMORY;
        WAON_notes_free(notes);
        free(left);
        free(right);
        free(x);
        free(y);
        return ctx->last_error;
    }
    
    /* Phase vocoder arrays */
    double *p0 = NULL, *dphi = NULL, *ph0 = NULL, *ph1 = NULL;
    if (flag_phase != 0) {
        p0 = (double *)malloc(sizeof(double) * (len / 2 + 1));
        dphi = (double *)malloc(sizeof(double) * (len / 2 + 1));
        ph0 = (double *)malloc(sizeof(double) * (len / 2 + 1));
        ph1 = (double *)malloc(sizeof(double) * (len / 2 + 1));
        
        if (!p0 || !dphi || !ph0 || !ph1) {
            ctx->last_error = WAON_ERROR_MEMORY;
            WAON_notes_free(notes);
            free(left);
            free(right);
            free(x);
            free(y);
            free(p);
            if (p0) free(p0);
            if (dphi) free(dphi);
            if (ph0) free(ph0);
            if (ph1) free(ph1);
            return ctx->last_error;
        }
    }
    
    double *pmidi = (double *)malloc(sizeof(double) * 128);
    if (!pmidi) {
        ctx->last_error = WAON_ERROR_MEMORY;
        WAON_notes_free(notes);
        free(left);
        free(right);
        free(x);
        free(y);
        free(p);
        if (p0) free(p0);
        if (dphi) free(dphi);
        if (ph0) free(ph0);
        if (ph1) free(ph1);
        return ctx->last_error;
    }
    
    /* Time-period for FFT */
    double t0 = (double)len / (double)sfinfo->samplerate;
    
    /* Weight of window function for FFT */
    double den = init_den(len, flag_window);
    
    /* Set range to analyse */
    int i0 = (int)(mid2freq[notelow] * t0 - 0.5);
    int i1 = (int)(mid2freq[notetop] * t0 - 0.5) + 1;
    if (i0 <= 0) i0 = 1;
    if (i1 >= (len/2)) i1 = len/2 - 1;
    
    /* Initialize FFTW plan */
#ifdef FFTW2
    rfftw_plan plan = rfftw_create_plan(len, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
#else
    fftw_plan plan = fftw_plan_r2r_1d(len, x, y, FFTW_R2HC, FFTW_ESTIMATE);
#endif
    
    /* For first step */
    if (hop != len) {
        if (sndfile_read(sf, *sfinfo, left + hop, right + hop, (len - hop)) != (len - hop)) {
            ctx->last_error = WAON_ERROR_IO;
            goto cleanup;
        }
    }
    
    /* Estimate total frames for progress */
    long total_frames = sfinfo->frames / hop;
    
    /* Main loop */
    pitch_shift = 0.0;
    n_pitch = 0;
    int icnt;
    
    for (icnt = 0; ; icnt++) {
        /* Shift buffers */
        for (i = 0; i < len - hop; i++) {
            if (sfinfo->channels == 2) {
                left[i] = left[i + hop];
                right[i] = right[i + hop];
            } else {
                left[i] = left[i + hop];
            }
        }
        
        /* Read from audio */
        if (sndfile_read(sf, *sfinfo, left + (len - hop), right + (len - hop), hop) != hop) {
            break;
        }
        
        /* Set double table x[] for FFT */
        for (i = 0; i < len; i++) {
            if (sfinfo->channels == 2) {
                x[i] = 0.5 * (left[i] + right[i]);
            } else {
                x[i] = left[i];
            }
        }
        
        /* Stage 1: calc power spectrum */
        windowing(len, x, flag_window, 1.0, x);
        
#ifdef FFTW2
        rfftw_one(plan, x, y);
#else
        fftw_execute(plan);
#endif
        
        if (flag_phase == 0) {
            HC_to_amp2(len, y, den, p);
        } else {
            HC_to_polar2(len, y, 0, den, p, ph1);
            
            if (icnt == 0) {
                for (i = 0; i < (len/2+1); ++i) {
                    dphi[i] = 0.0;
                    p0[i] = p[i];
                    ph0[i] = ph1[i];
                }
            } else {
                for (i = 0; i < (len/2+1); ++i) {
                    double twopi = 2.0 * M_PI;
                    dphi[i] = ph1[i] - ph0[i] - twopi * (double)i / (double)len * (double)hop;
                    for (; dphi[i] >= M_PI; dphi[i] -= twopi);
                    for (; dphi[i] < -M_PI; dphi[i] += twopi);
                    
                    dphi[i] = dphi[i] / twopi / (double)hop;
                    
                    p0[i] = p[i];
                    ph0[i] = ph1[i];
                    
                    p[i] = 0.5 * (sqrt(p[i]) + sqrt(p0[i]));
                    p[i] = p[i] * p[i];
                }
            }
        }
        
        /* Drum removal */
        if (psub_n != 0) {
            power_subtract_ave(len, p, psub_n, psub_f);
        }
        
        /* Octave removal */
        if (oct_f != 0.0) {
            power_subtract_octave(len, p, oct_f);
        }
        
        /* Stage 2: pickup notes */
        if (flag_phase == 0) {
            note_intensity(p, NULL, cut_ratio, rel_cut_ratio, i0, i1, t0, vel);
        } else {
            for (i = 0; i < (len/2+1); ++i) {
                dphi[i] = ((double)i / (double)len + dphi[i]) * (double)sfinfo->samplerate;
            }
            note_intensity(p, dphi, cut_ratio, rel_cut_ratio, i0, i1, t0, vel);
        }
        
        /* Stage 3: check previous time for note-on/off */
        WAON_notes_check(notes, icnt, vel, on_event, 8, 0, peak_threshold);
        
        /* Progress callback */
        if (ctx->progress_callback) {
            double progress = (double)icnt / (double)total_frames;
            if (progress > 1.0) progress = 1.0;
            ctx->progress_callback(progress, ctx->progress_user_data);
        }
    }
    
    /* Clean notes */
    WAON_notes_regulate(notes);
    WAON_notes_remove_shortnotes(notes, 1, 64);
    WAON_notes_remove_shortnotes(notes, 2, 28);
    WAON_notes_remove_octaves(notes);
    
    /* Calculate division */
    long div = (long)(0.5 * (double)sfinfo->samplerate / (double)hop);
    
    /* Output MIDI */
    WAON_notes_output_midi(notes, div, (char*)output_file);
    
    ctx->last_error = WAON_SUCCESS;
    
cleanup:
#ifdef FFTW2
    rfftw_destroy_plan(plan);
#else
    fftw_destroy_plan(plan);
#endif
    
    WAON_notes_free(notes);
    free(left);
    free(right);
    free(x);
    free(y);
    free(p);
    if (p0) free(p0);
    if (dphi) free(dphi);
    if (ph0) free(ph0);
    if (ph1) free(ph1);
    free(pmidi);
    
    return ctx->last_error;
}

/* Transcribe audio file to MIDI */
waon_error_t waon_transcribe(waon_context_t *ctx,
                            const char *input_file,
                            const char *output_file,
                            const waon_options_t *opts)
{
    if (!ctx || !input_file || !output_file) {
        if (ctx) ctx->last_error = WAON_ERROR_INVALID_PARAM;
        return WAON_ERROR_INVALID_PARAM;
    }
    
    /* Open input file */
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));
    SNDFILE *sf = sf_open(input_file, SFM_READ, &sfinfo);
    if (!sf) {
        ctx->last_error = WAON_ERROR_FILE_NOT_FOUND;
        return ctx->last_error;
    }
    
    /* Perform transcription */
    waon_error_t result = waon_transcribe_internal(ctx, sf, &sfinfo, output_file, opts);
    
    sf_close(sf);
    return result;
}

/* Transcribe audio data to MIDI */
waon_error_t waon_transcribe_data(waon_context_t *ctx,
                                  const double *audio_data,
                                  long num_samples,
                                  int sample_rate,
                                  int channels,
                                  const char *output_file,
                                  const waon_options_t *opts)
{
    if (!ctx || !audio_data || !output_file || num_samples <= 0 || 
        sample_rate <= 0 || (channels != 1 && channels != 2)) {
        if (ctx) ctx->last_error = WAON_ERROR_INVALID_PARAM;
        return WAON_ERROR_INVALID_PARAM;
    }
    
    /* Create virtual file info */
    SF_INFO sfinfo;
    sfinfo.samplerate = sample_rate;
    sfinfo.channels = channels;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfo.frames = num_samples / channels;
    
    /* TODO: Implement memory-based processing */
    /* For now, this would require creating a virtual SNDFILE from memory */
    /* which is not trivial with libsndfile */
    
    ctx->last_error = WAON_ERROR_INTERNAL;
    return ctx->last_error;
}

/* Analyze audio and return note events */
waon_error_t waon_analyze(waon_context_t *ctx,
                         const char *input_file,
                         const waon_options_t *opts,
                         int *notes,
                         int *velocities,
                         double *start_times,
                         double *durations,
                         int max_notes,
                         int *num_notes)
{
    /* TODO: Implement analysis without MIDI output */
    /* This would require modifying the core processing to return note data */
    /* instead of writing directly to MIDI */
    
    if (ctx) ctx->last_error = WAON_ERROR_INTERNAL;
    return WAON_ERROR_INTERNAL;
}

/* Get library version string */
const char* waon_version_string(void)
{
    return "0.11.0";
}

/* Get library version numbers */
void waon_version(int *major, int *minor, int *patch)
{
    if (major) *major = WAON_LIB_VERSION_MAJOR;
    if (minor) *minor = WAON_LIB_VERSION_MINOR;
    if (patch) *patch = WAON_LIB_VERSION_PATCH;
}