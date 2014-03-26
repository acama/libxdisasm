/*  xdisasm.h -- xdisasm header file
    Copyright (C) 2014  Amat I. Cama                                         
                                                                             
    This file is part of xdisasm.                                            
                                                                             
    Xdisasm is free software: you can redistribute it and/or modify          
    it under the terms of the GNU General Public License as published by     
    the Free Software Foundation, either version 3 of the License, or        
    (at your option) any later version.                                      
                                                                             
    Xdisasm is distributed in the hope that it will be useful,               
    but WITHOUT ANY WARRANTY; without even the implied warranty of           
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
    GNU General Public License for more details.                             
                                                                             
    You should have received a copy of the GNU General Public License        
    along with this program.  If not, see <http://www.gnu.org/licenses/>.    
                                                                             
*/                                                                          

#ifndef XDISASM_H
#define XDISASM_H

#define ARCH_arm 1
#define ARCH_mips 2
#define ARCH_powerpc 3
#define ARCH_x86 4

// instruction structure
typedef struct insn_t{
    unsigned int vma;
    size_t instr_size;
    char * opcodes;
    char * decoded_instrs;
}insn_t;

// list container for instruction
typedef struct insn_list{
    insn_t * instr;
    struct insn_list * next;
}insn_list;

// insn_t *, insn_list ** -> void
// Append instruction to list
void append_instr(insn_t * i, insn_list **ilist);

// char *, char *, unsigned int -> void
// Copy the bytes from src to dest, inverted way
void * disassemble(unsigned int vma, char * rawbuf, size_t buflen, int arch, int bits, int endian);

// insn_list ** -> size_t
// Count the number of instructions in the list
size_t instr_num(insn_list **ilist);

// insn_t * -> void
// Print instruction in a formatted way
void print_instr(insn_t * ins);

// insn_list ** -> void
// Print all the instructions in a formatted way
void print_all_instrs(insn_list **ilist);

#endif
