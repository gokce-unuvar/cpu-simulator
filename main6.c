#include "exo6.h"


int main() {
    //Initialiser le CPU avec une taille de 1024 octets
    CPU *cpu = cpu_init(2048);
    if (!cpu) {
        printf("Erreur lors de l'initialisation du CPU\n");
        return 1;
    }

    //Parser le fichier de programme "programme.txt"
    ParserResult *result = parse("fichier_assembleur.txt");
    if (!result) {
        printf("Erreur lors du parsing du fichier\n");
        cpu_destroy(cpu);
        return 1;
    }

    //Résoudre les constantes (labels et variables)
    if(!resolve_constants(result)){
        printf("Erreur lors de la résolution des constantes\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }
    
    //Allouer les variables dans le segment de données DS
    allocate_variables(cpu, result->data_instructions, result->data_count);

    //Allouer le segment de code CS
    allocate_code_segment(cpu, result->code_instructions, result->code_count);

    //Exécuter le programme
    if(!run_program(cpu)){
    	free_parser_result(result);
    	cpu_destroy(cpu);
        printf("Erreur lors de l'exécution du programme\n");
    }

    //Libérer la mémoire
    free_parser_result(result);
    cpu_destroy(cpu);

    return 0;
}
