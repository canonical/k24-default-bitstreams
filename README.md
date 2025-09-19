# k24-default-bitstreams

This repo is for a snap application which connects with fpgad to load a bitstream to the fpga0 device.
The contained bitstream is a fan controller for the `K*24*` series of Xilinx boards.

If running on target device:
```shell
snapcraft
sudo snap install k24-default-bitstreams..._arm64.snap
sudo snap connect k24-default-bitstreams:fpgad-dbus fpgad:daemon-dbus
```

NB: the `fpgad:daemon-dbus` is external to this repo so may be subject to change. Check [fpgad's snapcraft.yaml](https://github.com/canonical/fpgad/blob/main/snap/snapcraft.yaml) for changes if this command fails.

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
  k26-default-bitstreams:
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
[TODODODODODODODOD]
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
Here `version` just runs a simple script to generate a unique version string, `k24-default-bitstreams` part defines how to build the rust package which creates the `bin/k24-default-bitstreams` used in the app section and `bitstream-data` clones a remote repository and makes the `k24_starter_kits.bit.bin` and `LICENSE-BINARIES` files available from the snap root at `$SNAP/data`. See [the snapcraft docs on package versioning](https://documentation.ubuntu.com/snapcraft/stable/how-to/crafting/configure-package-information/) for more information.

# Licenses

The source code here is distributed under the GPL-3.0-only licence provided in the repository root's `LICENSE` file.

The bitstream packaged in the snap is distributed under a binary only license, provided in the `$SNAP/data/k24-starter-kits/LICENSE-BINARIES` file.
