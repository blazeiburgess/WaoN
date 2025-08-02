#!/usr/bin/env python
"""Example of using WaoN with NumPy arrays."""

import waon
import numpy as np
import sys

def generate_test_audio(duration=2.0, sample_rate=44100):
    """Generate a test audio signal with multiple frequencies."""
    t = np.linspace(0, duration, int(duration * sample_rate))
    
    # Create a chord: C4 (261.63 Hz), E4 (329.63 Hz), G4 (392.00 Hz)
    frequencies = [261.63, 329.63, 392.00]
    signal = np.zeros_like(t)
    
    for freq in frequencies:
        signal += 0.3 * np.sin(2 * np.pi * freq * t)
    
    # Add some envelope
    envelope = np.exp(-t * 0.5)
    signal *= envelope
    
    # Normalize
    signal = signal / np.max(np.abs(signal)) * 0.8
    
    return signal

def main():
    if len(sys.argv) > 2:
        print(f"Usage: {sys.argv[0]} [output_midi_file]")
        return 1
    
    output_file = sys.argv[1] if len(sys.argv) == 2 else "test_output.mid"
    
    print(f"WaoN version: {waon.version_string()}")
    print("Generating test audio signal...")
    
    try:
        # Generate test audio
        sample_rate = 44100
        audio_data = generate_test_audio(duration=2.0, sample_rate=sample_rate)
        
        print(f"Audio shape: {audio_data.shape}")
        print(f"Sample rate: {sample_rate} Hz")
        print(f"Duration: {len(audio_data) / sample_rate:.2f} seconds")
        
        # Transcribe with custom options
        print(f"Transcribing to {output_file}...")
        waon.transcribe(
            audio_data,
            sample_rate,
            output_file,
            fft_size=2048,
            cutoff=-5.0,
            note_bottom=48,  # C3
            note_top=84,     # C6
            phase_vocoder=True
        )
        
        print("Transcription completed successfully!")
        print(f"Output saved to: {output_file}")
        
    except waon.WaonError as e:
        print(f"WaoN error: {e}")
        return 1
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())