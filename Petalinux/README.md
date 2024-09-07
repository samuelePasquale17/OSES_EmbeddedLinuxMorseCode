# OSES - Morse Project
- [Petalinux Workflow](#titolo-1) 
	- [Petalinux project workflow](#titolo-2) 
	- [Petalinux add Linux Kernel Module](#titolo-3)
	- [Petalinux add custom Application](#titolo-4) 
	- [Petalinux add Script executable on boot](#titolo-5)
	- [Petalinux add/edit users](#titolo-6)
- [Authors](#titolo-7)
- [Sources and Credits](#titolo-8)

<a name="titolo-1"></a>
## Petalinux Workflow
<a name="titolo-2"></a>
### Petalinux project workflow
First off all, every time that you work with Petalinux, you must set up the working environment, otherwise every time that you’ll type a Petalinux command an error, such as Petalunux command not found, will occurs.

For Bash, as a user login shell, run this command when you are in your project directory
>source &lt;path-to-installed-PetaLinux&gt;/settings.sh

Verify that the working environment has been set
>echo $PETALINUX

After the generation of the .xsa file (from Vivado) you need to create a project.

In this case you have not to create a project from a BSP but from a Hardware Platform. To create an empty project, run the command
>petalinux-create --type project --template &lt;PLATFORM&gt; --name &lt;PROJECT_NAME&gt;

Go into the directory of your PetaLinux project
>cd &lt;plnx-proj-root&gt;

Import the hardware description with petalinux-config command
>petalinux-config --get-hw-description &lt;PATH-TO-XSA-FILE&gt;

Now, if needed, you can go for some configurations of the file system, the kernel and the bootloader through the petalinux menu
>petalinux-config -c rootfs
>petalinux-config -c kernel
>petalinux-config -c u-boot

To build a PetaLinux System Image go into the directory of your PetaLinux project and run the following command to build the system image
>petalinux-build

When the build finishes, the generated images are stored in
>&lt;plnx-proj-root&gt;/images/linux

Now, in order to Package and Boot the Image for Zynq 7000 Devices, we need to run the following command
>petalinux-package --boot --format BIN --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/system.bit --u-boot images/linux/u-boot.elf --output BOOT.BIN

A boot image usually contains a first stage boot loader image, FPGA bitstream and U-Boot.

Once this is done you can move on to prepare your SD card using for example GParted software. Create 2 partitions called respectively BOOT (FAT16 partition type) and RootFS (EXT4 partition type). The first one must be at least 500 MB in size, while the size of the second one is the remaining space. Once the SD card is ready, go to home directory and from here copy and paste the files needed for booting Embedded Linux on the board.
>cp &lt;plnx-proj-root&gt;/images/linux/BOOT.BIN &lt;SDCARD-path&gt;/BOOT/
>
>cp &lt;plnx-proj-root&gt;/images/linux/image.ub &lt;SDCARD-path&gt;/BOOT/
>
>cp &lt;plnx-proj-root&gt;/images/linux/boot.scr &lt;SDCARD-path&gt;/BOOT/
>
>sudo tar xvf &lt;plnx-proj-root&gt;/images/linux/rootfs.tar.gz -C &lt;SDCARD-path&gt;/RootFS/

  

The last command copies and extracts the compressed folder that contains the entire root file system.

Now we are ready to boot the image on the PYNQ-Z2 board through the SD card.

  
<a name="titolo-3"></a>
### Petalinux add Linux Kernel Module
Under the PetaLinux project directory, use the command below to create your module
>petalinux-create --type modules --name mymodule --enable

Petalinux creates the module under the following directory
>/&lt;plnx-proj-root&gt;/project-spec/meta-user/recipes-modules/

The default driver creation includes a Makefile, C-file, and README files.
1. Change the C-file (driver file) and the Makefile in the proper way for your driver
2. Copy your .c and .h files into this directory
3. Open the .bb recipe and add .h file entry in SRC_URI

Now run the command
>petalinux-build

After successful compilation the .ko file is created in the following location
>&lt;petalinux-build_directory&gt;/build/tmp/sysroots-components/zc702_zynq7/blink/lib/modules/5.4.0-xilinx-v2021.1/extra/drivername.ko

<a name="titolo-4"></a>
### Petalinux add custom Application
Under the PetaLinux project directory, use the command below to create your custom application
>petalinux-create --type apps --template &lt;TYPE&gt; --name &lt;user-application-name&gt; --enable

The new application sources can be found in the
>&lt;plnx-proj-root&gt;/project-spec/meta-user/recipes-apps/myapp directory.

In this directory you can easily import any existing application code by copying it into this directory, and editing the automatically generated Makefile.

Before building the application, it might be needed to enable the application from PetaLinux menuconfig by running
> petalinux-config -c rootfs

From the menu go to user packages and select myapp to enable it.
To install the application to the target filesystem host copy and update the bootable images run
>petalinux-build -c morse
>petalinux-build -c rootfs
>petalinux-build -x package

Unless you have changed the location of the user application through its Makefile, the user application is placed into the /usr/bin directory into the file system.

<a name="titolo-5"></a>
### Petalinux add Script executable on boot
Under the PetaLinux project directory, use the command below to create your custom script
> petalinux-create -t apps --template install -n bootscript --enable

The new script sources can be found in the
> &lt;plnx-proj-root&gt;/project-spec/meta-user/recipes-apps/myscript directory.

In this directory you can easily import any existing script by copying it into this directory.

In order to run this script automatically during the booting up of the system you need to set the .bb file as shown below

  ```c
  SRC_URI = "file://bootscript"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "bootscript"
INITSCRIPT_PARAMS = " start 99 S ."

do_install() {
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${S}/bootscript ${D}${sysconfdir}/init.d/bootscript
}

FILES_${PN} += "${sysconfdir}/*"
  ```
The commands written within the do_install() function create the necessary directory structure for system initialization scripts and install a boot script file into the appropriate directory with executable permissions.
The script is executed during the booting up because it is stored into the ``` ./init.d ``` folder.

This directory is where various scripts responsible for starting, stopping, and managing system services are stored. During boot, the system looks into this directory for scripts that need to be executed as part of the initialization process.
  

To install the script to the target filesystem run

> petalinux-build -c bootscript -x do_install -f

> petalinux-build -c rootfs

> petalinux-build

with ```-x do_install``` is possible to indicate to execute only the do_install() function specified in the .bb file.

<a name="titolo-6"></a>
### Petalinux add/edit users
In petalinux configuration menu is possible to add extra users to the PetaLinux system.

First of all run the command
>petalinux-config -c rootfs

Now it is possible to add extra users going in PetaLinux Rootfs Settings > Add extra users.
To add extra users to the PetaLinux system, provide the user ID and password separated by `:`.
It is also possible to add multiple users, separate sets of user IDs and passwords using `;`.
For each user it is also possible to set an empty password simply omitting it.

Once all users have been created it possible to procede defining with are allowed to run the `sudo` command.
In the same page is possible to add users to sudo users giving only the user ID.

<a name="titolo-7"></a>
## Authors
- Samuele Pasquale
- Davide Lezzoche
- Mansour Sohrabian
- Manuel Riso

<a name="titolo-8"></a>
## Sources and Credits
Sources and credits
- [AMD Xilinx PYNQ-Z2](https://www.xilinx.com/support/university/xup-boards/XUPPYNQ-Z2.html)
- [AMD Petalinux guide v2023.2](https://docs.xilinx.com/r/en-US/ug1144-petalinux-tools-reference-guide/Introduction)
- [AMD Xilinx Vivado](https://www.xilinx.com/products/design-tools/vivado.html)
- [Morse Code](https://it.wikipedia.org/wiki/Codice_Morse)
- [PYNQ-Z2 constraints file](https://github.com/Xilinx/PYNQ/blob/master/boards/Pynq-Z2/base/vivado/constraints/base.xdc)
- [PYQN-Z2 schematic file](https://dpoauwgwqsy2x.cloudfront.net/Download/TUL_PYNQ_Schematic_R12.pdf)
- [AMD Xilinx Embedded design](https://xilinx.github.io/Embedded-Design-Tutorials/docs/2022.2/build/html/index.html)
- [Device driver Linux](https://lwn.net/Kernel/LDD3/)
- [Yocto project](https://docs.yoctoproject.org)
