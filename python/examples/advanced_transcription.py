#!/usr/bin/env python
"""Advanced example of using WaoN Python bindings with custom options."""

import waon
import sys

def progress_callback(progress):
    """Progress callback function."""
    bar_width = 50
    filled = int(bar_width * progress)
    bar = '#' * filled + '-' * (bar_width - filled)
    print(f'\r[{bar}] {progress*100:.1f}%', end='', flush=True)

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_audio_file> <output_midi_file>")
        return 1
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"WaoN version: {waon.version_string()}")
    print(f"Transcribing {input_file} to {output_file} with custom options...")
    
    try:
        # Create transcriber and options
        transcriber = waon.Transcriber()
        options = waon.Options()
        
        # Configure options
        options.set_fft_size(4096)  # Larger FFT for better frequency resolution
        options.set_hop_size(1024)  # Custom hop size
        options.set_window(waon.WindowType.HANNING)
        options.set_cutoff(-4.0)  # More sensitive cutoff
        options.set_note_range(36, 84)  # C2 to C6
        options.set_phase_vocoder(True)  # Enable phase vocoder
        
        # Optional: drum removal
        # options.set_drum_removal(5, 1.5)
        
        # Optional: octave removal
        # options.set_octave_removal(0.5)
        
        # Set progress callback
        transcriber.set_progress_callback(progress_callback)
        
        # Perform transcription
        transcriber.transcribe(input_file, output_file, options)
        
        print("\nTranscription completed successfully!")
        
    except waon.WaonError as e:
        print(f"\nWaoN error: {e}")
        return 1
    except Exception as e:
        print(f"\nError: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())