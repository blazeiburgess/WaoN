/* WaoN - a Wave-to-Notes transcriber : main
 * Copyright (C) 1998-2008,2011 Kengo Ichiki <kichiki@users.sourceforge.net>
 * $Id: main.c,v 1.12 2011/12/27 13:11:00 kichiki Exp $
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
#include <stdio.h> /* printf(), fprintf(), strerror()  */
#include <sys/errno.h> /* errno  */
#include <stdlib.h> /* exit()  */
#include <string.h> /* strcat(), strcpy()  */
#include "memory-check.h" // CHECK_MALLOC() macro

/* FFTW library  */
#ifdef FFTW2
#include <rfftw.h>
#else // FFTW3
#include <fftw3.h>
#endif // FFTW2

#include "fft.h" // FFT utility functions
#include "hc.h" // HC array manipulation routines

// libsndfile
#include <sndfile.h>
#include "snd.h"

#include "midi.h" /* smf_...(), mid2freq[], get_note()  */
#include "analyse.h" /* note_intensity(), note_on_off(), output_midi()  */
#include "notes.h" // struct WAON_notes

#include "VERSION.h"
#include "cli.h"
#include "config.h"
#include "progress.h"


/* These functions are now in cli.c, but we keep the declarations for compatibility */
extern void print_version(void);
extern void print_usage(const char *program_name);

