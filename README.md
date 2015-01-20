## Real-time kernel for Raspberry Pi

This repository holds a fork of Raspberry Pi Linux kernel patched with rt-patch and configured as a fully preemtible kernel. Instructions on compiling, setting up and testing the kernel are provided below.

### Cross-compiling the rt kernel on Linux

Download Raspberry Pi tools:
```
git clone https://github.com/raspberrypi/tools.git
```

Download kernel sources:
```
git clone https://github.com/emlid/linux.git
```

Export the following variables to specify cross-compilation options:
```
export ARCH=arm 
export CROSS_COMPILE=~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
```

Load the rt config:
```
make bcmrpi_rt_defconfig
```

Optional: make changes to the config
```
make menuconfig
```

Compile the kernel:
```
make -j5
```

Compile modules:
```
make -j5 modules
```

Copy modules, will result in "lib" folder with modules and firmware:
```
mkdir kernel-rt
INSTALL_MOD_PATH=kernel-rt make modules_install
```

### Setting up an SD card with new rt kernel

Copy arch/arm/boot/Image to /boot/kernel-rt.img on SD card.

Copy (merge if necessary) kernel-rt/lib to / on SD card.

Add the following to /boot/config.txt:
```
kernel=kernel-rt.img
```

Add the following to /boot/cmdline.txt:
```
sdhci_bcm2708.enable_llm=0
```

Now you can insert an SD card into your Raspberry Pi and check if it boots.
If something is wrong it is recommended to observe serial console.

### Testing real-time capabilites using cyclictest utility

Installing cyclictest utility on Raspberry Pi:
```
git clone git://git.kernel.org/pub/scm/linux/kernel/git/clrkwllms/rt-tests.git 
cd rt-tests
make all
cp ./cyclictest /usr/bin/
cd ~
```

Testing real-time:
```
sudo cyclictest -l1000000 -m -n -a0 -t1 -p99 -i400 -h400 -q
```
