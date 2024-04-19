#!/bin/bash

embedded_command_shell.sh

make rbf

make dtb

echo "Remember to use mount /dev/mmcblk0p1 /mnt before using scp"