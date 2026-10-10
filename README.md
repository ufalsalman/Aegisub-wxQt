# Aegisub

For binaries and general information [see the homepage](http://aegisub.org).

The bug tracker can be found at https://github.com/TypesettingTools/Aegisub/issues.

Support is available on [Discord](https://discord.com/invite/AZaVyPr) or [IRC](irc://irc.rizon.net/aegisub).

## Building Aegisub

### Windows

Prerequisites:

1. Visual Studio (Community edition of any recent version is fine, needs the Windows SDK included)
2. Python 3
3. Meson
4. CMake

There are a few optional dependencies that must be installed and on your PATH:

1. msgfmt, to build the translations (installing from https://mlocati.github.io/articles/gettext-iconv-windows.html seems to be the easiest option)
2. InnoSetup, to build the regular installer (iscc.exe on your PATH)
3. 7zip, to build the regular installer (7z.exe on your PATH)

All other dependencies are either stored in the repository or are included as submodules.

Building:

1. Clone Aegisub's repository: `git clone https://github.com/TypesettingTools/Aegisub.git`
2. From the Visual Studio "x64 Native Tools Command Prompt", generate the build directory: `meson build -Ddefault_library=static` (if building for release, add `--buildtype=release`)
3. Build with `cd build` and `ninja`

You should now have a binary: `aegisub.exe`.

Installer:

You can generate the installer with `ninja win-installer` after a successful build. This assumes a working internet connection and installation of the optional dependencies.

You can generate the portable zip with `ninja win-portable` after a successful build.

### OS X

A vaguely recent version of Xcode and the corresponding command-line tools are required.

For personal usage, you can use pip and homebrew to install almost all of Aegisub's dependencies:

    pip3 install meson      # or brew install meson if you installed Python via brew
    brew install cmake ninja pkg-config  libass boost zlib ffms2 fftw hunspell uchardet
    export LDFLAGS="-L/usr/local/opt/icu4c/lib"
    export CPPFLAGS="-I/usr/local/opt/icu4c/include"
    export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig"

When compiling on Apple Silicon, replace `/usr/local` with `/opt/homebrew`.

Once the dependencies are installed, build Aegisub with `meson build && meson compile -C build`.

#### Build a local DMG

Homebrew bottles target the macOS release they were built for, which may be
newer than Aegisub's default deployment target. For a local package, target
the current macOS major release and explicitly request an ad-hoc signature.
The resulting DMG is for development and personal use on that macOS release;
release packages use CI's source-built dependencies and the
[macOS release-signing process](docs/developer_docs.md#macos-release-signing).

```bash
deployment_target="$(sw_vers -productVersion)"
meson setup build_static \
  -Ddefault_library=static \
  -Dbuildtype=debugoptimized \
  -Dbuild_osx_bundle=true \
  -Dmacos_deployment_target="${deployment_target}" \
  --force-fallback-for=boost
meson compile -C build_static
meson test -C build_static --verbose
export AEGISUB_BUNDLE_SIGNATURE=-
meson compile osx-bundle -C build_static
meson compile osx-build-dmg -C build_static
```

### Linux or other

#### Build dependencies for Debian-based systems

```
compiler:    build-essential
pkgconfig:   pkg-config  or  pkgconf
meson:       meson ninja-build
gettext:     gettext intltool
fontconfig:  libfontconfig1-dev
libass:      libass-dev
boost:       libboost-chrono-dev libboost-locale-dev libboost-regex-dev libboost-system-dev libboost-thread-dev
zlib:        zlib1g-dev
WxWidgets:   wx3.2-headers libwxgtk3.2-dev
ICU:         icu-devtools libicu-dev
pulse-audio: libpulse-dev
ALSA:        libasound2-dev
OpenAL:      libopenal-dev
ffms2:       libffms2-dev
fftw3:       libfftw3-dev
hunspell:    libhunspell-dev
uchardet:    libuchardet-dev
libcurl:     libcurl4-openssl-dev  or  libcurl4-gnutls-dev
opengl:      libgl1-mesa-dev
gtest:       libgtest-dev
gmock:       libgmock-dev
libportal:   libportal-gtk3-dev
```

I.e. to install on Ubuntu 24.04 run this command:
``` bash
sudo apt install build-essential pkg-config meson ninja-build gettext intltool libfontconfig1-dev libass-dev libboost-chrono-dev libboost-locale-dev libboost-regex-dev libboost-system-dev libboost-thread-dev zlib1g-dev wx3.2-headers libwxgtk3.2-dev icu-devtools libicu-dev libpulse-dev libasound2-dev libopenal-dev libffms2-dev libfftw3-dev libhunspell-dev libuchardet-dev libcurl4-gnutls-dev libgl1-mesa-dev libgtest-dev libgmock-dev libportal-gtk3-dev
```

#### Build Aegisub

``` bash
meson setup build --prefix=/usr/local --buildtype=release --strip -Dsystem_luajit=false -Ddefault_library=static
meson compile -C build
meson install -C build --skip-subprojects luajit
```

#### Packaging
If you are packaging Aegisub for a Linux distribution, here are a few things you may need to know:
- Aegisub cannot be built with LTO (See: https://github.com/TypesettingTools/Aegisub/issues/290).
- Aegisub depends on LuaJIT and *requires* LuaJIT to be build with Lua 5.2 compatibility enabled.
  We are aware that most distributions do not compile LuaJIT with this flag, and that this complicates packaging for them, see https://github.com/TypesettingTools/Aegisub/issues/239 for a detailed discussion of the situation.

  Like for its other dependencies, Aegisub includes a meson subproject for LuaJIT that can be used to statically link a version of LuaJIT with 5.2 compatibility.
  For distributions that do not allow downloading additional sources at build time, the downloaded LuaJIT subproject is included in the source tarballs distributed with releases.
- When linked against libstdc++, Aegisub needs libstdc++ 6.0.32 or later due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95048.
  Aegisub's tests will detect this bug, but if you're not running tests on packaging you'll need to make sure the libstdc++ version is recent enough.
- Aegisub uses OpenGL through wxWidgets. For Aegisub to work directly on Wayland (as opposed to Xwayland), wxWidgets needs to be built with EGL enabled.
  Aegisub will automatically fall back to X11 when it detects missing EGL support.

The following commands are an example for how to build Aegisub with the goal of creating a distribution package:

```bash
meson subprojects download luajit              # Or use the tarball
meson subprojects packagefiles --apply luajit

meson setup builddir --wrap-mode=nodownload --prefix=/usr --buildtype=release -Dsystem_luajit=false -Ddefault_library=static -Dtests=false

meson compile -C builddir
meson install -C builddir --skip-subprojects luajit
```

## Developer Documenation
Some documentation for developers is available in [docs/developer_docs.md](docs/developer_docs.md).

## License

All files in this repository are licensed under various GPL-compatible BSD-style licenses; see LICENCE and the individual source files for more information.
The official Windows and OS X builds are GPLv2 due to including fftw3.

## wxQt Edition

This is an unofficial fork of Aegisub that builds against [wxWidgets](https://github.com/wxWidgets/wxWidgets) using its Qt backend (`wxBUILD_TOOLKIT=qt`), so it integrates with Qt-based Linux desktops (developed on Arch Linux with KDE Plasma). Licences are unchanged from upstream.

Differences from upstream:

- **Theme-aware subtitle grid and edit box.** Colours follow the Qt palette (light/dark) instead of fixed defaults, and Qt palette changes are forwarded to wx as `wxEVT_SYS_COLOUR_CHANGED`, so the UI re-styles itself when the system theme changes.
- **wxQt fix set** (maintained in the `wxQt` branch of our [wxWidgets fork](https://github.com/ufalsalman/wxWidgets), on top of wxWidgets revision `e320e82ec94df7ff5bb85b73344407190ce1d4f6`): mouse wheel events report the correct axis and rotation, GL content rendered outside of paint events is composited immediately (`wxGLCanvas::SwapBuffers`), and out-of-range `wxListBox` selections are ignored instead of dereferencing a null item (this used to abort when opening a video whose colour matrix differs from the subtitle file's, if a stale choice was saved in the configuration).
- **Versioning:** non-tagged builds report themselves in the upstream format `<revision>-<branch>-<hash>` — the `wxQt` branch name already identifies the edition (see `tools/version.sh`).

### Building on Linux

The maintained build path is the *aegisub-wxqt* build kit (`build.py`), which clones this fork and our wxWidgets fork (both on the `wxQt` branch), builds wxWidgets with the Qt toolkit into a private prefix, then configures and builds this repository with Meson. Manual equivalent:

1. Build wxWidgets at the pinned revision with `-DwxBUILD_TOOLKIT=qt -DwxUSE_OPENGL=ON -DwxUSE_STC=ON` plus the wxQt fix set above.
2. `meson setup builddir` (point `wx-config` at the private prefix), then `meson compile -C builddir` and `meson install -C builddir --skip-subprojects luajit`.

### Diagnostics: `AEGISUB_PROFILE_SEEK`

Setting `AEGISUB_PROFILE_SEEK=1` in the environment enables end-to-end instrumentation of the video seek path: active line change → worker frame render → frame ready on the UI thread → GL paint.

Why it is kept in the tree instead of being removed after debugging: the class of regression it watches — *a frame that is rendered but never becomes visible, or is delivered seconds late* — is invisible during normal use and cannot be caught by screenshots or by timing individual stages, yet this is exactly how wxQt failed in practice (video appearing to seek only every few seconds). The instrumentation costs nothing when the variable is unset (a one-time `getenv`), and its `[seek]` lines carry steady-clock microsecond stamps so entries written by the UI thread and the video worker thread can be correlated even when they interleave.

Usage:

```
AEGISUB_PROFILE_SEEK=1 aegisub file.ass 2> seek.log
```

Line meanings:

- `[seek] active_line row=…` — a row was activated and the video seek was requested (this is t0).
- `[seek] worker_done frame=… render_ms=…` — the video worker finished decoding + subtitle rendering.
- `[seek] frame_ready … dt_click_ms=…` — the frame arrived on the UI thread; `dt_click_ms` is the click → ready latency.
- `[seek] paintGL …` — the GL content actually became visible on screen. This line is emitted by the matching probe on the wxWidgets side (its Qt GL canvas), which the same environment variable enables.
