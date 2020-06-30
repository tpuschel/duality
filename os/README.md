# Duality OS

Duality OS is a small, experimental OS with the eventual goal of
bootstrapping just enough to launch a Duality REPL.

Requires support for UEFI, and currently only runs on x64.

## Building

Duality OS is designed to be build with clang and lld-link.

To create the PE32+ executable as required by UEFI, run:

`clang -target x86_64-unknown-windows -ffreestanding -fuse-ld=lld-link -nostdlib -O2 -mno-red-zone -Wl,/subsystem:efi_application,/entry:boot boot.c -o BOOTx64.EFI`

## Creating an image

To be able to boot Duality OS, we need to create an image with a FAT32 filesystem and /EFI/BOOT/BOOTx64.EFI as the folder structure.

On macOS, creating an image can be done like this:

`mkdir -p image/EFI/BOOT`

`cp BOOTx64.EFI image/EFI/BOOT/.`

`hdiutil create -srcfolder image -fs FAT32 -volname "Duality OS" -format RdWr duality-os`

If `duality-os.img` already exists, append `-ov` to the above hdiutil command.

## Booting

The image can run virtualized with something like [QEMU](https://www.qemu.org), or be copied to a USB drive to run on real hardware.

For QEMU, a UEFI firmware like [OVMF](https://github.com/tianocore/tianocore.github.io/wiki/OVMF) is needed.

To start QEMU, run:

`qemu-system-x86_64 --bios <OVMF file> duality-os.img`
