#!/bin/bash
read -p "Enter Computer Number: " ComputerNumber

scp "bk2746@micro${ComputerNumber}.ee.columbia.edu:~/Projects/EmbeddedLab/Project_hw/output_files/soc_system.rbf" .

scp "bk2746@micro${ComputerNumber}.ee.columbia.edu:~/Projects/EmbeddedLab/Project_hw/soc_system.dtb" .

echo "Transfer initiated for micro${ComputerNumber}."