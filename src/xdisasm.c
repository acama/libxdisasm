/*
    xdisasm.c -- disassembling library  
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


#include "package.h"
#include <stdlib.h>
#include <stdarg.h>
#include <bfd.h>     
#include <dis-asm.h>
#include <malloc.h>
#include <string.h>
#include "xdisasm.h"

#define MAX_INS_STRSIZE 2048

char curr_insn_str[MAX_INS_STRSIZE] = {0};
char * currptr = curr_insn_str;

// insn_t * -> void
// Print instruction in a formatted way
void print_instr(insn_t * ins){
    size_t i, l;
    char * tmpbuf = NULL, * ptr;

    if(ins == NULL){
        return;
    }

    printf("%08X  ", ins->vma);
    l = ins->instr_size;
   
    tmpbuf = (char *) malloc((l * 2) + 1);

    if(!tmpbuf){
        perror("malloc");
        return;
    }

    ptr = tmpbuf;
    for(i = 0; i < l; i++){
        sprintf(ptr, "%02X", (unsigned char)(ins->opcodes[i]));
        ptr += 2;
    }

    if(l < 15)
        printf("%-18s", tmpbuf);
    else
        printf("%-36s", tmpbuf);
    printf("%s\n", ins->decoded_instrs);
    free(tmpbuf);
}

// insn_list ** -> void
// Print all the instructions in a formatted way
void print_all_instrs(insn_list **ilist){
    insn_list * l = *ilist;

    while(l != NULL){
        print_instr(l->instr);
        l = l->next;
    }
}

// insn_list ** -> size_t
// Count the number of instructions in the list
size_t instr_num(insn_list **ilist){
    insn_list * l = *ilist;
    size_t len = 0;

    while(l != NULL){
        len++;
        l = l->next;
    }

    return len;
}

// insn_t *, insn_list ** -> void
// Initialize list
void init_list(insn_t *i, insn_list **ilist){
    insn_list * l = (insn_list *) malloc(sizeof(insn_list));
    l->instr = i;
    l->next = NULL;
    *ilist = l;
}

// insn_t *, insn_list ** -> void
// Append instruction to list
void append_instr(insn_t * i, insn_list **ilist){
    insn_list * tmp = NULL;
    insn_list * c = *ilist; 

    if(c == NULL){
        init_list(i, ilist);
        return;
    }

    if(c != NULL){
        while(c->next != NULL){
            c = c->next;
        }
    }

    tmp = (insn_list *) malloc(sizeof(insn_list));
    tmp->instr = i;
    tmp->next = NULL;

    c->next = tmp; 
}

// char *, char *, unsigned int -> void
// Copy the bytes from src to dest
void copy_bytes_x86(char * dest, char * src, unsigned int siz){
    int i = 0;

    for(; i < siz; i++){
        dest[i] = src[i];
    }

}

// char *, char *, unsigned int -> void
// Copy the bytes from src to dest, inverted way
void copy_bytes(char * dest, char * src, unsigned int siz){
    int i = 0, j = 0;

    for(i = siz - 1; i >= 0; i--, j++){
        dest[j] = src[i];
    }
}

// bfd_vma, struct disassemble_info * -> void
// Formatter for address in memory referencing instructions
void override_print_address(bfd_vma addr, struct disassemble_info *info){
    sprintf(currptr, "0x%x", (unsigned int) addr);
}

// void*, char * -> int
// Callback function for instruction printing function
int my_fprintf(void* stream, const char * format, ...){
    va_list arg;

    va_start(arg, format);
    vsnprintf(currptr, MAX_INS_STRSIZE - (currptr - curr_insn_str),format, arg);
    currptr = curr_insn_str + strlen(curr_insn_str);
    return 0;
}

// uint, char*, size_t, int, int -> insn_list *
// Disassemble the raw buf for the given parameters
void * disassemble(unsigned int vma, char * rawbuf, size_t buflen, int arch, int bits, int endian){
    insn_list * ilist = NULL;
    bfd_byte* buf = NULL;
    disassemble_info* dis = NULL;
    unsigned int count = 0;
    size_t pos = 0, length = 0, max_pos = 0;
    disassembler_ftype disas; 

    dis = (struct disassemble_info*) calloc(1, sizeof(disassemble_info));
    init_disassemble_info (dis, stdout, my_fprintf);
    buf = (bfd_byte*) rawbuf;

    dis->buffer_vma = vma;
    dis->buffer = buf;
    dis->buffer_length = buflen;
    dis->print_address_func = override_print_address;

    length = dis->buffer_length;
    max_pos = dis->buffer_vma + length;
    pos = vma;

    switch(arch){
        case ARCH_arm:
            if(endian) disas = print_insn_big_arm;
            else disas = print_insn_little_arm; 
            break;

        case ARCH_mips: // TODO: add mips64 support
            if(endian) disas = print_insn_big_mips;
            else disas = print_insn_little_mips; 
            break;

        case ARCH_powerpc: // TODO: add powerpc64 support
            if(endian) disas = print_insn_big_powerpc;
            else disas = print_insn_little_powerpc;
            dis->arch = bfd_arch_powerpc;       // ppc cares about this
            disassemble_init_for_target(dis);   // otherwise segfault
            break;

        case ARCH_x86:
            if (bits == 16) dis->mach = bfd_mach_i386_i8086;
            else if (bits == 64) dis->mach = !(bfd_mach_i386_i8086 | bfd_mach_i386_i386);
            else dis->mach = bfd_mach_i386_i386;
            disas = print_insn_i386_intel;
            break;

        default:
            fprintf(stderr, "libxdisasm: Invalid architecture\n");
            return NULL;
    } 
   
    while(pos < max_pos)
      {
          insn_t * curri = (insn_t *) malloc(sizeof(insn_t));
          if(!curri){
              perror("malloc");
              return NULL;
          }

          curri->vma = pos;
          unsigned int size = disas((bfd_vma) pos, dis);
          curri->instr_size = size;

          char * opcodes = (char *) malloc(size);
          if(!opcodes){
              perror("malloc");
              return NULL;
          }
        
          if(arch == ARCH_x86){
              copy_bytes_x86(opcodes, rawbuf + (pos - vma), size);
          }else{
              copy_bytes(opcodes, rawbuf + (pos - vma), size);
          }
          curri->opcodes = opcodes;

          size_t istrlen = strlen(curr_insn_str);
          char * decoded_instrs = (char *) malloc(istrlen + 1);
          if(!decoded_instrs){
              perror("malloc");
              return NULL;
          }
          memcpy(decoded_instrs, curr_insn_str, istrlen + 1);
          curri->decoded_instrs = decoded_instrs;


          memset(curr_insn_str, 0, sizeof(curr_insn_str));
          currptr = curr_insn_str;
          pos += size;
          count++;
          append_instr(curri, &ilist);
      }

    free(dis);
    return ilist;
}
