"""WaoN - Wave-to-Notes transcriber

A library for transcribing audio files to MIDI format.
"""

from ._waon import (
    Transcriber,
    Options,
    WindowType,
    ErrorCode,
    WaonError,
    version_string,
    version
)

__version__ = version_string()
__all__ = [
    'Transcriber',
    'Options', 
    'WindowType',
    'ErrorCode',
    'WaonError',
    'version_string',
    'version',
    'transcribe',
    'transcribe_file'
]

def transcribe_file(input_file, output_file, **kwargs):
    """Convenience function to transcribe an audio file to MIDI.
    
    Args:
        input_file: Path to input audio file
        output_file: Path to output MIDI file
        **kwargs: Optional parameters:
            - fft_size: FFT size (default: 2048)
            - hop_size: Hop size (default: fft_size/4)
            - window: Window type (default: WindowType.HANNING)
            - cutoff: Cutoff ratio in log10 (default: -5.0)
            - note_bottom: Bottom MIDI note (default: 48)
            - note_top: Top MIDI note (default: 72)
            - phase_vocoder: Enable phase vocoder (default: True)
            - drum_removal_bins: Number of bins for drum removal (default: 0)
            - drum_removal_factor: Drum removal factor (default: 0.0)
            - octave_removal_factor: Octave removal factor (default: 0.0)
            - progress_callback: Progress callback function
    """
    transcriber = Transcriber()
    options = Options()
    
    # Set options from kwargs
    if 'fft_size' in kwargs:
        options.set_fft_size(kwargs['fft_size'])
    if 'hop_size' in kwargs:
        options.set_hop_size(kwargs['hop_size'])
    if 'window' in kwargs:
        options.set_window(kwargs['window'])
    if 'cutoff' in kwargs:
        options.set_cutoff(kwargs['cutoff'])
    if 'note_bottom' in kwargs and 'note_top' in kwargs:
        options.set_note_range(kwargs['note_bottom'], kwargs['note_top'])
    elif 'note_bottom' in kwargs or 'note_top' in kwargs:
        raise ValueError("Both note_bottom and note_top must be specified")
    if 'phase_vocoder' in kwargs:
        options.set_phase_vocoder(kwargs['phase_vocoder'])
    if 'drum_removal_bins' in kwargs and 'drum_removal_factor' in kwargs:
        options.set_drum_removal(kwargs['drum_removal_bins'], kwargs['drum_removal_factor'])
    elif 'drum_removal_bins' in kwargs or 'drum_removal_factor' in kwargs:
        raise ValueError("Both drum_removal_bins and drum_removal_factor must be specified")
    if 'octave_removal_factor' in kwargs:
        options.set_octave_removal(kwargs['octave_removal_factor'])
    
    # Set progress callback if provided
    if 'progress_callback' in kwargs:
        transcriber.set_progress_callback(kwargs['progress_callback'])
    
    transcriber.transcribe(input_file, output_file, options)

def transcribe(audio_data, sample_rate, output_file, **kwargs):
    """Convenience function to transcribe audio data to MIDI.
    
    Args:
        audio_data: NumPy array of audio samples (mono or stereo)
        sample_rate: Sample rate in Hz
        output_file: Path to output MIDI file
        **kwargs: Same as transcribe_file
    """
    transcriber = Transcriber()
    options = Options()
    
    # Set options from kwargs (same as transcribe_file)
    if 'fft_size' in kwargs:
        options.set_fft_size(kwargs['fft_size'])
    if 'hop_size' in kwargs:
        options.set_hop_size(kwargs['hop_size'])
    if 'window' in kwargs:
        options.set_window(kwargs['window'])
    if 'cutoff' in kwargs:
        options.set_cutoff(kwargs['cutoff'])
    if 'note_bottom' in kwargs and 'note_top' in kwargs:
        options.set_note_range(kwargs['note_bottom'], kwargs['note_top'])
    elif 'note_bottom' in kwargs or 'note_top' in kwargs:
        raise ValueError("Both note_bottom and note_top must be specified")
    if 'phase_vocoder' in kwargs:
        options.set_phase_vocoder(kwargs['phase_vocoder'])
    if 'drum_removal_bins' in kwargs and 'drum_removal_factor' in kwargs:
        options.set_drum_removal(kwargs['drum_removal_bins'], kwargs['drum_removal_factor'])
    elif 'drum_removal_bins' in kwargs or 'drum_removal_factor' in kwargs:
        raise ValueError("Both drum_removal_bins and drum_removal_factor must be specified")
    if 'octave_removal_factor' in kwargs:
        options.set_octave_removal(kwargs['octave_removal_factor'])
    
    # Set progress callback if provided
    if 'progress_callback' in kwargs:
        transcriber.set_progress_callback(kwargs['progress_callback'])
    
    transcriber.transcribe_data(audio_data, sample_rate, output_file, options)