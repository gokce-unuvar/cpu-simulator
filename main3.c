#include "exo3.h"

int main(int argc, char * argv[]){

    if (argc < 2) {
        printf("Usage : %s <fichier_assembleur>\n", argv[0]);
        return 1;
    }
    
    ParserResult * res = parse(argv[1]);
    if (res == NULL) {
        printf("Erreur lors du parsre du fichier.\n");
        return 1;
    }
    
    Instruction * inst;
    
    // Affichage du résultat du parse
    printf("----- Section .DATA -----\n");
    for (int i = 0; i < res->data_count; i++) {
        inst = res->data_instructions[i];
        printf("%s %s %s\n", inst->mnemonic, inst->operand1, inst->operand2);
    }
    
    //Affichage des tables de hachage
    printf("x a comme valeur %d\n", *(int*)hashmap_get(res->memory_locations, "x"));
    printf("arr a comme valeur %d\n", *(int*)hashmap_get(res->memory_locations, "arr"));
    printf("y a comme valeur %d\n", *(int*)hashmap_get(res->memory_locations, "y"));
    
    //Affichage des count
    printf("data_count a comme valeur %d\n", res->data_count );
    
    
    printf("\n-----Section .CODE -----\n");
    
    for (int i = 0; i < res->code_count; i++) {
        inst = res->code_instructions[i];
        printf("%s %s %s\n", inst->mnemonic, inst->operand1, inst->operand2);
    }
    //Affichage des tables de hachage
    printf("start a comme valeur %d\n", *(int*)hashmap_get(res->labels, "start"));
    printf("loop a comme valeur %d\n", *(int*)hashmap_get(res->labels, "loop")); 
    
    //Affichage des count
    printf("code_count a comme valeur %d\n", res->code_count);
    
    // Suppression de tout
    free_parser_result(res);
    return 0;
}
