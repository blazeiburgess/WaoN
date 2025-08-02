#!/usr/bin/env python
"""Basic example of using WaoN Python bindings."""

import waon
import sys

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_audio_file> <output_midi_file>")
        print("Example: python basic_transcription.py input.wav output.mid")
        return 1
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"WaoN version: {waon.version_string()}")
    print(f"Transcribing {input_file} to {output_file}...")
    
    try:
        # Simple transcription with default settings
        waon.transcribe_file(input_file, output_file)
        print("Transcription completed successfully!")
        
    except waon.WaonError as e:
        print(f"WaoN error: {e}")
        return 1
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())