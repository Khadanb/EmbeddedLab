#!/bin/bash
read -p "Enter Computer Number: " ComputerNumber

scp "bk2746@micro${ComputerNumber}.ee.columbia.edu:~/Projects/EmbeddedLab/Project_hw/output_files/soc_system.rbf" /mnt

scp "bk2746@micro${ComputerNumber}.ee.columbia.edu:~/Projects/EmbeddedLab/soc_system.dtb" /mnt

echo "Transfer initiated for micro${ComputerNumber}."
