CC := gcc
MAJOR := 1
MINOR := 0
NAME := xdisasm
BUILD_DIR ?= ../build
VERSION := $(MAJOR).$(MINOR)
BINUTILS = binutils
ZSTD = zstd
ZSTD_LIB = $(ZSTD)/lib/libzstd.a
BFD = $(BINUTILS)/bfd/
OPCODES = $(BINUTILS)/opcodes/
IBERTY = $(BINUTILS)/libiberty/
SFRAME = $(BINUTILS)/libsframe/
BFD_LIB = $(BINUTILS)/bfd/.libs/libbfd.a
OPCODES_LIB = $(BINUTILS)/opcodes/.libs/libopcodes.a
IBERTY_LIB = $(BINUTILS)/libiberty/libiberty.a
SFRAME_LIB = $(BINUTILS)/libsframe/.libs/libsframe.a
TARGETS = "aarch64-linux-gnu arm-linux-gnu mips-linux-gnu powerpc-linux-gnu armel-linux-gnu mipsel-linux-gnu powerpc64-linux-gnu x86_64-linux-gnu mips64-linux-gnu mips64el-linux-gnu i386-linux-gnu x86_64-pe arm-pe arm-pex x86_64-darwin i386-darwin riscv64-unknown-linux-gnu riscv32-unknown-linux-gnu sh4-linux-gnu sparc*-*-* sparc-linux-gnu aarch64-apple-darwin"
CFLAGS := -I$(BFD) -I$(BINUTILS)/include -I$(OPCODES) -fPIC -O3 -Wall -Werror
UNAME := $(shell uname)

# Set the appropriate flag based on the host OS
ifeq ($(UNAME), Linux)
  PFLAGS := -Wl,-soname,lib$(NAME).so
  STRIP_FLAGS = -s
else ifeq ($(UNAME), Darwin)
  PFLAGS := -Wl,-install_name,lib$(NAME).so
  STRIP_FLAGS = -x
endif
 
lib: binutils-libs lib$(NAME).so

binutils-libs:
	test -e $(SFRAME_LIB) || (cd $(SFRAME) && ./configure && make -j 12 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(BFD_LIB) || (cd $(BFD) && ./configure --enable-targets=$(TARGETS) && make -j 12 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(OPCODES_LIB) || (cd $(OPCODES) && ./configure --enable-targets=$(TARGETS) && make -j 12 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(IBERTY_LIB) || (cd $(IBERTY) && ./configure && make -j 12 LDFLAGS="-all-static" CFLAGS="-fPIC")
	test -e $(ZSTD_LIB) || (cd $(ZSTD) && make -j 12)

lib$(NAME).so: $(NAME).o
	$(CC) -shared $(PFLAGS) $^ -o $(BUILD_DIR)/lib/lib$(NAME).so $(OPCODES_LIB) $(BFD_LIB) $(IBERTY_LIB) $(SFRAME_LIB) $(ZSTD_LIB) -lz -ldl
	strip $(STRIP_FLAGS) $(BUILD_DIR)/lib/lib$(NAME).so

clean:
	$(RM) *.o *.so*
	$(RM) $(BUILD_DIR)/lib/*.so
	cd $(BFD) && make distclean
	cd $(OPCODES) && make distclean
	cd $(IBERTY) && make distclean
	cd $(SFRAME) && make distclean
	cd $(ZSTD) && make clean
