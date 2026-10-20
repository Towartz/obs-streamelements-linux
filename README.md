# obs-streamelements-linux

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52?style=flat-square&logo=qt&logoColor=white)](https://qt.io)
[![OBS Studio](https://img.shields.io/badge/OBS%20Studio-30%2B-302E31?style=flat-square&logo=obsstudio&logoColor=white)](https://obsproject.com)
[![License: GPL-2.0](https://img.shields.io/badge/License-GPL--2.0-blue.svg?style=flat-square)](LICENSE)

Native Linux implementation of the StreamElements (SE.Live / OBS.Live) plugin for OBS Studio.

Re-implemented in modern C++ using Qt 6 and `libobs` to provide native dock integration, overlay communication, and authentication without Wine or Windows runtime layers.

---

## Architecture & Features

| Component | Description | Implementation |
|---|---|---|
| **StreamElements Menu** | Top-level menu bar integration matching official SE.Live branding | Native Qt menu injected before Help |
| **Activity Feed Dock** | Live event stream for followers, tips, subs, and cheers | Chromium-based `QWebEngineView` dock |
| **Chat Dock** | Live stream chat for Twitch, YouTube, and Kick | Persistent profile web dock |
| **Media Request Dock** | Song request playback control and queue management | Embedded StreamElements dock |
| **API WebSocket Server** | Local JSON-RPC bridge on `ws://127.0.0.1:8092` for overlay events | `Qt6WebSockets` event loop |
| **Transition Audio Wrapper** | Registers `streamelements_transition_audio_wrapper_source` | libobs source filter |
| **Cloud Backup** | Packages and restores OBS scenes, collections, and profile data | Native JSON serialization |
| **Authentication System** | Loopback OAuth (RFC 8252) + JWT token fallback | `QTcpServer` + `QWebEngineCookieStore` |
| **Localization** | Multi-language translation support | 50+ official language files included |

---

## Authentication Flow

Google and YouTube disallow OAuth authentication inside embedded web views (`QWebEngineView` / CEF). This plugin resolves the block by providing two connection flows:

1. **System Browser Loopback (RFC 8252)**:
   - Spawns a temporary loopback HTTP server on `127.0.0.1:4269` with Private Network Access and CORS headers.
   - Launches your default system browser (Brave, Chrome, Firefox) where you are already signed in.
   - Catches the redirect token on loopback, sets persistent cookies on `.streamelements.com`, and notifies OBS.

2. **Quick Connect (JWT Token)**:
   - Paste your JWT token directly from your StreamElements account settings.
   - Validates via `https://api.streamelements.com/kappa/v2/channels/me` and injects session cookies directly into the QtWebEngine profile.

---

## Dependencies

### System Requirements
- Linux (x86_64)
- OBS Studio 30.0 or newer (with development headers)
- Qt 6 (Core, Widgets, WebEngineWidgets, WebSockets, Network)
- libcurl
- C++20 compiler (`gcc` 11+ or `clang` 14+)

### Package Installation

#### Arch Linux / Manjaro
```sh
sudo pacman -S base-devel cmake obs-studio qt6-base qt6-webengine qt6-websockets curl
```

#### Ubuntu / Debian (22.04+ / 24.04+)
```sh
sudo apt update
sudo apt install build-essential cmake libobs-dev qt6-base-dev qt6-webengine-dev \
    libqt6websockets6-dev libcurl4-openssl-dev
```

#### Fedora
```sh
sudo dnf install gcc-c++ cmake obs-studio-devel qt6-qtbase-devel qt6-qtwebengine-devel \
    qt6-qtwebsockets-devel libcurl-devel
```

---

## Build & Installation

### Option 1: Makefile (Recommended)

```sh
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
make -j$(nproc)
make install
```

### Option 2: CMake

```sh
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cmake --install build
```

### Installed Locations
- Binary: `~/.config/obs-studio/plugins/obs-streamelements/bin/64bit/obs-streamelements.so`
- Assets & Locales: `~/.config/obs-studio/plugins/obs-streamelements/data/`

---

## Configuration & Usage

1. Launch OBS Studio.
2. Navigate to **StreamElements** > **Login** in the top menu.
3. Choose your connection method:
   - Click your platform under **Browser Login** and approve the session in your browser.
   - Or paste your JWT Token from [streamelements.com/dashboard/account/channels](https://streamelements.com/dashboard/account/channels) under **Quick Connect**.
4. Docks will automatically load and mount your authenticated feed, chat, and media request widgets.

---

## Uninstallation

To remove the plugin and restore OBS Studio to its default state:

```sh
rm -rf ~/.config/obs-studio/plugins/obs-streamelements
```

---

## License

This project is licensed under the [GNU General Public License v2.0](LICENSE).
StreamElements and SE.Live are trademarks of StreamElements Ltd. This project is an independent community implementation.
