# obs-streamelements-linux

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52?style=flat-square&logo=qt&logoColor=white)](https://qt.io)
[![OBS Studio](https://img.shields.io/badge/OBS%20Studio-30%2B-302E31?style=flat-square&logo=obsstudio&logoColor=white)](https://obsproject.com)
[![License: GPL-2.0](https://img.shields.io/badge/License-GPL--2.0-blue.svg?style=flat-square)](LICENSE)

StreamElements (SE.Live) plugin ported to Linux for OBS Studio.

It brings the official StreamElements docks, menus, and overlay integrations to Linux without Wine.

## Docks and features

- Activity Feed dock (tips, follows, subs, cheers)
- Chat dock (Twitch, YouTube, Kick)
- Media Request dock
- StreamElements top bar menu
- Audio transition filter
- Backup and restore for scene collections and profiles

## Install dependencies

**Arch Linux:**
```sh
sudo pacman -S base-devel cmake obs-studio qt6-base qt6-webengine qt6-websockets curl
```

**Ubuntu / Debian:**
```sh
sudo apt install build-essential cmake libobs-dev qt6-base-dev qt6-webengine-dev libqt6websockets6-dev libcurl4-openssl-dev
```

**Fedora:**
```sh
sudo dnf install gcc-c++ cmake obs-studio-devel qt6-qtbase-devel qt6-qtwebengine-devel qt6-qtwebsockets-devel libcurl-devel
```

## Build and install

```sh
git clone https://github.com/Towartz/obs-streamelements-linux.git
cd obs-streamelements-linux
make -j$(nproc)
make install
```

Files install to `~/.config/obs-studio/plugins/obs-streamelements/`.

## Logging in

1. Open OBS Studio.
2. Click **StreamElements** > **Login** in the top menu.
3. Choose your login method:
   - **Browser Login**: Click your platform (YouTube, Twitch, Kick). It opens your browser to sign in.
   - **Quick Connect**: Open [streamelements.com/dashboard/account/channels](https://streamelements.com/dashboard/account/channels), click Show Secrets, copy your JWT token, and paste it into OBS.

## Uninstall

```sh
rm -rf ~/.config/obs-studio/plugins/obs-streamelements
```

## License

GPL-2.0
