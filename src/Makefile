CFLAGS := -fPIC -O3 -g -Wall -Werror
CC := gcc
MAJOR := 1
MINOR := 0
NAME := xdisasm
BUILD_DIR ?= ../build
VERSION := $(MAJOR).$(MINOR)
BINUTILS = binutils-2.24
BFD = $(BINUTILS)/bfd/
OPCODES = $(BINUTILS)/opcodes/
IBERTY = $(BINUTILS)/libiberty/
TARGETS = "arm-linux-gnu mips-linux-gnu powerpc-linux-gnu armel-linux-gnu mipsel-linux-gnu powerpc64-linux-gnu x86_64-linux-gnu mips64-linux-gnu mips64el-linux-gnu i386-linux-gnu x86_64-pe arm-pe arm-pei x86_64-darwin i386-darwin"
 
lib: binutils lib$(NAME).so

binutils:
	test -d $(BINUTILS) || (wget http://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.gz && tar -zxf $(BINUTILS).tar.gz)
	cd $(BFD) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static" CFLAGS="-fPIC"
	cd $(OPCODES) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static" CFLAGS="-fPIC" 
	cd $(IBERTY) && ./configure && make LDFLAGS="-all-static" CFLAGS="-fPIC" 
	rm -rf $(BINUTILS).tar.gz

lib$(NAME).so: $(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $(BUILD_DIR)/lib/lib$(NAME).so $(OPCODES)/libopcodes.a $(BFD)/libbfd.a $(IBERTY)/libiberty.a -lz -ldl

clean:
	$(RM) *.o *.so*
	$(RM) $(BUILD_DIR)/lib/*.so
	$(RM) -rf $(BINUTILS)
	$(RM) -rf $(BINUTILS).tar.gz
