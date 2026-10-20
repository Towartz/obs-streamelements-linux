# obs-streamelements-linux

Native Linux port of the StreamElements (SE.Live / OBS.Live) plugin for OBS Studio.

Re-implemented in C++ using Qt 6 and `libobs` to provide native docks, API bridge, and authentication without running Windows binaries or Wine.

## Features

- **StreamElements Menu**: Setup links, cloud backup, and dock toggles.
- **Browser Docks**: Activity Feed, Chat, and Media Request powered by QtWebEngine.
- **Local WebSocket Bridge (`ws://127.0.0.1:8092`)**: SE.Live JSON-RPC protocol implementation for overlay events and scene controls.
- **Transition Audio Wrapper**: Registers `streamelements_transition_audio_wrapper_source` for stinger audio.
- **Authentication**:
  - System browser OAuth (bypasses Google's embedded browser restrictions).
  - StreamElements JWT token quick connect.
  - Session cookies persisted in plugin config directory.

## Dependencies

- OBS Studio (30.x or newer) with development headers (`libobs`, `obs-frontend-api`)
- Qt 6 (`Core`, `Widgets`, `WebEngineWidgets`, `WebSockets`, `Network`)
- `libcurl`
- C++20 compiler (`gcc` or `clang`)

### Package installation

**Arch Linux / Manjaro:**
```sh
sudo pacman -S base-devel qt6-base qt6-webengine qt6-websockets curl obs-studio
```

**Debian / Ubuntu:**
```sh
sudo apt install build-essential qt6-base-dev qt6-webengine-dev libqt6websockets6-dev libcurl4-openssl-dev libobs-dev
```

**Fedora:**
```sh
sudo dnf install gcc-c++ qt6-qtbase-devel qt6-qtwebengine-devel qt6-qtwebsockets-devel libcurl-devel obs-studio-devel
```

## Build and Install

```sh
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
make -j$(nproc)
make install
```

The plugin installs to `~/.config/obs-studio/plugins/obs-streamelements/`.

Or with CMake:
```sh
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
cmake --install .
```

## Setup

1. Open OBS Studio.
2. Go to **StreamElements** > **Login**.
3. Sign in via your default browser or paste your JWT token from [streamelements.com/dashboard/account/channels](https://streamelements.com/dashboard/account/channels).

## Uninstall

```sh
rm -rf ~/.config/obs-studio/plugins/obs-streamelements
```

## License

GPL-2.0
