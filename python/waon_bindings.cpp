/* waon_bindings.cpp - Python bindings for WaoN library
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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <waon.h>
#include <string>
#include <stdexcept>

namespace py = pybind11;

// Exception class for WaoN errors
class WaonError : public std::runtime_error {
public:
    waon_error_t error_code;
    
    WaonError(waon_error_t code) : 
        std::runtime_error(waon_error_string(code)), 
        error_code(code) {}
};

// RAII wrapper for waon_context_t
class WaonContext {
private:
    waon_context_t* ctx;
    
public:
    WaonContext() {
        ctx = waon_create();
        if (!ctx) {
            throw std::runtime_error("Failed to create WaoN context");
        }
    }
    
    ~WaonContext() {
        if (ctx) {
            waon_destroy(ctx);
        }
    }
    
    // Disable copy
    WaonContext(const WaonContext&) = delete;
    WaonContext& operator=(const WaonContext&) = delete;
    
    // Enable move
    WaonContext(WaonContext&& other) noexcept : ctx(other.ctx) {
        other.ctx = nullptr;
    }
    
    WaonContext& operator=(WaonContext&& other) noexcept {
        if (this != &other) {
            if (ctx) waon_destroy(ctx);
            ctx = other.ctx;
            other.ctx = nullptr;
        }
        return *this;
    }
    
    waon_context_t* get() { return ctx; }
    
    void check_error(waon_error_t err) {
        if (err != WAON_SUCCESS) {
            throw WaonError(err);
        }
    }
};

// RAII wrapper for waon_options_t
class WaonOptions {
private:
    waon_options_t* opts;
    
public:
    WaonOptions() {
        opts = waon_options_create();
        if (!opts) {
            throw std::runtime_error("Failed to create WaoN options");
        }
    }
    
    ~WaonOptions() {
        if (opts) {
            waon_options_destroy(opts);
        }
    }
    
    // Disable copy
    WaonOptions(const WaonOptions&) = delete;
    WaonOptions& operator=(const WaonOptions&) = delete;
    
    // Enable move
    WaonOptions(WaonOptions&& other) noexcept : opts(other.opts) {
        other.opts = nullptr;
    }
    
    WaonOptions& operator=(WaonOptions&& other) noexcept {
        if (this != &other) {
            if (opts) waon_options_destroy(opts);
            opts = other.opts;
            other.opts = nullptr;
        }
        return *this;
    }
    
    waon_options_t* get() { return opts; }
    
    void set_fft_size(int size) {
        auto err = waon_options_set_fft_size(opts, size);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_hop_size(int size) {
        auto err = waon_options_set_hop_size(opts, size);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_window(waon_window_t window) {
        auto err = waon_options_set_window(opts, window);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_cutoff(double cutoff) {
        auto err = waon_options_set_cutoff(opts, cutoff);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_note_range(int bottom, int top) {
        auto err = waon_options_set_note_range(opts, bottom, top);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_phase_vocoder(bool enable) {
        auto err = waon_options_set_phase_vocoder(opts, enable ? 1 : 0);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_drum_removal(int bins, double factor) {
        auto err = waon_options_set_drum_removal(opts, bins, factor);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
    
    void set_octave_removal(double factor) {
        auto err = waon_options_set_octave_removal(opts, factor);
        if (err != WAON_SUCCESS) throw WaonError(err);
    }
};

// Main transcriber class
class WaonTranscriber {
private:
    WaonContext context;
    std::function<void(double)> progress_callback;
    
    static void progress_callback_wrapper(double progress, void* user_data) {
        auto* self = static_cast<WaonTranscriber*>(user_data);
        if (self->progress_callback) {
            py::gil_scoped_acquire acquire;
            self->progress_callback(progress);
        }
    }
    
public:
    WaonTranscriber() = default;
    
    void set_progress_callback(std::function<void(double)> callback) {
        progress_callback = callback;
        if (callback) {
            waon_set_progress_callback(context.get(), progress_callback_wrapper, this);
        } else {
            waon_set_progress_callback(context.get(), nullptr, nullptr);
        }
    }
    
    void transcribe(const std::string& input_file, 
                   const std::string& output_file,
                   WaonOptions* options = nullptr) {
        waon_error_t err = waon_transcribe(
            context.get(), 
            input_file.c_str(), 
            output_file.c_str(),
            options ? options->get() : nullptr
        );
        context.check_error(err);
    }
    
    void transcribe_data(py::array_t<double> audio_data,
                        int sample_rate,
                        const std::string& output_file,
                        WaonOptions* options = nullptr) {
        auto buf = audio_data.request();
        
        int channels;
        long num_samples;
        
        if (buf.ndim == 1) {
            channels = 1;
            num_samples = buf.shape[0];
        } else if (buf.ndim == 2) {
            if (buf.shape[1] != 1 && buf.shape[1] != 2) {
                throw std::invalid_argument("Audio data must have 1 or 2 channels");
            }
            channels = buf.shape[1];
            num_samples = buf.shape[0] * channels;
        } else {
            throw std::invalid_argument("Audio data must be 1D or 2D array");
        }
        
        waon_error_t err = waon_transcribe_data(
            context.get(),
            static_cast<double*>(buf.ptr),
            num_samples,
            sample_rate,
            channels,
            output_file.c_str(),
            options ? options->get() : nullptr
        );
        context.check_error(err);
    }
};

PYBIND11_MODULE(_waon, m) {
    m.doc() = "Python bindings for WaoN - Wave-to-Notes transcriber";
    
    // Version info
    m.def("version_string", &waon_version_string, "Get library version string");
    m.def("version", []() {
        int major, minor, patch;
        waon_version(&major, &minor, &patch);
        return py::make_tuple(major, minor, patch);
    }, "Get library version as tuple (major, minor, patch)");
    
    // Error codes enum
    py::enum_<waon_error_t>(m, "ErrorCode")
        .value("SUCCESS", WAON_SUCCESS)
        .value("MEMORY", WAON_ERROR_MEMORY)
        .value("FILE_NOT_FOUND", WAON_ERROR_FILE_NOT_FOUND)
        .value("FILE_FORMAT", WAON_ERROR_FILE_FORMAT)
        .value("INVALID_PARAM", WAON_ERROR_INVALID_PARAM)
        .value("IO", WAON_ERROR_IO)
        .value("INTERNAL", WAON_ERROR_INTERNAL);
    
    // Window types enum
    py::enum_<waon_window_t>(m, "WindowType")
        .value("NONE", WAON_WINDOW_NONE)
        .value("PARZEN", WAON_WINDOW_PARZEN)
        .value("WELCH", WAON_WINDOW_WELCH)
        .value("HANNING", WAON_WINDOW_HANNING)
        .value("HAMMING", WAON_WINDOW_HAMMING)
        .value("BLACKMAN", WAON_WINDOW_BLACKMAN)
        .value("STEEPER", WAON_WINDOW_STEEPER);
    
    // WaonError exception
    py::register_exception<WaonError>(m, "WaonError");
    
    // Options class
    py::class_<WaonOptions>(m, "Options", "WaoN transcription options")
        .def(py::init<>())
        .def("set_fft_size", &WaonOptions::set_fft_size, 
             "Set FFT size (must be power of 2)",
             py::arg("size"))
        .def("set_hop_size", &WaonOptions::set_hop_size,
             "Set hop size (0 = auto, default is fft_size/4)",
             py::arg("size"))
        .def("set_window", &WaonOptions::set_window,
             "Set window type",
             py::arg("window"))
        .def("set_cutoff", &WaonOptions::set_cutoff,
             "Set cutoff ratio (log10)",
             py::arg("cutoff"))
        .def("set_note_range", &WaonOptions::set_note_range,
             "Set MIDI note range to analyze",
             py::arg("bottom"), py::arg("top"))
        .def("set_phase_vocoder", &WaonOptions::set_phase_vocoder,
             "Enable/disable phase vocoder",
             py::arg("enable"))
        .def("set_drum_removal", &WaonOptions::set_drum_removal,
             "Set drum removal parameters",
             py::arg("bins"), py::arg("factor"))
        .def("set_octave_removal", &WaonOptions::set_octave_removal,
             "Set octave removal factor",
             py::arg("factor"));
    
    // Transcriber class
    py::class_<WaonTranscriber>(m, "Transcriber", "WaoN audio-to-MIDI transcriber")
        .def(py::init<>())
        .def("transcribe", &WaonTranscriber::transcribe,
             "Transcribe audio file to MIDI",
             py::arg("input_file"),
             py::arg("output_file"),
             py::arg("options") = nullptr)
        .def("transcribe_data", &WaonTranscriber::transcribe_data,
             "Transcribe audio data to MIDI",
             py::arg("audio_data"),
             py::arg("sample_rate"),
             py::arg("output_file"),
             py::arg("options") = nullptr)
        .def("set_progress_callback", &WaonTranscriber::set_progress_callback,
             "Set progress callback function",
             py::arg("callback"));
}