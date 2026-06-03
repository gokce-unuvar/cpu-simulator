#include "exo3.h"
#include <string.h>
#include <ctype.h>

static int next_adr_data = 0;

char* trim(char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}

////////////////////////////////////////////////////////////////////
Instruction* parse_data_instruction(const char* line, HashMap* memory_locations) {
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) {
        printf("Erreur d'allocation de l'instruction pour une ligne (.DATA)\n");
        return NULL;
    }
    inst->mnemonic = NULL;
    inst->operand1 = NULL;
    inst->operand2 = NULL;

    char buffer[256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* var = strtok(buffer, " ");
    char* type = strtok(NULL, " ");
    char* init = strtok(NULL, "\n");

    if (!var || !type || !init) {
        printf("Ligne .DATA mal formée: %s\n", line);
        free(inst);
        return NULL;
    }

    inst->mnemonic = strdup(trim(var));
    inst->operand1 = strdup(trim(type));
    inst->operand2 = strdup(trim(init));
    if (!inst->mnemonic || !inst->operand1 || !inst->operand2) {
        printf("Erreur d'allocation dans Instruction pour les chaînes (.DATA)\n");
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
        return NULL;
    }

    int cpt = 1;
    for (int i = 0; init[i]; i++) {
        if (init[i] == ',') cpt++;
    }

    int adresse = next_adr_data;
    int* addr_ptr = (int*)malloc(sizeof(int));
    if (!addr_ptr) {
        printf("Erreur d'allocation pour l'adresse de %s\n", inst->mnemonic);
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
        return NULL;
    }
    *addr_ptr = adresse;

    if (hashmap_insert(memory_locations, inst->mnemonic, addr_ptr) != 0) {
        printf("Erreur lors de l'insertion de la variable %s dans memory_locations\n", inst->mnemonic);
        free(addr_ptr);
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
        return NULL;
    }
    next_adr_data += cpt;
    printf("Variable '%s' insérée à l'adresse %d, cpt=%d\n", inst->mnemonic, adresse, cpt);

    return inst;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count) {
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    if (!inst) {
        printf("Erreur d'allocation de l'instruction pour une ligne (.CODE)\n");
        return NULL;
    }
    inst->mnemonic = NULL;
    inst->operand1 = NULL;
    inst->operand2 = NULL;

    char buffer[256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* trimmed_line = trim(buffer);
    if (strlen(trimmed_line) == 0) {
        free(inst);
        return NULL; // Ignore les lignes vides
    }

    char* label = NULL;
    char* instruction_part = trimmed_line;

    // Vérifier si la ligne commence par un label
    char* colon = NULL;
    int in_brackets = 0;
    for (char* p = trimmed_line; *p; ++p) {
        if (*p == '[') in_brackets++;
        else if (*p == ']') in_brackets--;
        else if (*p == ':' && in_brackets == 0) {
            colon = p;
            break;
        }
    }

    if (colon) {
        *colon = '\0';
        label = trim(trimmed_line);
        if (strlen(label) > 0) {
            int* index = (int*)malloc(sizeof(int));
            if (!index) {
                printf("Erreur d'allocation pour l'index du label '%s'\n", label);
                free(inst);
                return NULL;
            }
            *index = code_count;
            if (hashmap_insert(labels, label, index) != 0) {
                printf("Erreur lors de l'insertion du label '%s'\n", label);
                free(index);
            } else {
                printf("Label '%s' inséré à l'indice %d\n", label, code_count);
            }
        }
        instruction_part = colon + 1;
        instruction_part = trim(instruction_part);
        if (strlen(instruction_part) == 0) {
            free(inst); // Ne pas créer d'instruction pour un label seul
            return NULL;
        }
    }

    // Découper l'instruction
    char* mnemonic = strtok(instruction_part, " \t");
    if (!mnemonic) {
        printf("Aucun mnémonique trouvé dans la ligne: %s\n", line);
        free(inst);
        return NULL;
    }

    inst->mnemonic = strdup(trim(mnemonic));
    if (!inst->mnemonic) {
        printf("Erreur d'allocation pour mnemonic\n");
        free(inst);
        return NULL;
    }

    char* operands = strtok(NULL, "\n");
    if (operands) {
        operands = trim(operands);
        int depth = 0;
        char* comma = NULL;
        for (char* p = operands; *p; ++p) {
            if (*p == '[') depth++;
            else if (*p == ']') depth--;
            else if (*p == ',' && depth == 0) {
                comma = p;
                break;
            }
        }

        if (comma) {
            *comma = '\0';
            char* op1 = trim(operands);
            char* op2 = trim(comma + 1);
            inst->operand1 = strdup(op1);
            inst->operand2 = strdup(op2);
        } else {
            inst->operand1 = strdup(trim(operands));
            inst->operand2 = strdup("");
        }
    } else {
        inst->operand1 = strdup("");
        inst->operand2 = strdup("");
    }

    if (!inst->operand1 || !inst->operand2) {
        printf("Erreur d'allocation pour les opérandes\n");
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
        free(inst);
        return NULL;
    }

    printf("Instruction parsée: mnemonic='%s', operand1='%s', operand2='%s'\n",
           inst->mnemonic, inst->operand1, inst->operand2);
    return inst;
}

ParserResult* parse(const char* filename) {
    next_adr_data = 0;
    ParserResult* res = (ParserResult*)malloc(sizeof(ParserResult));
    if (!res) {
        printf("Erreur d'allocation de ParserResult\n");
        return NULL;
    }

    res->data_count = 0;
    res->code_count = 0;
    int data_capacity = 10, code_capacity = 10;
    res->data_instructions = (Instruction**)malloc(sizeof(Instruction*) * data_capacity);
    if (!res->data_instructions) {
        printf("Erreur d'allocation du tableau d'instructions data\n");
        free(res);
        return NULL;
    }
    res->code_instructions = (Instruction**)malloc(sizeof(Instruction*) * code_capacity);
    if (!res->code_instructions) {
        printf("Erreur d'allocation du tableau d'instructions code\n");
        free(res->data_instructions);
        free(res);
        return NULL;
    }

    res->labels = hashmap_create();
    if (!res->labels) {
        printf("Erreur lors de la création de la table de hachage labels\n");
        free(res->data_instructions);
        free(res->code_instructions);
        free(res);
        return NULL;
    }
    res->memory_locations = hashmap_create();
    if (!res->memory_locations) {
        printf("Erreur lors de la création de la table de hachage memory_locations\n");
        free(res->data_instructions);
        free(res->code_instructions);
        hashmap_destroy(res->labels);
        free(res);
        return NULL;
    }

    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Erreur lors de l'ouverture du fichier %s\n", filename);
        free_parser_result(res);
        return NULL;
    }

    int section = 0;
    char line[526];
    int current_code_count = 0; // Compteur pour les labels
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char* trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0) continue;

        if (strncmp(trimmed_line, ".DATA", 5) == 0 || strncmp(trimmed_line, ".data", 5) == 0) {
            section = 1;
            continue;
        }
        if (strncmp(trimmed_line, ".CODE", 5) == 0 || strncmp(trimmed_line, ".code", 5) == 0) {
            section = 2;
            continue;
        }

        if (section == 1) {
            Instruction* inst = parse_data_instruction(trimmed_line, res->memory_locations);
            if (!inst) continue;
            if (res->data_count >= data_capacity) {
                data_capacity *= 2;
                Instruction** temp = (Instruction**)realloc(res->data_instructions, sizeof(Instruction*) * data_capacity);
                if (!temp) {
                    printf("Erreur lors du redimensionnement du tableau .DATA\n");
                    free(inst->mnemonic);
                    free(inst->operand1);
                    free(inst->operand2);
                    free(inst);
                    free_parser_result(res);
                    fclose(f);
                    return NULL;
                }
                res->data_instructions = temp;
            }
            res->data_instructions[res->data_count++] = inst;
        } else if (section == 2) {
            Instruction* inst = parse_code_instruction(trimmed_line, res->labels, current_code_count);
            if (!inst) {
                continue; // Ne pas incrémenter current_code_count pour les labels seuls
            }
            if (res->code_count >= code_capacity) {
                code_capacity *= 2;
                Instruction** temp = (Instruction**)realloc(res->code_instructions, sizeof(Instruction*) * code_capacity);
                if (!temp) {
                    printf("Erreur lors du redimensionnement du tableau .CODE\n");
                    free(inst->mnemonic);
                    free(inst->operand1);
                    free(inst->operand2);
                    free(inst);
                    free_parser_result(res);
                    fclose(f);
                    return NULL;
                }
                res->code_instructions = temp;
            }
            res->code_instructions[res->code_count++] = inst;
            current_code_count++; // Incrémenter uniquement pour les instructions valides
        }
    }

    fclose(f);
    return res;
}

void free_parser_result(ParserResult* result) {
    if (!result) return;

    for (int i = 0; i < result->data_count; i++) {
        free(result->data_instructions[i]->mnemonic);
        free(result->data_instructions[i]->operand1);
        free(result->data_instructions[i]->operand2);
        free(result->data_instructions[i]);
    }
    free(result->data_instructions);

    for (int i = 0; i < result->code_count; i++) {
        free(result->code_instructions[i]->mnemonic);
        free(result->code_instructions[i]->operand1);
        free(result->code_instructions[i]->operand2);
        free(result->code_instructions[i]);
    }
    free(result->code_instructions);

    if (result->labels) {
        for (int i = 0; i < result->labels->size; i++) {
            if (result->labels->table[i].key && result->labels->table[i].value != TOMBSTONE) {
                free(result->labels->table[i].value);
            }
        }
        hashmap_destroy(result->labels);
    }

    if (result->memory_locations) {
        for (int i = 0; i < result->memory_locations->size; i++) {
            if (result->memory_locations->table[i].key && result->memory_locations->table[i].value != TOMBSTONE) {
                free(result->memory_locations->table[i].value);
            }
        }
        hashmap_destroy(result->memory_locations);
    }

    free(result);
}
