#ifndef EXO6
#define EXO6

#include <regex.h>

#include "exo1.h"
#include "exo2.h"
#include "exo3.h"


typedef struct{
  MemoryHandler* memory_handler;
  HashMap* context;
  HashMap* constant_pool;//Table de hachage pour stocker les valeurs immédiates
}CPU;


int matches(const char* pattern , const char* string);
int search_and_replace(char ** str, HashMap * values);
int resolve_constants(ParserResult * result);
CPU * cpu_init(int memory_size); 
void cpu_destroy(CPU * cpu);
void allocate_code_segment(CPU * cpu, Instruction ** code_instructions, int code_count);
int handle_instruction(CPU * cpu, Instruction * instr, void * src, void * dest);
int execute_instruction(CPU * cpu, Instruction * instr);
Instruction * fetch_next_instruction(CPU * cpu);
int run_program(CPU * cpu);
void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count);
void print_data_segment(CPU* cpu);
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data);
void* load(MemoryHandler* handler, const char* segment_name, int pos);
void* immediate_addressing(CPU* cpu, const char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
void* resolve_addressing(CPU* cpu, const char* operand);


#endif
