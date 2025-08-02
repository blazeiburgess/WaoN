/* waon.h - Public C API for WaoN library
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

#ifndef WAON_LIB_H
#define WAON_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define WAON_LIB_VERSION_MAJOR 0
#define WAON_LIB_VERSION_MINOR 11
#define WAON_LIB_VERSION_PATCH 0

/* Error codes */
typedef enum {
    WAON_SUCCESS = 0,
    WAON_ERROR_MEMORY = -1,
    WAON_ERROR_FILE_NOT_FOUND = -2,
    WAON_ERROR_FILE_FORMAT = -3,
    WAON_ERROR_INVALID_PARAM = -4,
    WAON_ERROR_IO = -5,
    WAON_ERROR_INTERNAL = -6
} waon_error_t;

/* Window types for FFT */
typedef enum {
    WAON_WINDOW_NONE = 0,
    WAON_WINDOW_PARZEN = 1,
    WAON_WINDOW_WELCH = 2,
    WAON_WINDOW_HANNING = 3,
    WAON_WINDOW_HAMMING = 4,
    WAON_WINDOW_BLACKMAN = 5,
    WAON_WINDOW_STEEPER = 6
} waon_window_t;

/* Opaque types */
typedef struct waon_context waon_context_t;
typedef struct waon_options waon_options_t;

/* Progress callback function type */
typedef void (*waon_progress_callback_t)(double progress, void *user_data);

/* ===== Context Management ===== */

/**
 * Create a new WaoN context
 * @return New context or NULL on error
 */
waon_context_t* waon_create(void);

/**
 * Destroy a WaoN context
 * @param ctx Context to destroy
 */
void waon_destroy(waon_context_t *ctx);

/**
 * Get last error code
 * @param ctx WaoN context
 * @return Error code
 */
waon_error_t waon_get_error(waon_context_t *ctx);

/**
 * Get error message for error code
 * @param error Error code
 * @return Error message string
 */
const char* waon_error_string(waon_error_t error);

/* ===== Options Management ===== */

/**
 * Create default options
 * @return New options structure or NULL on error
 */
waon_options_t* waon_options_create(void);

/**
 * Destroy options
 * @param opts Options to destroy
 */
void waon_options_destroy(waon_options_t *opts);

/**
 * Set FFT size
 * @param opts Options structure
 * @param size FFT size (must be power of 2)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_fft_size(waon_options_t *opts, int size);

/**
 * Set hop size (shift between FFT windows)
 * @param opts Options structure
 * @param size Hop size (0 = auto, default is fft_size/4)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_hop_size(waon_options_t *opts, int size);

/**
 * Set window type
 * @param opts Options structure
 * @param window Window type
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_window(waon_options_t *opts, waon_window_t window);

/**
 * Set cutoff ratio
 * @param opts Options structure
 * @param cutoff Log10 of cutoff ratio for velocity scaling
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_cutoff(waon_options_t *opts, double cutoff);

/**
 * Set note range
 * @param opts Options structure
 * @param bottom Bottom MIDI note (0-127)
 * @param top Top MIDI note (0-127)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_note_range(waon_options_t *opts, int bottom, int top);

/**
 * Enable/disable phase vocoder
 * @param opts Options structure
 * @param enable 1 to enable, 0 to disable
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_phase_vocoder(waon_options_t *opts, int enable);

/**
 * Set drum removal parameters
 * @param opts Options structure
 * @param bins Number of averaging bins
 * @param factor Removal factor
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_drum_removal(waon_options_t *opts, int bins, double factor);

/**
 * Set octave removal factor
 * @param opts Options structure
 * @param factor Octave removal factor
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_options_set_octave_removal(waon_options_t *opts, double factor);

/* ===== Main Transcription Functions ===== */

/**
 * Transcribe audio file to MIDI
 * @param ctx WaoN context
 * @param input_file Input audio file path
 * @param output_file Output MIDI file path
 * @param opts Options (NULL for defaults)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_transcribe(waon_context_t *ctx,
                             const char *input_file,
                             const char *output_file,
                             const waon_options_t *opts);

/**
 * Transcribe audio data to MIDI
 * @param ctx WaoN context
 * @param audio_data Audio samples (mono or interleaved stereo)
 * @param num_samples Number of samples
 * @param sample_rate Sample rate in Hz
 * @param channels Number of channels (1 or 2)
 * @param output_file Output MIDI file path
 * @param opts Options (NULL for defaults)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_transcribe_data(waon_context_t *ctx,
                                  const double *audio_data,
                                  long num_samples,
                                  int sample_rate,
                                  int channels,
                                  const char *output_file,
                                  const waon_options_t *opts);

/**
 * Set progress callback
 * @param ctx WaoN context
 * @param callback Callback function (NULL to disable)
 * @param user_data User data passed to callback
 */
void waon_set_progress_callback(waon_context_t *ctx,
                               waon_progress_callback_t callback,
                               void *user_data);

/* ===== Advanced Functions ===== */

/**
 * Analyze audio and return note events without writing MIDI
 * @param ctx WaoN context
 * @param input_file Input audio file path
 * @param opts Options (NULL for defaults)
 * @param notes Output array of MIDI note numbers (caller allocates)
 * @param velocities Output array of velocities (caller allocates)
 * @param start_times Output array of start times in seconds (caller allocates)
 * @param durations Output array of durations in seconds (caller allocates)
 * @param max_notes Maximum number of notes to return
 * @param num_notes Output: actual number of notes found
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_analyze(waon_context_t *ctx,
                         const char *input_file,
                         const waon_options_t *opts,
                         int *notes,
                         int *velocities,
                         double *start_times,
                         double *durations,
                         int max_notes,
                         int *num_notes);

/* ===== Utility Functions ===== */

/**
 * Get library version string
 * @return Version string
 */
const char* waon_version_string(void);

/**
 * Get library version numbers
 * @param major Output: major version (can be NULL)
 * @param minor Output: minor version (can be NULL)
 * @param patch Output: patch version (can be NULL)
 */
void waon_version(int *major, int *minor, int *patch);

/**
 * Initialize library (called automatically)
 * @return WAON_SUCCESS or error code
 */
waon_error_t waon_init(void);

/**
 * Cleanup library resources
 */
void waon_lib_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* WAON_LIB_H */