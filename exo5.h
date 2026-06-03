#ifndef EXO5
#define EXO5

#include <regex.h>

#include "exo1.h"
#include "exo2.h"

typedef struct{
  MemoryHandler* memory_handler;
  HashMap* context;
  HashMap* constant_pool;//Table de hachage pour stocker les valeurs immédiates
}CPU;

int matches(const char* pattern , const char* string);
CPU* cpu_init(int memory_size);
void cpu_destroy(CPU * cpu);
void* immediate_addressing(CPU* cpu, const char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
CPU* setup_test_environment();
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data);
void* load(MemoryHandler* handler, const char* segment_name, int pos);
void* resolve_addressing(CPU* cpu, const char* operand);


#endif