/* Legacy argument parsing for backward compatibility */
static int parse_legacy_args(int argc, char **argv, waon_options_t *opts)
{
  int i;
  for (i = 1; i < argc; i++)
    {
      if ((strcmp (argv[i], "-input" ) == 0)
	 || (strcmp (argv[i], "-i" ) == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->input_file = strdup(argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "-output" ) == 0)
	      || (strcmp (argv[i], "-o" ) == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->output_file = strdup(argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--cutoff") == 0)
	       || (strcmp (argv[i], "-c") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->cutoff_ratio = atof (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--top") == 0)
	       || (strcmp (argv[i], "-t") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->top_note = atoi( argv[++i] );
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--bottom") == 0)
	       || (strcmp (argv[i], "-b") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->bottom_note = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--window") == 0)
	       || (strcmp (argv[i], "-w") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->window_type = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ( strcmp (argv[i], "-n") == 0)
	{
	  if ( i+1 < argc )
	    {
	      opts->fft_size = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--shift") == 0)
	       || (strcmp (argv[i], "-s") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->hop_size = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--patch") == 0)
	       || (strcmp (argv[i], "-p") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->patch_file = strdup(argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--relative") == 0)
	       || (strcmp (argv[i], "-r") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->relative_cutoff_ratio = atof (argv[++i]);
	      opts->use_relative_cutoff = 1;
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--peak") == 0)
	       || (strcmp (argv[i], "-k") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->peak_threshold = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--adjust") == 0)
	       || (strcmp (argv[i], "-a") == 0))
	{
	  if ( i+1 < argc )
	    {
	      opts->pitch_adjust = atof (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if ((strcmp (argv[i], "--help") == 0)
	       || (strcmp (argv[i], "-h") == 0))
	{
	  opts->show_help = 1;
	  break;
	}
      else if (strcmp (argv[i], "-nophase") == 0)
	{
	  opts->use_phase_vocoder = 0;
	}
      else if (strcmp (argv[i], "-psub-n") == 0)
	{
	  if ( i+1 < argc )
	    {
	      opts->drum_removal_bins = atoi (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if (strcmp (argv[i], "-psub-f") == 0)
	{
	  if ( i+1 < argc )
	    {
	      opts->drum_removal_factor = atof (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if (strcmp (argv[i], "-oct") == 0)
	{
	  if ( i+1 < argc )
	    {
	      opts->octave_removal_factor = atof (argv[++i]);
	    }
	  else
	    {
	      return -1;
	    }
	}
      else if (strcmp (argv[i], "-v") == 0 ||
	       strcmp (argv[i], "--version") == 0)
	{
	  opts->show_version = 1;
	}
      else
	{
	  opts->show_help = 1;
	}
    }
  return 0;
}


int main (int argc, char** argv)
{
  extern int abs_flg; /* flag for absolute/relative cutoff  */
  extern double adj_pitch;
  extern double pitch_shift;
  extern int n_pitch;

  int i;

  /* Initialize options structure */
  waon_options_t opts;
  waon_options_init(&opts);

  /* Try to load default config files first (can be overridden by command line) */
  load_default_configs(&opts);

  /* Check if using old-style argument format first */
  int using_legacy_args = 0;
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-psub-", 6) == 0 || 
        strcmp(argv[i], "-nophase") == 0 ||
        strcmp(argv[i], "-oct") == 0 ||
        (argv[i][0] == '-' && argv[i][1] != '-' && strlen(argv[i]) > 2)) {
      using_legacy_args = 1;
      break;
    }
  }

  /* Parse arguments */
  if (using_legacy_args) {
    /* Use legacy parser for backward compatibility */
    if (parse_legacy_args(argc, argv, &opts) < 0) {
      print_usage(argv[0]);
      waon_options_free(&opts);
      exit(1);
    }
  } else {
    /* Use new getopt_long parser */
    if (waon_parse_args(argc, argv, &opts) < 0) {
      print_usage(argv[0]);
      waon_options_free(&opts);
      exit(1);
    }
  }

  /* Handle help and version */
  if (opts.show_help) {
    if (opts.show_help == 2) {
      print_help_all();
    } else {
      print_usage(argv[0]);
    }
    waon_options_free(&opts);
    exit(0);
  }
  if (opts.show_version) {
    print_version();
    waon_options_free(&opts);
    exit(0);
  }

  /* Validate and set defaults */
  waon_options_validate(&opts);

  /* Set global variables for compatibility with existing code */
  abs_flg = opts.use_relative_cutoff ? 0 : 1;
  adj_pitch = opts.pitch_adjust;

  /* Local variables from options */
  char *file_midi = opts.output_file;
  char *file_wav = opts.input_file;
  char *file_patch = opts.patch_file;
  double cut_ratio = opts.cutoff_ratio;
  double rel_cut_ratio = opts.relative_cutoff_ratio;
  long len = opts.fft_size;
  int flag_window = opts.window_type;
  int notetop = opts.top_note;
  int notelow = opts.bottom_note;
  long hop = opts.hop_size;
  int peak_threshold = opts.peak_threshold;
  int flag_phase = opts.use_phase_vocoder;
  int psub_n = opts.drum_removal_bins;
  double psub_f = opts.drum_removal_factor;
  double oct_f = opts.octave_removal_factor;


  struct WAON_notes *notes = WAON_notes_init();
  CHECK_MALLOC (notes, "main");
  char vel[128];     // velocity at the current step
  int on_event[128]; // event index of struct WAON_notes.
  for (i = 0; i < 128; i ++)
    {
      vel[i]      = 0;
      on_event[i] = -1;
    }

  // allocate buffers
  double *left  = (double *)malloc (sizeof (double) * len);
  double *right = (double *)malloc (sizeof (double) * len);
  CHECK_MALLOC (left,  "main");
  CHECK_MALLOC (right, "main");

  double *x = NULL; /* wave data for FFT  */
  double *y = NULL; /* spectrum data for FFT */ 
#ifdef FFTW2
  x = (double *)malloc (sizeof (double) * len);
  y = (double *)malloc (sizeof (double) * len);
#else // FFTW3
  x = (double *)fftw_malloc (sizeof (double) * len);
  y = (double *)fftw_malloc (sizeof (double) * len);
#endif // FFTW2
  CHECK_MALLOC (x, "main");
  CHECK_MALLOC (y, "main");

  /* power spectrum  */
  double *p = (double *)malloc (sizeof (double) * (len / 2 + 1));
  CHECK_MALLOC (p, "main");

  double *p0 = NULL;
  double *dphi = NULL;
  double *ph0 = NULL;
  double *ph1 = NULL;
  if (flag_phase != 0)
    {
      p0 = (double *)malloc (sizeof (double) * (len / 2 + 1));
      CHECK_MALLOC (p0, "main");

      dphi = (double *)malloc (sizeof (double) * (len / 2 + 1));
      CHECK_MALLOC (dphi, "main");

      ph0 = (double *)malloc (sizeof (double) * (len/2+1));
      ph1 = (double *)malloc (sizeof (double) * (len/2+1));
      CHECK_MALLOC (ph0, "main");
      CHECK_MALLOC (ph1, "main");
    }

  double *pmidi = (double *)malloc (sizeof (double) * 128);
  CHECK_MALLOC (pmidi, "main");


  // MIDI output
  if (file_midi == NULL)
    {
      file_midi = (char *)malloc (sizeof (char) * (strlen("output.mid") + 1));
      CHECK_MALLOC (file_midi, "main");
      strcpy (file_midi, "output.mid");
    }

  // open input wav file
  if (file_wav == NULL)
    {
      file_wav = (char *) malloc (sizeof (char) * 2);
      CHECK_MALLOC (file_wav, "main");
      file_wav [0] = '-';
    }
  SF_INFO sfinfo;
  SNDFILE *sf = sf_open (file_wav, SFM_READ, &sfinfo);
  if (sf == NULL)
    {
      fprintf (stderr, "Can't open input file %s : %s\n",
	       file_wav, strerror (errno));
      exit (1);
    }
  sndfile_print_info (&sfinfo);


  // check stereo or mono
  if (sfinfo.channels != 2 && sfinfo.channels != 1)
    {
      fprintf (stderr, "only mono and stereo inputs are supported.\n");
      exit (1);
    }


  // time-period for FFT (inverse of smallest frequency)
  double t0 = (double)len/(double)sfinfo.samplerate;

  // weight of window function for FFT
  double den = init_den (len, flag_window);

  /* set range to analyse (search notes) */
  /* -- after 't0' is calculated  */
  int i0 = (int)(mid2freq[notelow]*t0 - 0.5);
  int i1 = (int)(mid2freq[notetop]*t0 - 0.5)+1;
  if (i0 <= 0)
    {
      i0 = 1; // i0=0 means DC component (frequency = 0)
    }
  if (i1 >= (len/2))
    {
      i1 = len/2 - 1;
    }

  // init patch
  init_patch (file_patch, len, flag_window);
  /*                      ^^^ len could be given by option separately  */

  // initialization plan for FFTW
#ifdef FFTW2
  rfftw_plan plan;
  plan = rfftw_create_plan (len, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
#else // FFTW3
  fftw_plan plan;
  plan = fftw_plan_r2r_1d (len, x, y, FFTW_R2HC, FFTW_ESTIMATE);
#endif

  // for first step
  if (hop != len)
    {
      if (sndfile_read (sf, sfinfo,
			left + hop,
			right + hop,
			(len - hop))
	  != (len - hop))
	{
	  fprintf (stderr, "No Wav Data!\n");
	  exit(0);
	}
    }

  /* Initialize progress bar if requested */
  progress_bar_t *progress = NULL;
  long total_frames = 0;
  if (opts.show_progress && !opts.quiet) {
    /* Estimate total frames from file info */
    total_frames = sfinfo.frames / hop;
    progress = progress_bar_init(total_frames, "Processing");
  }

  /** main loop (icnt) **/
  pitch_shift = 0.0;
  n_pitch = 0;
  int icnt; /* counter  */
  for (icnt=0; ; icnt++)
    {
      // shift
      for (i = 0; i < len - hop; i ++)
	{
	  if (sfinfo.channels == 2) // stereo
	    {
	      left  [i] = left  [i + hop];
	      right [i] = right [i + hop];
	    }
	  else // mono
	    {
	      left  [i] = left  [i + hop];
	    }
	}
      // read from wav
      if (sndfile_read (sf, sfinfo,
			left  + (len - hop),
			right + (len - hop),
			hop)
	  != hop)
	{
	  if (!opts.quiet) {
	    fprintf (stderr, "WaoN : end of file.\n");
	  }
	  break;
	}

      // set double table x[] for FFT
      for (i = 0; i < len; i ++)
	{
	  if (sfinfo.channels == 2) // stereo
	    {
	      x [i] = 0.5 * (left [i] + right [i]);
	    }
	  else // mono
	    {
	      x [i] = left [i];
	    }
	}

      /**
       * stage 1: calc power spectrum
       */
      windowing (len, x, flag_window, 1.0, x);

      /* FFTW library  */
#ifdef FFTW2
      rfftw_one (plan, x, y);
#else // FFTW3
      fftw_execute (plan); // x[] -> y[]
#endif

      if (flag_phase == 0)
	{
	  // no phase-vocoder correction
	  HC_to_amp2 (len, y, den, p);
	}
      else
	{
	  // with phase-vocoder correction
	  HC_to_polar2 (len, y, 0, den, p, ph1);

	  if (icnt == 0) // first step, so no ph0[] yet
	    {
	      for (i = 0; i < (len/2+1); ++i) // full span
		{
		  // no correction
		  dphi[i] = 0.0;

		  // backup the phase for the next step
		  p0  [i] = p   [i];
		  ph0 [i] = ph1 [i];
		}	  
	    }
	  else // icnt > 0
	    {
	      // freq correction by phase difference
	      for (i = 0; i < (len/2+1); ++i) // full span
		{
		  double twopi = 2.0 * M_PI;
		  //double dphi;
		  dphi[i] = ph1[i] - ph0[i]
		    - twopi * (double)i / (double)len * (double)hop;
		  for (; dphi[i] >= M_PI; dphi[i] -= twopi);
		  for (; dphi[i] < -M_PI; dphi[i] += twopi);

		  // frequency correction
		  // NOTE: freq is (i / len + dphi) * samplerate [Hz]
		  dphi[i] = dphi[i] / twopi / (double)hop;

		  // backup the phase for the next step
		  p0  [i] = p   [i];
		  ph0 [i] = ph1 [i];

		  // then, average the power for the analysis
		  p[i] = 0.5 *(sqrt (p[i]) + sqrt (p0[i]));
		  p[i] = p[i] * p[i];
		}
	    }
	}

      // drum-removal process
      if (psub_n != 0)
	{
	  power_subtract_ave (len, p, psub_n, psub_f);
	}

      // octave-removal process
      if (oct_f != 0.0)
	{
	  power_subtract_octave (len, p, oct_f);
	}

      /**
       * stage 2: pickup notes
       */
      /* new code
      if (flag_phase == 0)
	{
	  average_FFT_into_midi (len, (double)sfinfo.samplerate,
				 p, NULL,
				 pmidi);
	}
      else
	{
	  average_FFT_into_midi (len, (double)sfinfo.samplerate,
				 p, dphi,
				 pmidi);
	}
      pickup_notes (pmidi,
		    cut_ratio, rel_cut_ratio,
		    notelow, notetop,
		    vel);
      */

      /* old code */
      if (flag_phase == 0)
	{
	  // no phase-vocoder correction
	  note_intensity (p, NULL,
			  cut_ratio, rel_cut_ratio, i0, i1, t0, vel);
	}
      else
	{
	  // with phase-vocoder correction
	  // make corrected frequency (i / len + dphi) * samplerate [Hz]
	  for (i = 0; i < (len/2+1); ++i) // full span
	    {
	      dphi[i] = ((double)i / (double)len + dphi[i])
		* (double)sfinfo.samplerate;
	    }
	  note_intensity (p, dphi,
			  cut_ratio, rel_cut_ratio, i0, i1, t0, vel);
	}

      /**
       * stage 3: check previous time for note-on/off
       */
      WAON_notes_check (notes, icnt, vel, on_event,
			8, 0, peak_threshold);

      /* Update progress bar */
      if (progress) {
        progress_bar_update(progress, icnt);
      }
    }


  // clean notes
  WAON_notes_regulate (notes);

  WAON_notes_remove_shortnotes (notes, 1, 64);
  WAON_notes_remove_shortnotes (notes, 2, 28);

  WAON_notes_remove_octaves (notes);


  /*
  pitch_shift /= (double) n_pitch;
  fprintf (stderr, "WaoN : difference of pitch = %f ( + %f )\n",
	   -(pitch_shift - 0.5),
	   adj_pitch);
  */

  /* div is the divisions for one beat (quater-note).
   * here we assume 120 BPM, that is, 1 beat is 0.5 sec.
   * note: (hop / ft->rate) = duration for 1 step (sec) */
  long div = (long)(0.5 * (double)sfinfo.samplerate / (double) hop);
  if (!opts.quiet) {
    fprintf (stderr, "division = %ld\n", div);
    fprintf (stderr, "WaoN : # of events = %d\n", notes->n);
  }

  WAON_notes_output_midi (notes, div, file_midi);


#ifdef FFTW2
  rfftw_destroy_plan (plan);
#else
  fftw_destroy_plan (plan);
#endif /* FFTW2 */


  WAON_notes_free (notes);
  free (left);
  free (right);
  free (x);
  free (y);
  free (p);
  if (p0 != NULL) free (p0);
  if (dphi != NULL) free (dphi);
  if (ph0 != NULL) free (ph0);
  if (ph1 != NULL) free (ph1);

  if (pmidi != NULL) free (pmidi);

  /* Clean up progress bar */
  if (progress) {
    progress_bar_finish(progress);
    progress_bar_free(progress);
  }

  /* Note: file_wav and file_midi are now managed by opts structure */
  waon_options_free(&opts);

  sf_close (sf);

  return 0;
}
