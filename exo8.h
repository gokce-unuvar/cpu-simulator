#ifndef EXO8
#define EXO8

#include <regex.h>
#include <limits.h>

#include "exo1.h"
#include "exo2.h"
#include "exo3.h"


typedef struct{
  MemoryHandler* memory_handler;
  HashMap* context;
  HashMap* constant_pool;//Table de hachage pour stocker les valeurs immédiates
}CPU;


void* segment_override_addressing(CPU* cpu, const char* operand);
int find_free_address_strategy(MemoryHandler* handler, int size, int strategy);
void* resolve_addressing(CPU* cpu, const char* operand);
CPU * cpu_init(int memory_size); 
void cpu_destroy(CPU * cpu);
int alloc_es_segment(CPU* cpu);
int free_es_segment(CPU* cpu);
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest);
int run_program(CPU * cpu);

//Les fonctions des exercices précédents
int push_value(CPU *cpu, int value); //Retourne 0 si tout va bien, -1 sinon   //fnct de exo7
int pop_value(CPU* cpu, int *dest); //Retourne 0 si tout va bien, -1 sinon    //fnct de exo7
int matches(const char* pattern , const char* string);
int search_and_replace(char ** str, HashMap * values);
int resolve_constants(ParserResult * result);
void allocate_code_segment(CPU * cpu, Instruction ** code_instructions, int code_count);
Instruction * fetch_next_instruction(CPU * cpu);
void print_data_segment(CPU* cpu);    //fnct de exo4 
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data); //fnct de exo4
void* load(MemoryHandler* handler, const char* segment_name,
int pos);  //fnct de exo4
void insert_data(CPU* cpu, Instruction** data_instructions, int data_count);   //fnct de exo7
void* immediate_addressing(CPU* cpu, const char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void handle_MOV(CPU* cpu, void* src, void* dest);


#endif
