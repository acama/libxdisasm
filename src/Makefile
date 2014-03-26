CFLAGS := -fPIC -O3 -g -Wall -Werror
CC := gcc
MAJOR := 1
MINOR := 0
NAME := xdisasm
BUILD_DIR ?= ../build
VERSION := $(MAJOR).$(MINOR)
 
lib: lib$(NAME).so

withstatic: lib$(NAME)_withstatic.so

lib$(NAME).so: $(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $(BUILD_DIR)/lib/$@ -lbfd -lopcodes
 
lib$(NAME)_withstatic.so: $(NAME).o
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $(BUILD_DIR)/lib/lib$(NAME).so static/libopcodes.a static/libbfd.a static/libiberty.a -lz

clean:
	$(RM) *.o *.so*
	$(RM) $(BUILD_DIR)/lib/*.so
