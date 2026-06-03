#include "exo5.h"

int main(){

	CPU* cpu = setup_test_environment();
	if(!cpu){
        return 1;
    }
	
	//Récupérer les pointeurs des registres 
    void* ax_ptr = register_addressing(cpu, "AX");
    void* bx_ptr = register_addressing(cpu, "BX");
    void* cx_ptr = register_addressing(cpu, "CX");
	
	printf("\n");
	printf("\n");
	
    //Tester les modes d'adressage avec MOV
    //Adressage immédiat (MOV AX, 7)
    void* src_immediate = immediate_addressing(cpu, "7");
    handle_MOV(cpu, src_immediate, ax_ptr);
    //Afficher le contenu de ce registre
    printf("MOV AX, 7 -> AX = %d\n", *((int*)ax_ptr)); // AX = 7

    //Adressage par registre (MOV AX, CX)
    void* src_cx = register_addressing(cpu, "CX");
    handle_MOV(cpu, src_cx, ax_ptr);
    printf("MOV AX, CX -> AX = %d (CX = %d)\n", *((int*)ax_ptr), *((int*)cx_ptr));

    //Adressage direct (MOV AX, [2])
    void* src_direct = memory_direct_addressing(cpu, "[2]");
    handle_MOV(cpu, src_direct, ax_ptr);
    printf("MOV AX, [2] -> AX = %d\n", *((int*)ax_ptr));

    //Adressage indirect (MOV AX, [BX])
    void* src_indirect = register_indirect_addressing(cpu, "[BX]");
    handle_MOV(cpu, src_indirect, ax_ptr);
    printf("MOV AX, [BX] -> AX = %d (BX = %d)\n", *((int*)ax_ptr), *((int*)bx_ptr)); 
	
	printf("\n");
	printf("\n");
	
	void* value = resolve_addressing(cpu, "7");
	if(value){
		printf("%d\n", *((int*)value));
	}
	value = resolve_addressing(cpu, "CX");
	if(value){
		printf("%d\n", *((int*)value));
	}
	value = resolve_addressing(cpu, "[2]");
	if(value){
		printf("%d\n", *((int*)value));
	}
	value = resolve_addressing(cpu, "[BX]");
	if(value){
		printf("%d\n", *((int*)value));
	}

    cpu_destroy(cpu);

    return 0;
	
}
