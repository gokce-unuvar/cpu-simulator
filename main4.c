#include "exo4.h"

int main(int argc, char * argv[]){

    if (argc < 2) {
        printf("Usage : %s <fichier_DATASS>\n", argv[0]);
        return 1;
    }
    
    ParserResult * res = parse(argv[1]);
    if (res == NULL) {
        printf("Erreur lors du parsre du fichier.\n");
        return 1;
    }
    
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur : Échec de l'initialisation du CPU\n");
        return 1;
    }
    
    allocate_variables(cpu, res->data_instructions, res->data_count);

    print_data_segment(cpu);

    cpu_destroy(cpu);
    
    free_parser_result(res);
    
    return 0;
}
