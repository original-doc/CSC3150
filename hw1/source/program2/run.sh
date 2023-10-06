#!/bin/bash

# 4. Remove the kernel module
sudo rmmod program2

# 1. Clean the project
make clean

# 2. Build the project
make

# 3. Load the kernel module
sudo insmod program2.ko

# 5. Display kernel messages
dmesg
