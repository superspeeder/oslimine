#!/bin/bash

dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd
PATH=$PATH:/usr/sbin:/sbin sgdisk image.hdd -n 1:2048 -t 1:ef00 -m 1

git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1

make -C limine

./limine/limine bios-install image.hdd

mformat -i image.hdd@@1M
mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine

mcopy -i image.hdd@@1M bin/myos ::/boot
mcopy -i image.hdd@@1M limine.conf limine/limine-bios.sys ::/boot/limine
mcopy -i image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
mcopy -i image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

