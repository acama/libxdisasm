libxdisasm
==========

libxdisasm is a simple and small library that allows disassembly of raw bytes. It currently supports x86, x86_64, arm, ppc and mips. It is meant to provide a quick way to disassemble a raw chunk of bytes. An example of a tool that uses libxdisasm is github.com/acama.

Build Instructions
------------------
### Quick way
If you are on a 64-bit Linux machine and don't feel like building binutils from source, I have included the static libraries needed for the tool to compile. You can build the tool with:
```
make withstatic
```
### Longer way
First Build binutils with the appropriate flags. You can get the source from http://ftp.gnu.org/gnu/binutils/. By default binutils will install the shared libraries in /usr/local/lib. If this is not in your library path you might run into some issues. Run the following commands in the directory where you extracted the binutils archive.
```
./configure --enable-targets=all --enable-shared
make
sudo make install
```
Then you can build xdisasm. From the top level directory, run the following command:
```
make
```

Usage:
------
As stated above, the library is pretty small and it basically is defined by one function:
```
insn_list * disassemble(unsigned int vma, char * rawbuf, size_t buflen, int arch, int bits, int endian)
```
This function returns a linked list of insn_t types which are basically containeres for decoded instructions. The arguments to the disassemble() function are described below:
* __vma__ - This is the address where the raw buffer will be assumed to be loaded in memory
* __rawbuf__ - This buffer points to the raw bytes that are supposed to be disassembled
* __buflen__ - This is the length of the data being disassembled
* __arch__ - One of ARCH__{arm ,x86, powerpc, mips}
* __bits__ - 64-bit, 32-bit or 16-bit
