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


#include "../include/package.h"
#include <stdlib.h>
#include <stdarg.h>
#include <bfd.h>     
#include <dis-asm.h>
#include <malloc.h>
#include <string.h>
#include "../include/xdisasm.h"

#define MAX_INS_STRSIZE 2048

#define COLADDR "\e[34m"
#define COLEND "\e[m"

char curr_insn_str[MAX_INS_STRSIZE] = {0};
char * currptr = curr_insn_str;
disassembler_ftype disas = NULL; // disassembler 
disassemble_info* dis = NULL;   // disassembly information structure
char * disas_options = NULL;
int xdisasm_no_color_g=0;


// insn_t -> void
// Free the memory
void free_instr(insn_t *i){
    
    if(!i){
        return;
    }
    
    free(i->decoded_instrs);
    free(i->opcodes);
    free(i);
}

// insn_list ** -> void
// Free the memory
// Dirty
void free_all_instrs(insn_list **ilist){
    insn_list * l, * f;

    if(!ilist){
        return;
    }
    l = *ilist;

    /* free the insn_t's */
    while(l != NULL){
        free_instr(l->instr);
        f = l;
        l = l->next;
        free(f);
    }
}

// insn_t * -> void
// Print instruction in a formatted way
void print_instr(insn_t * ins){
    size_t i, l;
    char * tmpbuf, * ptr;

    if(!ins)
        return;

    printf("%016llX  ", ins->vma);
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
    insn_list * l;
    size_t len = 0;

    if(!ilist)
        return 0;

    l = *ilist;

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

    if(!l){
        perror("malloc");
        return;
    }

    l->instr = i;
    l->next = NULL;
    *ilist = l;
}

// insn_t *, insn_list ** -> void
// Prepend instruction to list
void prepend_instr(insn_t * i, insn_list **ilist){
    insn_list * tmp, *c;

    if(!ilist)
        return;

    c = *ilist;

    if(!c){
        init_list(i, ilist);
        return;
    }


    tmp = (insn_list *) malloc(sizeof(insn_list));
    tmp->instr = i;
    tmp->next = *ilist;

    *ilist = tmp; 
}

// insn_t *, insn_list ** -> void
// Append instruction to list
void append_instr(insn_t * i, insn_list **ilist){
    insn_list * tmp, *c;

    if(!ilist)
        return;

    c = *ilist; 
    
    if(!c){
        init_list(i, ilist);
        return;
    }

    while(c->next != NULL){
        c = c->next;
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

    if(!dest || !src)
        return;

    for(; i < siz; i++){
        dest[i] = src[i];
    }

}

// char *, char *, unsigned int -> void
// Copy the bytes from src to dest, inverted way
void copy_bytes(char * dest, char * src, unsigned int siz){
    int i = 0, j = 0;

    if(!dest || !src)
        return;

    for(i = siz - 1; i >= 0; i--, j++){
        dest[j] = src[i];
    }
}

// bfd_vma, struct disassemble_info * -> void
// Formatter for address in memory referencing instructions
void override_print_address(bfd_vma addr, struct disassemble_info *info){
    sprintf(currptr, COLADDR "%p" COLEND, (void *) addr);
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

int init_dis_env(int arch, int bits, int endian){
   
    switch(arch){
        case ARCH_arm:
            if(endian) disas = print_insn_big_arm;
            else disas = print_insn_little_arm;
            if(bits == 16) disas_options = "force-thumb";
            else if (bits == 64) disas = print_insn_aarch64;
            else disas_options = "no-force-thumb";
            break;
        case ARCH_mips: 
            if(endian) disas = print_insn_big_mips;
            else disas = print_insn_little_mips; 
            if(bits == 64) dis->mach = bfd_mach_mipsisa64;
            break;

        case ARCH_powerpc: 
            if(endian) disas = print_insn_big_powerpc;
            else disas = print_insn_little_powerpc;
            if (bits == 64) disas_options = "64";
            dis->arch = bfd_arch_powerpc;       // ppc cares about this
            disassemble_init_for_target(dis);   // otherwise segfault
            break;

        case ARCH_x86:
            if (bits == 16) dis->mach = bfd_mach_i386_i8086;
            else if (bits == 64) dis->mach = !(bfd_mach_i386_i8086 | bfd_mach_i386_i386);
            else dis->mach = bfd_mach_i386_i386;
            disas = print_insn_i386_intel;
            break;

        case ARCH_riscv:
            if (bits == 64) dis->mach = bfd_mach_riscv64;
            else dis->mach = bfd_mach_riscv32;
            disas = print_insn_riscv;
            break;

        case ARCH_sparc: // TODO: use the different mach's based on the mach from the ELF
            dis->mach = bits;
            if(endian)
                dis->endian = BFD_ENDIAN_BIG;
            else
                dis->endian = BFD_ENDIAN_LITTLE;
            disas = print_insn_sparc;
            break;

        case ARCH_sh4:
            dis->mach = bfd_mach_sh4;
            disas = print_insn_sh;
            if(endian)
                dis->endian = BFD_ENDIAN_BIG;
            else
                dis->endian = BFD_ENDIAN_LITTLE;
            break;

        default:
            fprintf(stderr, "libxdisasm: Invalid architecture\n");
            return -1;
    }

    return 0;

}

// unsigned int, char*, size_t, int, int -> insn_t *
// Disassemble one instruction from the given buf
insn_t * disassemble_one(unsigned long long vma, char * rawbuf, size_t buflen, int arch, int bits, int endian){
    insn_t * curri = NULL;
    bfd_byte* buf = NULL;
    size_t pos = 0;

    dis = (struct disassemble_info*) calloc(1, sizeof(disassemble_info));
    
    if(!dis){
        return NULL;
    }
   
    init_disassemble_info (dis, stdout, my_fprintf);
    buf = (bfd_byte*) rawbuf;

    disas_options = NULL;
    if(init_dis_env(arch, bits, endian)){
        return NULL;
    }
 
    dis->buffer_vma = vma;
    dis->buffer = buf;
    dis->buffer_length = buflen;
    if(!xdisasm_no_color_g)
    {
        dis->print_address_func = override_print_address;
    }
    dis->disassembler_options = disas_options;

    pos = vma;

  
    curri = (insn_t *) malloc(sizeof(insn_t));
    if(!curri){
        perror("malloc");
        return NULL;
    }

    curri->vma = pos;
    int size = disas((bfd_vma) pos, dis);
    curri->instr_size = size;
    
    if(size < 1){
        return NULL;
    }

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

    free(dis);
    return curri;
}

// unsigned int, char*, size_t, int, int -> insn_list *
// Disassemble the raw buf for the given parameters
insn_list * disassemble(unsigned long long vma, char * rawbuf, size_t buflen, int arch, int bits, int endian){
    insn_list * ilist = NULL;
    bfd_byte* buf = NULL;
    unsigned int count = 0;
    unsigned long long pos = 0, length = 0, max_pos = 0;

    dis = (struct disassemble_info*) calloc(1, sizeof(disassemble_info));

    if(!dis){
        return NULL;
    }

    init_disassemble_info (dis, stdout, my_fprintf);
    buf = (bfd_byte*) rawbuf;

    if(init_dis_env(arch, bits, endian)){
        return NULL;
    }
   
    dis->buffer_vma = vma;
    dis->buffer = buf;
    dis->buffer_length = buflen;
    if(!xdisasm_no_color_g)
    {
        dis->print_address_func = override_print_address;
    }
    dis->disassembler_options = disas_options;

    length = dis->buffer_length;
    max_pos = dis->buffer_vma + length;
    pos = vma;

    while(pos < max_pos){
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
