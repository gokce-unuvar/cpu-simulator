#include "exo7.h"

int main() {

    CPU* cpu = cpu_init(2048);
    if (!cpu) {
        printf("Erreur lors de l'initialisation du CPU\n");
        return 1;
    }

    ParserResult* result = parse("exo7.txt");
    if (!result) {
        printf("Erreur lors du parsing du fichier\n");
        cpu_destroy(cpu);
        return 1;
    }

    if (!resolve_constants(result)) {
        printf("Erreur lors de la résolution des constantes\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }

    insert_data(cpu, result->data_instructions, result->data_count);

    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        printf("Segment DS non alloué, impossible de continuer\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }

    allocate_code_segment(cpu, result->code_instructions, result->code_count);

    if (!hashmap_get(cpu->memory_handler->allocated, "CS")) {
        printf("Segment CS non alloué, impossible de continuer\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }

    if (!run_program(cpu)) {
        printf("Erreur lors de l'exécution du programme\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }

    free_parser_result(result);
    cpu_destroy(cpu);
    return 0;
}
