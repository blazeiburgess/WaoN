NOTE: This is an update to the codebase, which can be found [here](https://sourceforge.net/projects/waon/). The README here is rewritten, but is basically entirely taken from the original.

# WaoN - Wave-to-Notes Transcriber

WaoN is a Wave-to-Notes transcriber, a tool that converts audio files (WAV format) into MIDI files by analyzing the frequency spectrum. The name "WaoN" also means "harmony of notes" or "chord" in Japanese.

## Overview

WaoN analyzes audio files and attempts to transcribe the musical notes it detects into standard MIDI format. It's particularly designed for transcribing piano music but can work with other instruments as well.

The project includes three main applications:
- **waon** - The core command-line transcriber
- **pv** - A phase vocoder for pitch shifting and time stretching
- **gwaon** - A GTK+ based GUI version of WaoN

## Features

- Read WAV files and output standard MIDI format 0 files
- Support for stdin/stdout for real-time processing
- No explicit limits on the number of simultaneous notes
- Adjustable pitch detection parameters
- Phase vocoder for audio manipulation
- GUI interface for easier interaction

## Building from Source

### Prerequisites

- CMake 3.10 or higher
- FFTW3 library
- libsndfile
- libao (for audio output)
- libsamplerate
- GTK+ 2.0 (for gwaon)
- ncurses (for pv)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Build Options

You can disable specific components:
```bash
cmake -DBUILD_WAON=ON -DBUILD_PV=ON -DBUILD_GWAON=OFF ..
```

To build with Python bindings:
```bash
cmake -DBUILD_SHARED_LIB=ON -DBUILD_PYTHON_BINDINGS=ON ..
make

# Install the Python package
cd ../python
pip install .
```

## Usage

### Basic WAV to MIDI conversion:
```bash
waon -i input.wav -o output.mid
```

### With custom parameters:
```bash
waon -i input.wav -o output.mid -w 3 -n 4096 -s 2048
```

### Real-time processing with timidity:
```bash
cat input.wav | waon -i - -o - | timidity -id -
```

### For more options:
```bash
waon --help
pv --help
gwaon --help
```

### Python Usage:
```python
import waon

# Simple transcription
waon.transcribe_file("input.wav", "output.mid")

# With custom options
waon.transcribe_file("input.wav", "output.mid", 
                    fft_size=4096, 
                    cutoff=-4.0,
                    note_bottom=36, 
                    note_top=84)
```

## Documentation

- See `docs/TIPS.md` for usage tips and examples
- Man pages are available for each tool (`man waon`, `man pv`, `man gwaon`)
- Check `docs/CHANGELOG.md` for version history

## Project Structure

```
waon/
├── src/
│   ├── common/      # Shared libraries (FFT, sound processing)
│   ├── waon/        # Core transcriber application
│   ├── pv/          # Phase vocoder application  
│   ├── gwaon/       # GTK+ GUI application
│   └── lib/         # Shared library for language bindings
├── python/          # Python bindings
│   ├── waon/        # Python package
│   └── examples/    # Python examples
├── docs/            # Documentation and man pages
├── examples/        # Example files and scripts
└── tests/           # Test suite
```

## License

WaoN is released under the GNU General Public License version 2. See the LICENSE file for details.

## Author

Copyright (C) 1998-2007 Kengo Ichiki <kichiki@users.sourceforge.net>

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## Acknowledgments

This project uses the following libraries:
- FFTW - Fast Fourier Transform library
- libsndfile - Audio file I/O
- GTK+ - GUI toolkit
- libao - Cross-platform audio output