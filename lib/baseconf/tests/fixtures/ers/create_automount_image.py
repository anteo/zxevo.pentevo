#!/usr/bin/env python3
"""Create a FAT HDD with an ERS automount list."""

import sys
from pathlib import Path

from create_device_images import SECTOR_SIZE, fat32_image


def main():
    output = Path(sys.argv[1])
    output.mkdir(parents=True, exist_ok=True)
    mount_list = b"E:/DOCS/DISK.TRD C:\r\nE:/DOCS/MISSING.TRD D:\r\n"
    fat32_image(output / "hdd.img", 64 * 1024 * 1024 // SECTOR_SIZE, mount_list)


if __name__ == "__main__":
    main()
