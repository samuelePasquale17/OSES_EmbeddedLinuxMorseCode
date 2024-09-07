  
# OSES - Morse Project
- [Morse Application](#titolo-1) 
	- [Options](#titolo-2) 
	- [Some lines of code](#titolo-3)
	- [Some examples](#titolo-4) 
- [Authors](#titolo-5)
- [Sources and Credits](#titolo-6)

<a name="titolo-1"></a>
## Morse Application
Using the Morse application is possible to start the conversion from human language to morse of a 63-character input string. The source string can be read either from the terminal or from a file given its path. Once the command is run, the conversion will start automatically and the result will be shown by the most right led (LD0) on the PYNQ-Z2 board. It is possible to listen it through a speaker as well.

The speaker should be connected properly to the board (see README file under hardware folder for more details).
<a name="titolo-2"></a>
### Options 

| Options available | Description |
|------|------|
|-h <br/> --help|Displays help message with all the information that are needed to run properly the application |
|-f <br/> --file|Through this flag is possible to set that the source string must be read from a text file. Specify the file’s path as input of the Morse application is needed|
|-s <br/> --string|Through this flag is possible to set that the source string is sent directly from the command line as input of the Morse application |
|-b <br/> --back|Through this command is possible to enable the application to print back the source string. It can be used as confirmation that the string have been read and sent correctly|
|-e <br/> --enable|Through this command is possible to start the conversion of the string that have been set up before. It is also possible to combine this option with all the other ones, in order to play the conversion immediately|

<a name="titolo-3"></a>
### Some lines of code
```c
file_desc = open(DEVICE_FILE_NAME, O_RDWR | O_SYNC);
```
This line of code opens the device file associated with the morse device driver and sets some flags:
1. **O_RDWR**, it opens the file in read/write mode.
2. **O_SYNC**, I/O operations will be synchronous. The program will wait for the I/O operation to be completed before continuing with further execution. This ensures that data is physically written to the storage device before the program proceeds.
```c
result = write(file_desc, (void*) str, lenMessage(str, MAX_LEN+1));
```
With this line of code is possible to send the string containing the message to the device driver which interfaces with the custom hardware programmed on the FPGA.
```c
result = ioctl(file_desc, ENABLE_CONVERSION_VALUE, NULL);
```
This line of code allows to send a command to the device driver. In particular this command is needed to start the conversion of the string that have been sent to the driver before. The first parameter il the file descriptor of the device file, while the second one is the command that enables the conversion. The last parameter is useless so it is simply setled to NULL.

<a name="titolo-4"></a>
### Some examples
1. Convert text to morse from file
>morse --file --enable ./home/test_file.txt

2. Convert text to morse from command line
>morse --string --enable "*your string here*"

3. Convert text to morse from file and print it back
>morse --file --back --enable ./home/test_file.txt

4. Convert text to morse from command line and print it back
>morse --string --enable --back "*your string here*"

5. Write text from command line and print it back
>morse --string --back "*your string here*"

6. Start message conversion written before
>morse --enable

<a name="titolo-5"></a>
## Authors
- Samuele Pasquale
- Davide Lezzoche
- Mansour Sohrabian
- Manuel Riso

<a name="titolo-6"></a>
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
