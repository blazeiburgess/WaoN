# WaoN Python Bindings

Python bindings for WaoN - Wave-to-Notes transcriber.

## Installation

### Building from Source

The Python bindings require the WaoN shared library to be built first. From the project root:

```bash
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIB=ON -DBUILD_PYTHON_BINDINGS=ON
make
```

### Installing with pip

Once built, you can install the Python package:

```bash
cd python
pip install .
```

For development installation:

```bash
pip install -e .
```

## Requirements

- Python >= 3.6
- NumPy >= 1.16.0
- pybind11 >= 2.6.0 (for building)
- CMake >= 3.12 (for building)

## Usage

### Basic Usage

```python
import waon

# Simple transcription with default settings
waon.transcribe_file("input.wav", "output.mid")
```

### Advanced Usage

```python
import waon

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

# Optional: set progress callback
def progress_callback(progress):
    print(f"Progress: {progress * 100:.1f}%")

transcriber.set_progress_callback(progress_callback)

# Perform transcription
transcriber.transcribe("input.wav", "output.mid", options)
```

### Using NumPy Arrays

```python
import waon
import numpy as np

# Generate or load audio data
sample_rate = 44100
audio_data = np.random.randn(sample_rate * 2)  # 2 seconds of noise

# Transcribe
waon.transcribe(
    audio_data,
    sample_rate,
    "output.mid",
    fft_size=2048,
    cutoff=-5.0,
    note_bottom=48,
    note_top=84
)
```

## API Reference

### Classes

#### `Transcriber`
Main transcription class.

- `transcribe(input_file, output_file, options=None)`: Transcribe audio file to MIDI
- `transcribe_data(audio_data, sample_rate, output_file, options=None)`: Transcribe NumPy array to MIDI
- `set_progress_callback(callback)`: Set progress callback function

#### `Options`
Configuration options for transcription.

- `set_fft_size(size)`: Set FFT size (must be power of 2)
- `set_hop_size(size)`: Set hop size (0 = auto, default is fft_size/4)
- `set_window(window)`: Set window type (WindowType enum)
- `set_cutoff(cutoff)`: Set cutoff ratio (log10)
- `set_note_range(bottom, top)`: Set MIDI note range to analyze
- `set_phase_vocoder(enable)`: Enable/disable phase vocoder
- `set_drum_removal(bins, factor)`: Set drum removal parameters
- `set_octave_removal(factor)`: Set octave removal factor

### Enums

#### `WindowType`
- `NONE`: No windowing
- `PARZEN`: Parzen window
- `WELCH`: Welch window
- `HANNING`: Hanning window (default)
- `HAMMING`: Hamming window
- `BLACKMAN`: Blackman window
- `STEEPER`: Steeper 30-dB/octave rolloff window

#### `ErrorCode`
- `SUCCESS`: Operation successful
- `MEMORY`: Memory allocation failed
- `FILE_NOT_FOUND`: File not found
- `FILE_FORMAT`: Invalid file format
- `INVALID_PARAM`: Invalid parameter
- `IO`: I/O error
- `INTERNAL`: Internal error

### Functions

- `version_string()`: Get library version string
- `version()`: Get library version as tuple (major, minor, patch)
- `transcribe_file(input_file, output_file, **kwargs)`: Convenience function for file transcription
- `transcribe(audio_data, sample_rate, output_file, **kwargs)`: Convenience function for array transcription

### Exceptions

- `WaonError`: Exception raised for WaoN-specific errors

## Examples

See the `examples/` directory for more detailed examples:

- `basic_transcription.py`: Simple file transcription
- `advanced_transcription.py`: Advanced options and progress callback
- `numpy_transcription.py`: Using NumPy arrays

## License

The Python bindings are released under the same license as WaoN (GPL v2 or later).