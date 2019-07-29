CC := gcc
MAJOR := 1
MINOR := 0
NAME := xdisasm
BUILD_DIR ?= ../build
VERSION := $(MAJOR).$(MINOR)
BINUTILS = binutils
BFD = $(BINUTILS)/bfd/
OPCODES = $(BINUTILS)/opcodes/
IBERTY = $(BINUTILS)/libiberty/
BFD_LIB = $(BINUTILS)/bfd/libbfd.a
OPCODES_LIB = $(BINUTILS)/opcodes/libopcodes.a
IBERTY_LIB = $(BINUTILS)/libiberty/libiberty.a
TARGETS = "aarch64-linux-gnu arm-linux-gnu mips-linux-gnu powerpc-linux-gnu armel-linux-gnu mipsel-linux-gnu powerpc64-linux-gnu x86_64-linux-gnu mips64-linux-gnu mips64el-linux-gnu i386-linux-gnu x86_64-pe arm-pe arm-pex x86_64-darwin i386-darwin riscv64-unknown-linux-gnu riscv32-unknown-linux-gnu sh4-linux-gnu sparc*-*-* sparc-linux-gnu"
CFLAGS := -I$(BFD) -I$(BINUTILS)/include -fPIC -O3 -Wall -Werror
 
lib: binutils-libs lib$(NAME).so

binutils-libs:
	test -e $(BFD_LIB) || (cd $(BFD) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(OPCODES_LIB) || (cd $(OPCODES) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(IBERTY_LIB) || (cd $(IBERTY) && ./configure && make -j 4 LDFLAGS="-all-static" CFLAGS="-fPIC")

lib$(NAME).so: $(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $(BUILD_DIR)/lib/lib$(NAME).so $(OPCODES_LIB) $(BFD_LIB) $(IBERTY_LIB) -lz -ldl
	strip -s $(BUILD_DIR)/lib/lib$(NAME).so

clean:
	$(RM) *.o *.so*
	$(RM) $(BUILD_DIR)/lib/*.so
	cd $(BFD) && make distclean
	cd $(OPCODES) && make distclean
	cd $(IBERTY) && make distclean
