# k24-default-bitstreams

This repo is for a snap application which connects with fpgad to load a bitstream to the fpga0 device.
The contained bitstream is a fan controller for the `K*24*` series of Xilinx boards.

If running on target device:

```shell
snapcraft
sudo snap install k24-default-bitstreams..._arm64.snap
sudo snap connect k24-default-bitstreams:fpgad-dbus fpgad:daemon-dbus
```

NB: the `fpgad:daemon-dbus` is external to this repo so may be subject to change.
Check [fpgad's snapcraft.yaml](https://github.com/canonical/fpgad/blob/main/snap/snapcraft.yaml) for changes if this
command fails.

## snapcraft.yaml explained

The `plugs` entry here allows the connection to be made between this snap and the fpgad daemon

```yaml
plugs:
  fgpad-dbus:
    interface: dbus
    bus: system
    name: com.canonical.fpgad
```

but it must also be added to the application:

```yaml
apps:
  k24-default-bitstreams:
    command: bin/k24-default-bitstreams
    daemon: oneshot
    plugs:
      - fpgad-dbus
```

here `daemon: oneshot` means "run once on startup and then it is finished".

The parts section describes how to form the snap package

```yaml

parts:
  version:
    plugin: nil
    source: .
    build-snaps:
      - jq
    override-pull: |
      craftctl default
      cargo_version=$(cargo metadata --no-deps --format-version 1 | jq -r .packages[0].version)
      craftctl set version="$cargo_version+git$(date +'%Y%m%d').$(git describe --always --exclude '*')"
  k24-default-bitstreams:
    build-packages: # the apt packages required at build time
      - pkg-config
      - libglib2.0-dev
    stage-packages: # the apt packages required at run time
      - libglib2.0-0
    plugin: cmake
    source: ./k24-default-bitstreams
    cmake-parameters: # cmake overrides
      - -DCMAKE_INSTALL_PREFIX=/ # Otherwise the DESTDIR is relative to $SNAPCRAFT_PART_INSTALL/usr/local/
bitstream-data:
  plugin: dump
  source: https://github.com/Xilinx/kria-base-firmware
  source-type: git
  override-build: |
    echo $(ls -a)
    mkdir -p $SNAPCRAFT_PART_INSTALL/data/k24-starter-kits
    cp k24_starter_kits/k24_starter_kits.bit $SNAPCRAFT_PART_INSTALL/data/k24-starter-kits/
    cp LICENSE-BINARIES $SNAPCRAFT_PART_INSTALL/data/k24-starter-kits/
```

Here `version` just runs a simple script to generate a unique version string, `k24-default-bitstreams` part defines how
to build the rust package which creates the `bin/k24-default-bitstreams` used in the app section and `bitstream-data`
clones a remote repository and makes the `k24_starter_kits.bit.bin` and `LICENSE-BINARIES` files available from the snap
root at `$SNAP/data`.
See [the snapcraft docs on package versioning](https://documentation.ubuntu.com/snapcraft/stable/how-to/crafting/configure-package-information/)
for more information.

# dependencies

This example has been written hoping to need the minimum number of dependencies, however pkgconfig is (one of the available methods) required for cmake to detect the location of installed packages and a single library is used for making the dbus calls. These are outlined below

## build dependencies

Because of the following lines in the `CMakeLists.txt`, `pkg-config` is required:
```
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-2.0)
```

On Ubuntu, install via
```
sudo apt update && sudo apt install pkg-config
```

## dbus library

For DBus interfacing, we have chosen to use
GLib 2.0: https://docs.gtk.org/glib/

On Ubuntu, install via

```shell
sudo apt update
sudo apt install libglib2.0-dev # for development
sudo apt install libglib2.0-0 # for usage
```

There are alternative dbus libraries for C++ available such as [sdbus-c++](https://github.com/Kistler-Group/sdbus-cpp)
or [dbus-cxx](https://dbus-cxx.github.io/), but the prior requires extra plugs (such as network-bind) in order to make a
connection, and the latter is not available as prebuilt binaries (not available oin the Ubuntu Archive).

## snapcraft.yaml dependencies

In order to provide the build dependencies and runtime dependencies inside the snap confinement, the following lines can be added to the snapcraft.yaml for the part which builds the binary.
```
...
parts:
...
  k24-default-bitstreams:
    build-packages:
      - pkg-config
      - libglib2.0-dev
    stage-packages:
      - libglib2.0-0
    ...
...
```

# Licenses

The source code here is distributed under the GPL-3.0-only licence provided in the repository root's `LICENSE` file.

The bitstream packaged in the snap is distributed under a binary only license, provided in the
`$SNAP/data/k24-starter-kits/LICENSE-BINARIES` file.
