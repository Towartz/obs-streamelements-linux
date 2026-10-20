# StreamElements (SE.Live) Linux Plugin for OBS Studio

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)](https://www.qt.io/)
[![OBS Studio](https://img.shields.io/badge/OBS%20Studio-30%2B%20%2F%2031%2B%20%2F%2032%2B-red.svg)](https://obsproject.com/)
[![License: GPL-2.0](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](LICENSE)

A clean, native Linux port of the **StreamElements (SE.Live)** OBS Studio plugin. Built with modern **C++20**, **Qt 6**, and `libobs` to deliver full feature parity with the Windows version without Wine or emulation.

---

## ✨ Features

- **🔴 Top-Level StreamElements Menu**:
  - Inserted directly into OBS Studio's main menu bar with official StreamElements branding.
  - Direct links to *Setup Bot & Tipping*, *Browse Overlays*, *Support*, and *Account Management*.
- **📊 Native Browser Docks**:
  - **Activity Feed**: Real-time event notifications, tips, followers, subscribers, and cheer alerts.
  - **Live Chat**: Embedded chat dock for Twitch, YouTube, and Kick.
  - **Media Request**: Live song request queue and volume control.
- **⚡ Local API WebSocket Server (`ws://127.0.0.1:8092`)**:
  - Full JSON-RPC implementation compatible with SE.Live web widgets.
  - Emits real-time OBS state changes (`streamingStarted`, `streamingStopped`, `sceneChanged`, `recordingStarted`).
- **🎵 Audio Wrapper Source Filter**:
  - Registers `streamelements_transition_audio_wrapper_source` to handle seamless transition audio for StreamElements scene stinger transitions.
- **💾 Cloud Backup Manager**:
  - Full packaging and restoration of OBS scenes, collections, and profiles.
- **🔐 Multi-Method Authentication**:
  - **Browser Loopback Login (RFC 8252)**: Opens your default browser (Brave, Chrome, Firefox) to authenticate via Google/YouTube without triggering Google's *"This browser or app may not be secure"* embedded webview block.
  - **Quick Connect (JWT Token)**: Paste your StreamElements JWT Token for instant authentication in 5 seconds.
  - Persistent session storage in `~/.config/obs-studio/plugins/obs-streamelements/`.
- **🌍 50+ Languages Included**:
  - Complete official locale translations loaded from `data/locale/`.

---

## 🛠️ Prerequisites & Dependencies

### Arch Linux / Manjaro
```bash
sudo pacman -S base-devel cmake qt6-base qt6-webengine qt6-websockets curl obs-studio
```

### Ubuntu / Debian (22.04+ / 24.04+)
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-webengine-dev \
    libqt6websockets6-dev libcurl4-openssl-dev libobs-dev
```

### Fedora
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtwebengine-devel \
    qt6-qtwebsockets-devel libcurl-devel obs-studio-devel
```

---

## 🚀 Building & Installing

### Quick Install (Makefile)

Clone and compile directly:
```bash
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
make -j$(nproc)
make install
```

### Or using CMake

```bash
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
cmake --install .
```

The plugin binary and assets will be installed to:
- Binary: `~/.config/obs-studio/plugins/obs-streamelements/bin/64bit/obs-streamelements.so`
- Assets: `~/.config/obs-studio/plugins/obs-streamelements/data/`

---

## 🔑 Logging In

1. Open **OBS Studio**.
2. Click **StreamElements** → **Login** in the top menu bar.
3. Choose your preferred connection method:
   - **Browser Login**: Click your platform (e.g., **YouTube**). It opens your default browser where you are already signed in. Approve the connection, and OBS will connect automatically.
   - **Quick Connect (JWT Token)**: Click *Open StreamElements Account Settings*, copy your **JWT Token** under *Show Secrets*, paste it into OBS, and click *Connect Account*.

---

## 🗑️ Uninstallation

To completely remove the plugin and restore OBS Studio to vanilla state:
```bash
rm -rf ~/.config/obs-studio/plugins/obs-streamelements
```

---

## ⚖️ License & Disclaimer

- Distributed under the **GNU General Public License v2.0** ([GPL-2.0](LICENSE)).
- This is an independent, community-driven open-source project and is not officially affiliated with or endorsed by StreamElements.
