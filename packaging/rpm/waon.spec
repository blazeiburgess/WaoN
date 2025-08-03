Name:           waon
Version:        0.11
Release:        1%{?dist}
Summary:        A Wave-to-Notes Transcriber and Some Utility Tools

License:        GPLv2+
URL:            https://github.com/blazeiburgess/WaoN
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  fftw3-devel
BuildRequires:  libsndfile-devel
BuildRequires:  libao-devel
BuildRequires:  libsamplerate-devel
BuildRequires:  gtk2-devel
BuildRequires:  ncurses-devel
BuildRequires:  python3-devel
BuildRequires:  pybind11-devel
BuildRequires:  python3-numpy

Requires:       fftw3
Requires:       libsndfile
Requires:       libao
Requires:       libsamplerate
Requires:       gtk2

%description
WaoN is a Wave-to-Notes transcriber (converts audio file into midi file)
and some utility tools such as gWaoN, graphical visualization of the
spectra, and phase vocoder for time-stretching and pitch-shifting.

%package -n libwaon
Summary:        WaoN shared library
%description -n libwaon
This package contains the shared library for WaoN, which can be used
by other applications and language bindings.

%package -n libwaon-devel
Summary:        Development files for WaoN
Requires:       libwaon%{?_isa} = %{version}-%{release}
Requires:       fftw3-devel
Requires:       libsndfile-devel

%description -n libwaon-devel
This package contains the development headers for building applications
that use the WaoN library.

%package -n python3-waon
Summary:        Python bindings for WaoN
Requires:       libwaon%{?_isa} = %{version}-%{release}
Requires:       python3-numpy

%description -n python3-waon
This package contains Python 3 bindings for the WaoN library,
allowing Python programs to transcribe audio files to MIDI.

%prep
%autosetup

%build
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WAON=ON \
    -DBUILD_PV=ON \
    -DBUILD_GWAON=ON \
    -DBUILD_SHARED_LIB=ON \
    -DBUILD_PYTHON_BINDINGS=ON

%cmake_build

%install
%cmake_install

# Install Python module using CMake-configured setup.py
# Copy README.md to avoid path access issues
cp README.md %{_vpath_builddir}/
# The extension is already built in the correct location by CMake
# Just ensure the waon package directory has all necessary files
cd %{_vpath_builddir}/python
%py3_install
cd ../..

# Install man pages
mkdir -p %{buildroot}%{_mandir}/man1
install -m 644 docs/man/waon.1 %{buildroot}%{_mandir}/man1/
install -m 644 docs/man/pv.1 %{buildroot}%{_mandir}/man1/
install -m 644 docs/man/gwaon.1 %{buildroot}%{_mandir}/man1/

%files
%license LICENSE
%doc README.md docs/TIPS docs/ChangeLog
%{_bindir}/waon
%{_bindir}/pv
%{_bindir}/gwaon
%{_mandir}/man1/waon.1*
%{_mandir}/man1/pv.1*
%{_mandir}/man1/gwaon.1*

%files -n libwaon
%license LICENSE
%{_libdir}/libwaon.so.*

%files -n libwaon-devel
%{_includedir}/waon.h
%{_libdir}/libwaon.so

%files -n python3-waon
%license LICENSE
%doc python/README.md
%{python3_sitearch}/waon/
%{python3_sitearch}/waon-*.egg-info/

%changelog
* Mon Jan 01 2024 WaoN Development Team <kichiki@users.sourceforge.net> - 0.11-1
- New upstream release
- Added Python bindings
- Modernized CLI interface
- Improved project structure
- Added shared library support

* Mon Nov 05 2007 Kengo Ichiki <kichiki@users.sourceforge.net> - 0.9-1
- Update for waon-0.9

* Wed Oct 10 2007 Kengo Ichiki <kichiki@users.sourceforge.net> - 0.1-1
- Initial RPM release