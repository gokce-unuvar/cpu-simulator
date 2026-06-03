#include "exo8.h"


//pattern est sous forme d'une expression régulière elle est composée par exemple de ^, $, ?
//Exemple d'utilisation : pour savoir si string est un nombre composée de chiffre on fait pour pattern ^[0-9]+$
//Ce qui veut dire contient au moins un chiffre donc peut etre 9 ou 57 ou 366546 etc
int matches(const char* pattern , const char* string){
	regex_t regex;
	int result = regcomp(&regex, pattern, REG_EXTENDED);
	if(result){
		fprintf(stderr, "Regex compilation failed for pattern : %s \n", pattern);
		return 0;
	}
	result = regexec(&regex, string, 0, NULL, 0);
	regfree(&regex);
	return result == 0;
}




int search_and_replace(char** str, HashMap* values) {
    if (!str || !*str || !values) {
        printf("search_and_replace: paramètres invalides (str=%p, *str=%s, values=%p)\n",
               str, str ? *str : "NULL", values);
        return 0;
    }

    printf("search_and_replace: recherche de '%s'\n", *str);
    int* value = (int*)hashmap_get(values, *str);
    if (!value) {
        printf("search_and_replace: clé '%s' non trouvée\n", *str);
        return 0;
    }

    printf("search_and_replace: valeur trouvée=%d\n", *value);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d", *value);
    free(*str);
    *str = strdup(buffer);
    if (!*str) {
        printf("search_and_replace: échec de strdup pour '%s'\n", buffer);
        return 0;
    }

    printf("search_and_replace: '%s' remplacé par '%s'\n", *str, buffer);
    return 1;
}


//retourne 1 si tout va bien, 0 sinon, comme la fonction search_and_replace
int resolve_constants(ParserResult* result) {
    if (!result) {
        printf("Erreur : ParserResult null\n");
        return 0;
    }
    printf("Débogage de resolve_constants:\n");
    const char* pattern = "^[ABCD]X$|^\\[[A-Z]{2}:[A-Z]{2}\\]$"; // Inclure [ES:AX]
    for (int i = 0; i < result->code_count; i++) {
        printf("Instruction %d\n", i);
        Instruction* instr = result->code_instructions[i];
        if (!instr || !instr->mnemonic) continue;

        // Traiter operand2 pour les variables
        if (instr->operand2 && strcmp(instr->operand2, "") != 0 && 
            instr->operand1 && matches(pattern, instr->operand1)) {
            printf("Traitement variable '%s'\n", instr->operand2);
            if (search_and_replace(&(instr->operand2), result->memory_locations) == 0) {
                printf("Erreur de remplacement memory_locations pour '%s'\n", instr->operand2);
                return 0;
            }
            printf("Variable '%s' remplacée par '%s'\n", instr->operand2, instr->operand2);
        }

        // Traiter operand1 pour les labels
        if (instr->operand1 && strcmp(instr->operand1, "") != 0 && 
            instr->operand2 && strcmp(instr->operand2, "") == 0 &&
            !matches(pattern, instr->operand1)) {
            printf("Traitement label '%s'\n", instr->operand1);
            if (search_and_replace(&(instr->operand1), result->labels) == 0) {
                printf("Erreur de remplacement labels pour '%s'\n", instr->operand1);
                return 0;
            }
        }
    }
    return 1;
}

//operand de l'argument de la fnct est l'operand2 de l'instruction
//ADRESSAGE IMMEDIAT
void* immediate_addressing(CPU* cpu, const char* operand){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;
	}
	
	if(!operand){
		printf("operand donné en argument de la fonction immediate_addressing est vide\n");
		return NULL;
	}
	
	//pattern peut commencer pas un signe - ou pas suivi des chiffres entre 0 et 9 inclu, au moins un
	char* pattern = "^-?[0-9]+$";
	
	if(matches(pattern , operand) == 0){
		return NULL;
	}
	
	//Stocker operand dans constant_pool si elle n'y existe pas
	//Si il y est deja le retourner
	void* existe = hashmap_get(cpu->constant_pool, operand);
	if(existe){
		return existe;
	}
	
	int* value = (int*)malloc(sizeof(int));
	if(!value){
		printf("Erreur à l'allocation dynamique dans la fonction immediate_addressing\n"); 
		return NULL;
	}
	
	*value = atoi(operand);
	if(hashmap_insert(cpu->constant_pool, operand, (void*)value) == 1){
		printf("insertion impossible dans le constant pool\n");
		free(value);
		return NULL;
	}
	
	return (void*)value;
}

///////////////////////////////////////////////////////////////////
//ADRESSAGE PAR REGISTRE
void* register_addressing(CPU* cpu, const char* operand){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;
	}
	
	if(!operand){
		printf("operand donné en argument de la fonction register_addressing est vide\n");
		return NULL;
	}
	
	char* pattern = "^[ABCD]X$";
	
	if(matches(pattern , operand) == 0){
		return NULL;
	}
	
	//Vérifier si operand existe deja dans context
	void* existe = hashmap_get(cpu->context, operand);
	if(existe && existe != TOMBSTONE){
		return existe;
	}
	
	return NULL;
	
}

///////////////////////////////////////////////////////////////////
//ADRESSAGE DIRECT
void* memory_direct_addressing(CPU* cpu, const char* operand){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;
	}
	
	if(!operand){
		printf("operand donné en argument de la fonction memory_direct_addressing est vide\n");
		return NULL;
	}
	
	char* pattern = "^\\[([0-9]+)\\]$";
	
	if(matches(pattern , operand) == 0){
		return NULL;
	}
	
	//Extraire l'adresse qui est entre '[' et ']'
	      //Calculer la taille entre '[' et ']'
	int len = 0;
	while(operand[len] != ']'){
		len++;
	}
	len--;  //Retirer le '['
	     //Recopier l'adresse qui entre '[' et ']', dans s
	char* s = (char*)malloc(len + 1);  //+1 pour '\0'
	if(!s){
		return NULL;
	}
	int i = 0, j = 0;
	while(operand[i] != ']'){
    	if(operand[i] != '['){
        	s[j] = operand[i];  //Copier le caractère
        	j++;
    	}
    	i++;
	}
	s[j] = '\0';  //Terminer la chaîne
	
	int adresse = atoi(s);
	free(s);  //Libérer la mémoire
	
	void* value = load(cpu->memory_handler, "DS", adresse);
	
	return value;
	
}

///////////////////////////////////////////////////////////////////
//ADRESSAGE INDIRECT PAR REGISTRE
void* register_indirect_addressing(CPU* cpu, const char* operand){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;
	}
	
	if(!operand){
		printf("operand donné en argument de la fonction register_indirect_addressing est vide\n");
		return NULL;
	}
	
	char* pattern = "^\\[([ABCD]X)\\]$";
	
	if(matches(pattern , operand) == 1){
		return NULL;
	}
	
	//Extraire l'adresse qui est entre '[' et ']'
	int len = 2;  //La taille est de 2, AX, BX, CX ou DX
	char* s = (char*)malloc(len + 1);  //+1 pour '\0'
	if(!s){
		return NULL;
	}
	//operand peut etre [AX], [BX], [CX] ou [DX]
	s[0] = operand[1];
	s[1] = operand[2];
	s[2] = '\0'; //Terminer s
	
	//Récupérer la valeur de s
	int* adresse = (int*)hashmap_get(cpu->context, s);
	if(!adresse || (void*)adresse == TOMBSTONE){
		printf("registre non retrouvé dans context\n");
		free(s);
		return NULL;
	}
	
	free(s);
	
	//Récupérer la valeur qui est à la position adresse de "DS"
	void* value = load(cpu->memory_handler, "DS", *adresse);
	
	return value;
	
}

///////////////////////////////////////////////////////////////////
void handle_MOV(CPU* cpu, void* src, void* dest){
	
	if(!cpu){
		printf("CPU vide\n");
		return;
	}
	
	if(!src || !dest){
		printf("Source ou destination invalide\n");
		return;
	}
	
	*((int*)dest) = *((int*)src);
}

///////////////////////////////////////////////////////////////////
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data){

  	if(!handler){
    	printf("MemoryHandler vide\n");
    	return NULL;
  	}
  
  	Segment * seg = (Segment *)hashmap_get(handler->allocated, segment_name);
  	if(!seg){
    	printf("Segment non-alloué\n");
    	return NULL;
  	}
  
  	if(seg->size <= pos){
    	printf("Dépassement de la taille du segment\n");
    	return NULL;
  	}
  
  	int position = seg->start + pos;
  
  	handler->memory[position] = data;
  
  	return data;
  
}

///////////////////////////////////////////////////////////////////
void* load(MemoryHandler* handler, const char* segment_name,
int pos){
	
	if(!handler){
    	printf("MemoryHandler vide\n");
    	return NULL;
  	}
  	
  	if(!handler->memory){
    	printf("la table memory de MemoryHandler vide\n");
    	return NULL;
  	}
  	
  	Segment* seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if (!seg) {
        printf("Segment non-alloué\n");
        return NULL;
    }
    
    if(pos >= seg->size){
        printf("Dépassement de la taille du segment\n");
        return NULL;
    }
  	//Car la position dans le segment, pas dans memory
	//donc si on veut la position 1 et que le segment son start est 10 alors on recupere ce qui est a l indice (10+1)=11
    int position = seg->start + pos;
    
    return handler->memory[position];
}

///////////////////////////////////////////////////////////////////
//Il faut stocker les instructions dans la cpu exactement memory_handler
//Le segment de code "CS" est l'endroit où le CPU va chercher les instructions à exécuter
void allocate_code_segment(CPU *cpu, Instruction ** code_instructions, int code_count){
    // Chercher un segment libre pour CS
    Segment *fre = cpu->memory_handler->free_list;
    Segment *prev = NULL;

    while (fre != NULL) {
        if (fre->size >= code_count) {
            // Allouer un segment pour CS
            Segment *cs_seg = malloc(sizeof(Segment));
            cs_seg->start = fre->start;
            cs_seg->size = code_count;
            cs_seg->next = NULL;

            // Insérer CS dans la table des segments alloués
            hashmap_insert(cpu->memory_handler->allocated, "CS", cs_seg);

            // Copier les instructions dans la mémoire
            for (int i = 0; i < code_count; i++) {
                cpu->memory_handler->memory[cs_seg->start + i] = code_instructions[i];
            }

            // Mettre à jour la liste des segments libres
            if (fre->size == code_count) {
                // Juste enlever le segment de la liste
                if (prev == NULL) {
                    cpu->memory_handler->free_list = fre->next;
                } else {
                    prev->next = fre->next;
                }
                free(fre);
            } else {
                fre->start += code_count;
                fre->size -= code_count;
            }

            // Initialiser IP à 0
            int *ip = hashmap_get(cpu->context, "IP");
            if (ip) {
                *ip = 0;
            }

            return;
        }

        prev = fre;
        fre = fre->next;
    }

}

void insert_data(CPU* cpu, Instruction** data_instructions, int data_count) {

    if (!cpu || !cpu->memory_handler) {
        printf("CPU ou memory_handler vide\n");
        return;
    }
    if (!data_instructions || data_count <= 0) {
        printf("data_instructions vide\n");
        return;
    }

	//Récupérer le pointeur vers DS
	Segment* ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if(!ds){
        printf("Segment DS non alloué\n");
        return;
    }	
	
    // Remplir DS avec les valeurs
    int pos = 0;
    for (int i = 0; i < data_count; i++) {
        Instruction* tmp = data_instructions[i];
        if(!tmp || !tmp->operand2){
         	continue;
        }
        char str[256];
        strcpy(str, tmp->operand2);
        char num_buffer[256];
        int j = 0, k = 0;
        while (str[j] != '\0') {
            if (str[j] != ',') {
                num_buffer[k++] = str[j];
            }
            if (str[j] == ',' || str[j + 1] == '\0') {
                num_buffer[k] = '\0';
                if (pos >= ds->size) {
                    printf("Erreur : DS trop petit pour %s (pos=%d, size=%d)\n", tmp->mnemonic, pos, ds->size);
                    return;
                }
                int* value = (int*)malloc(sizeof(int));
                if (!value) {
                    printf("Erreur d'allocation pour %s\n", tmp->mnemonic);
                    continue;
                }
                *value = atoi(num_buffer);
                store(cpu->memory_handler, "DS", pos, (void*)value);
                pos++;
                k = 0;
            }
            j++;
        }
    }
}
///////////////////////////////////////////////////////////////////
void print_data_segment(CPU* cpu){
	
	if(!cpu){
  		printf("CPU vide\n");
    	return;
  	}
  	
  	if(!cpu->memory_handler){
  		printf("memory_handler vide\n");
    	return;
  	}
  	
  	Segment* DS = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
  	if(!DS){
  		printf("Le segment DS est non alloué\n");
  		return;
  	}
  	
  	int index = DS->start;
  	printf("Le contenu du segment DS : \n");
  	while(index != (DS->start + DS->size)){
  		printf("%d\n", *((int*)cpu->memory_handler->memory[index]));
  		index++;
  	}
}


////////////////////////////////////////////////////////////////////
void* segment_override_addressing(CPU* cpu, const char* operand){

    if (!cpu || !operand) return NULL;
    
    // [segment : registre]
    const char* pattern = "^\\[[A-Z]{2}:[A-Z]{2}\\]$";
    if (!matches(pattern, operand)) return NULL; //format invalide
    
    char segment[3], reg[3];
	if (sscanf(operand, "[%2s:%2s]", segment, reg) != 2) {
    	printf("Erreur : Extraction segment/registre échouée\n");
    	return NULL;
	}
    // récupérer segment
    Segment* s = (Segment*)hashmap_get(cpu->memory_handler->allocated, segment);
    if (!s) {
    	printf("Erreur : Segment %s non alloué\n", segment);
    	return NULL;
	}
    // récupérer registre
    int* rp = (int*)hashmap_get(cpu->context, reg);
    if (!rp) {
     	printf("Erreur : registre %s inconnu\n", reg);
    	return NULL;
	}
	//Vérifier les limites du segment s
    int offset = *rp;
    if (offset < 0 || offset >= s->start + s->size) {
    	printf("Erreur : Adresse %d hors segment %s\n", offset, segment);
    	return NULL;
	}
    // retourner cette adresse
    return cpu->memory_handler->memory + s->start + offset;
}

///////////////////////////////////////////////////////////////////
void* resolve_addressing(CPU* cpu, const char* operand){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;
	}
	
	if(!operand){
		printf("operand donné en argument de la fonction register_indirect_addressing est vide\n");
		return NULL;
	}
	
	//Tester les modes d'adressages
	void* value;
	value = immediate_addressing(cpu, operand);
	if(value){
		printf("Le mode d'adressage de l'opérande %s est l'adressage immédiat sa valeur est : \n", operand);
		return value;
	}
	
	value = register_addressing(cpu, operand);
	if(value){
		printf("Le mode d'adressage de l'opérande %s est l'adressage par registre sa valeur est : \n", operand);
		return value;
	}
	
	value = memory_direct_addressing(cpu, operand);
	if(value){
		printf("Le mode d'adressage de l'opérande %s est l'adressage direct sa valeur est : \n", operand);
		return value;
	}
	
	value = register_indirect_addressing(cpu, operand);
	if(value){
		printf("Le mode d'adressage de l'opérande %s est l'adressage indirect par registre sa valeur est : \n", operand);
		return value;
	}
	
	value = segment_override_addressing(cpu, operand);
	if(value){
		printf("Le mode d'adressage de l'opérande %s est l'adressage avec préfixe de segment explicite sa valeur est : \n", operand);
		return value;
	}
	
	printf("Aucun adressage n'est trouvé pour l'opérande %s\n", operand);
	return NULL;
	
}


CPU* cpu_init(int memory_size) {

    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (!cpu) {
        printf("Erreur dans l'allocation du cpu\n");
        return NULL;
    }

    cpu->memory_handler = memory_init(memory_size);
    if (!cpu->memory_handler) {
        printf("Erreur dans l'allocation du memory Handler\n");
        free(cpu);
        return NULL;
    }

    cpu->constant_pool = hashmap_create();
    if (!cpu->constant_pool) {
        printf("Erreur dans l'allocation de la HashMap\n");
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    cpu->context = hashmap_create();
    if (!cpu->context) {
        printf("Erreur dans l'allocation de la HashMap\n");
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Allouer les registres
    int *sp = (int *)malloc(sizeof(int));
    int *bp = (int *)malloc(sizeof(int));
    int *ax = (int *)malloc(sizeof(int));
    int *bx = (int *)malloc(sizeof(int));
    int *cx = (int *)malloc(sizeof(int));
    int *dx = (int *)malloc(sizeof(int));
    int *ip = (int *)malloc(sizeof(int));
    int *zf = (int *)malloc(sizeof(int));
    int *sf = (int *)malloc(sizeof(int));
    int *es = (int *)malloc(sizeof(int)); // Nouveau registre ES

    if (!sp || !bp || !ax || !bx || !cx || !dx || !ip || !zf || !sf || !es) {
        free(sp); free(bp); free(ax); free(bx); free(cx); free(dx); 
        free(ip); free(zf); free(sf); free(es); // Libérer ES
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Initialiser les valeurs
    *sp = 0; *bp = 0; *ax = 0; *bx = 0; *cx = 0; *dx = 0; 
    *ip = 0; *zf = 0; *sf = 0; *es = -1; // ES initialisé à -1

    // Insérer les registres dans context
    if (hashmap_insert(cpu->context, "SP", sp) != 0 ||
        hashmap_insert(cpu->context, "BP", bp) != 0 ||
        hashmap_insert(cpu->context, "AX", ax) != 0 ||
        hashmap_insert(cpu->context, "BX", bx) != 0 ||
        hashmap_insert(cpu->context, "CX", cx) != 0 ||
        hashmap_insert(cpu->context, "DX", dx) != 0 ||
        hashmap_insert(cpu->context, "IP", ip) != 0 ||
        hashmap_insert(cpu->context, "ZF", zf) != 0 ||
        hashmap_insert(cpu->context, "SF", sf) != 0 ||
        hashmap_insert(cpu->context, "ES", es) != 0) { // Insérer ES
        printf("Erreur dans l'insertion des registres dans la HashMap\n");
        free(sp); free(bp); free(ax); free(bx); free(cx); free(dx); 
        free(ip); free(zf); free(sf); free(es); // Libérer ES
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Créer le segment de données DS (taille 256)
    if (create_segment(cpu->memory_handler, "DS", 0, 256) != 0) {
        printf("Impossible d'allouer le segment DS\n");
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Créer le segment de pile SS (taille 128, après DS)
    Segment* ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    int ss_start = ds->start + ds->size + 256; // Réserve un espace pour DS
    int ss_size = 128;
    if (ss_start + ss_size > memory_size) {
        printf("Erreur: mémoire insuffisante pour SS (ss_start=%d, ss_size=%d, memory_size=%d)\n",
               ss_start, ss_size, memory_size);
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }
    if (create_segment(cpu->memory_handler, "SS", ss_start, ss_size) != 0) {
        printf("Impossible d'allouer le segment SS\n");
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Initialiser BP et SP
    *bp = ss_start + ss_size - 1; // Bas de la pile
    *sp = ss_start + ss_size;     // Haut de la pile (décrémente lors de PUSH)

    return cpu;
}

void cpu_destroy(CPU* cpu) {
    if (!cpu) {
        printf("CPU vide\n");
        return;
    }

    // Libérer les registres dans context (y compris ES)
    if (cpu->context) {
        for (int i = 0; i < cpu->context->size; i++) {
            if (cpu->context->table[i].key && 
                cpu->context->table[i].value != TOMBSTONE) {
                free(cpu->context->table[i].value); // Libère SP, BP, AX, ..., ES
            }
        }
        hashmap_destroy(cpu->context);
    }

    // Libérer les constantes dans constant_pool
    if (cpu->constant_pool) {
        for (int i = 0; i < cpu->constant_pool->size; i++) {
            if (cpu->constant_pool->table[i].key && 
                cpu->constant_pool->table[i].value != TOMBSTONE) {
                free(cpu->constant_pool->table[i].value);
            }
        }
        hashmap_destroy(cpu->constant_pool);
    }

    free_memory_handler(cpu->memory_handler);
    free(cpu);
}

///////////////////////////////////////////////////////////////////
//Retourne 1 si tout va bien, -1 sinon
//PUSH donc rajouter un elem à la pile, donc sp va se decrementer de 1
int push_value(CPU *cpu, int value){

	if(!cpu){
		printf("CPU vide\n");
		return -1;
	}
	
	if(!cpu->context || !cpu->memory_handler || !cpu->memory_handler->allocated){
		printf("Un ou plus des composants du CPU est vide\n");
		return -1;
	}
	
	//Récuperer le registre SP
	int* sp = (int*)hashmap_get(cpu->context, "SP");
	if(!sp){
		printf("SP non retrouvé dans le context du CPU\n");
		return -1;
	}
	
	//Récuperer le segment SS
	Segment* ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
	if(!ss){
		printf("Le segment SS non alloué dans memory_handler->allocated du CPU\n");
		return -1;
	}
	
	//Vérifier si la pile est pleine ou pas
	if(*sp <= ss->start){
		printf("La pile est pleine, aucune insertion n'est possible\n");
		return -1;
	}
	
	//Décrémenter le sp
	*sp -= 1;
	
	//Stocker la valeur dans memory du SS
	int* v = (int*)malloc(sizeof(int));
	if(!v){
		printf("Erreur à l'allocation dynamique dans la fnct PUSH\n");
		*sp += 1; //Remettre SP à sa valeur précédente
		return -1;
	}
	*v = value;
	cpu->memory_handler->memory[*sp] = (void*)v;
	
	return 0;
	
}

///////////////////////////////////////////////////////////////////
//Retourne 1 si tout va bien, -1 sinon
int pop_value(CPU* cpu, int *dest){
	
	if(!cpu){
		printf("CPU vide\n");
		return -1;
	}
	
	if(!cpu->context || !cpu->memory_handler || !cpu->memory_handler->allocated){
		printf("Un ou plus des composants du CPU est vide\n");
		return -1;
	}
	
	//Récuperer le registre SP
	int* sp = (int*)hashmap_get(cpu->context, "SP");
	if(!sp){
		printf("SP non retrouvé dans le context du CPU\n");
		return -1;
	}
	
	//Récuperer le segment SS
	Segment* ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
	if(!ss){
		printf("Le segment SS non alloué dans memory_handler->allocated du CPU\n");
		return -1;
	}
	
	//Vérifier si la pile est vide ou pas
	if(*sp >= ss->start + ss->size){
		printf("La pile est vide, aucune valeur à dépiler\n");
		return -1;
	}
	
	//Récupérer la valeur
	int* v = (int*)cpu->memory_handler->memory[*sp];
    if(!v){
        printf("La valeur dépilée est NULL\n");
        return -1;
    }
	*dest = *v;
	
	free(v); //Libérer le pointeur après avoir sauvegarder sa valeur
	cpu->memory_handler->memory[*sp] = NULL;
	//Incrémenter le SP
	*sp += 1;
	
	return 0;
	
}


int find_free_address_strategy(MemoryHandler* handler, int size, int strategy) {

    if (!handler || size < 0 || !handler->free_list){
    	return -1;
    }
    if(strategy < 0 || strategy > 2){
    	printf("Stratégie non reconnue\n");
    	return -1;
    }
    Segment* cur = handler->free_list;
    int chosen = -1;
    if (strategy == 0) {
        // First fit : Retourne le premier segment libre avec taille >= size
        while (cur) {
            if (cur->size >= size) {
                return cur->start;
            }
            cur = cur->next;
        }
        printf("Aucun segment libre trouvé\n");
    } else if (strategy == 1) {
        // Best fit : Retourne le segment avec la taille la plus proche de size
        int best_diff = INT_MAX;
        while (cur) {
            if (cur->size >= size) {
                int diff = cur->size - size;
                if (diff < best_diff) {
                    best_diff = diff;
                    chosen = cur->start;
                }
            }
            cur = cur->next;
        }
        return chosen;
    } else if (strategy == 2) {
        // Worst fit: Retourne le segment avec la plus grande taille disponible
        int max_size = -1;
        while (cur) {
            if (cur->size >= size && cur->size > max_size) {
                max_size = cur->size;
                chosen = cur->start;
            }
            cur = cur->next;
        }
        return chosen;
    }
    
    return -1;// Aucun segment trouvé
}


int alloc_es_segment(CPU* cpu) {

    if (!cpu) return -1;
    int size = 0, strat = 0;

    
    int* pax = (int*)hashmap_get(cpu->context, "AX");
    int* pbx = (int*)hashmap_get(cpu->context, "BX");
    int* ez = (int*)hashmap_get(cpu->context, "ES");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    
    if (!pax) {
        printf("Registre AX non trouvé\n");
        return -1;
    }
    if (!pbx) {
        printf("Registre BX non trouvé\n");
        return -1;
    }
    if (!ez) {
        printf("Registre ES non trouvé\n");
        return -1;
    }
    if (!zf) {
        printf("Registre ZF non trouvé\n");
        return -1;
    }
    
    // Supprimer un segment ES existant, si on l'a deja alloué on le supprime avant de créer un autre
    if (hashmap_get(cpu->memory_handler->allocated, "ES")) {
        if (remove_segment(cpu->memory_handler, "ES") != 0) {
            printf("Erreur : Impossible de libérer l'ancien segment ES\n");
            *zf = 1;
            return -1;
        }
    }
    size = *pax;
    strat = *pbx;
    if (size <= 0) {
        size = 256; // Taille par défaut si AX = 0
        printf("Taille par défaut utilisée pour ES : %d\n", size);
    }
    if (strat < 0 || strat > 2) {
        printf("Stratégie invalide dans BX (%d), utilisation de first fit\n", strat);
        strat = 0; // Par défaut, utiliser first fit
    }
    int addr = find_free_address_strategy(cpu->memory_handler, size, strat);
    if (addr < 0) {
        *zf = 1;
        return -1;
    }
    // créer le segment ES
    if (create_segment(cpu->memory_handler, "ES", addr, size) != 0) {
    	printf("segment ES non cré\n");
        *zf = 1;
        return -1;
    }
    // Vérifier que le segment ES est correctement alloué
    Segment* es = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
    if (!es || es->size != size || es->start != addr) {
        printf("Erreur : Segment ES mal alloué (start=%d, size=%d, attendu start=%d, size=%d)\n",
               es ? es->start : -1, es ? es->size : -1, addr, size);
        remove_segment(cpu->memory_handler, "ES");
        *zf = 1;
        return -1;
    }
    // initialiser à zéro
    for (int i = 0; i < size; i++) {
        int* cell = (int*)malloc(sizeof(int));
        if (!cell) {
            printf("Erreur : Échec de l'allocation pour ES[%d]\n", i);
            remove_segment(cpu->memory_handler, "ES"); // Annuler l'allocation
            *zf = 1;
            return -1;
        }
        *cell = 0;
        store(cpu->memory_handler, "ES", i, cell);
    }
    *ez = addr;
    *zf = 0;
    printf("Segment ES alloué : start=%d, size=%d\n", addr, size);
    return 0;
}

int free_es_segment(CPU* cpu) {

    if (!cpu || !cpu->memory_handler) return -1;
    int* ez = (int*)hashmap_get(cpu->context, "ES");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    if (!ez || !zf) return -1;
    
    // récupérer segment
    Segment* s = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
    if (!s) {
    	printf("Erreur : Segment ES non alloué\n");
        *zf = 1;
        return -1;
    }
    
    // libérer chaque cellule
    for (int i = 0; i < s->size; i++) {
        void* p = load(cpu->memory_handler, "ES", i);
        if (p){
        	free(p);
            store(cpu->memory_handler, "ES", i, NULL); // Nettoyer la case
        }
    }
    
    // Supprimer le segment
    if (remove_segment(cpu->memory_handler, "ES") != 0) {
        printf("Erreur : Échec de la suppression du segment ES\n");
        *zf = 1;
        return -1;
    }
    
    *ez = -1;
    *zf = 0;
    return 0;
}


int handle_instruction(CPU * cpu, Instruction * instr, void * src, void * dest){

	if(!cpu || !instr || !instr->mnemonic){
		printf("Un des paramètres de la fonction handle_instruction est nul\n");
		return 0;		
	}	
	
	if (!cpu->context || !cpu->memory_handler || !cpu->memory_handler->allocated) {
        printf("Le context ou memory_handler du CPU est vide\n");
        return 0;
    }
	
	char* mnemonic = instr->mnemonic;
	
	//MOV transfère la valeur source à la destination
	if(strcmp(mnemonic, "MOV") == 0){
		if(!dest || !src){
			printf("dest ou src est NULL\n");
			return 0;
		}
		*(int*)dest = *(int*)src;
		return 1;
	}
	
	//ADD modifie la destination en ajoutant la source
	if(strcmp(mnemonic, "ADD") == 0){
		if(!dest || !src){
			printf("dest ou src est NULL\n");
			return 0;
		}
		*(int*)dest += *(int*)src;
		return 1;
	}
	
	//CMP affecte les flags sf et zf sans modifier src ou dest
	if(strcmp(mnemonic, "CMP") == 0){
		if(!dest || !src){
			printf("dest ou src est NULL\n");
			return 0;
		}
		int sous = *(int*)dest - *(int*)src;
		
		//condition si sous < 0 alors sf=1 sinon sf=0
		//same pour zf
		int* sf = (int*)hashmap_get(cpu->context, "SF");
		if(!sf){
        	printf("Impossible de mettre SF à 1\n");
        	return 0;
		}
		int* zf = (int*)hashmap_get(cpu->context, "ZF");
		if(!zf){
        	printf("Impossible de mettre ZF à 1\n");
        	return 0;
		}
			
		if(sous < 0){
			*sf = 1;
		}
		else{
			*sf = 0;
		}
		if(sous == 0){
			*zf = 1;
		}
		else{
			*zf = 0;
		}
		return 1;
	}
	
	//JMP change l'instruction suivante à exécuter
	//JMP vers la destination donc utiliser dest et non pas src
	if(strcmp(mnemonic, "JMP") == 0){
		if(!dest){
			printf("dest est NULL\n");
			return 0;
		}
		int* ip = (int*)hashmap_get(cpu->context, "IP");
		if(!ip){
        	printf("Impossible de mettre IP à address\n");
        	return 0;
		}
		*ip = *(int*)dest;
		return 1;
	}
	
	if(strcmp(mnemonic, "JZ") == 0){
		if(!dest){
			printf("dest est NULL\n");
			return 0;
		}
		int* zf = (int*)hashmap_get(cpu->context, "ZF");
		if(!zf){
			printf("zf non trouvé dans le context du cpu\n");
			return 0;
		}
		//La condition
		if((*(int*)zf) == 1){
        	int* ip = (int*)hashmap_get(cpu->context, "IP");
			if(!ip){
        		printf("Impossible de mettre IP à address\n");
        		return 0;
			}
			*ip = *(int*)dest;
        	return 1;
		}
		return 1;
	}
	
	if(strcmp(mnemonic, "JNZ") == 0){
		if(!dest){
			printf("dest est NULL\n");
			return 0;
		}
		int* zf = (int*)hashmap_get(cpu->context, "ZF");
		if(!zf){
			printf("ZF non trouvé dans le context du cpu\n");
			return 0;
		}
		//La condition
		if((*(int*)zf) == 0){
        	int* ip = (int*)hashmap_get(cpu->context, "IP");
			if(!ip){
        		printf("Impossible de mettre IP à address\n");
        		return 0;
			}
			*ip = *(int*)dest;
        	return 1;
		}
		return 1;
	}
	
	//IP est un offset relatif au début du segment de code "CS", donc il pointe sur le début du segment cs, pour indiquer la prochaine instruction à exécuter
	if(strcmp(mnemonic, "HALT") == 0){
		Segment* seg_cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
		if(!seg_cs){
			printf("aucun segment cs n'est alloué dans memory_handler du cpu\n");
			return 0;
		}
		int code_count = seg_cs->size; //Pour récupérer le nombre de case dans memory du memory_handler du cpu occupées par CS
		int* ip = (int*)hashmap_get(cpu->context, "IP");
		if(!ip){
        	printf("Impossible de mettre IP à la fin de CS\n");
        	return 0;
		}
		*ip = code_count;  //Sur l instruction apres les instructions de CS
		return 1;
	}
	
	if(strcmp(mnemonic, "PUSH") == 0){
		if(!src){
			printf("src est NULL\n");
			return 0;
		}
		if(push_value(cpu, *(int*)src) != 0){
			printf("PUSH impossible\n");
			return 0;
		}
		return 1;
	}
	
	if(strcmp(mnemonic, "POP") == 0){
		if(!dest){
			printf("dest est NULL\n");
			return 0;
		}
		int value;
        if (pop_value(cpu, &value) != 0) {
            printf("POP impossible\n");
            return 0;
        }
        *(int*)dest = value;
        return 1;
	}
	
	//Les deux instructions ALLOC et FREE ignorent src et dest, utilisant AX, BX, ES, ZF via les fonctions appelées.
	
	// ALLOC alloue un segment ES en utilisant AX (taille) et BX (stratégie)
    if (strcmp(mnemonic, "ALLOC") == 0) {
        if (alloc_es_segment(cpu) != 0) {
            printf("ALLOC impossible : échec de l'allocation du segment ES\n");
            return 0;
        }
        return 1;
    }
    
    // FREE libère le segment ES
    if (strcmp(mnemonic, "FREE") == 0) {
        if (free_es_segment(cpu) != 0) {
            printf("FREE impossible : échec de la libération du segment ES\n");
            return 0;
        }
        return 1;
    }
	
	printf("Opération non reconnue\n");
	return 0;
	
}

int execute_instruction(CPU* cpu, Instruction* instr) {
    if (!cpu || !instr || !instr->mnemonic || strcmp(instr->mnemonic, "") == 0) {
        printf("Instruction invalide ou mnémonique vide\n");
        return 0;
    }
    char* mnemonic = instr->mnemonic;
    void* src = NULL;
    void* dest = NULL;

    if (strcmp(mnemonic, "MOV") == 0) {
        if (!instr->operand1 || !instr->operand2) {
            printf("Un des opérandes est null pour MOV\n");
            return 0;
        }
        // Vérifier si operand1 est un adressage segment:registre (par exemple, [ES:AX])
        const char* patt = "^\\[[A-Z]{2}:[A-Z]{2}\\]$";
        if (matches(patt, instr->operand1)) {
            dest = segment_override_addressing(cpu, instr->operand1);
            if (!dest) {
                printf("Erreur : operand1 '%s' n'est pas valide\n", instr->operand1);
                return 0;
            }
            printf("Destination résolue pour %s : %p\n", instr->operand1, dest);
        } else {
            dest = register_addressing(cpu, instr->operand1);
            if (!dest) {
                printf("Erreur : operand1 '%s' n'est pas un registre valide\n", instr->operand1);
                return 0;
            }
            printf("Destination résolue pour registre %s : %p\n", instr->operand1, dest);
        }
        char* pattern = "^[0-9]+$";
        if (matches(pattern, instr->operand2)) {
            int addr = atoi(instr->operand2);
            printf("Chargement depuis DS à l'adresse %d\n", addr);
            src = load(cpu->memory_handler, "DS", addr);
            if (!src) {
                printf("Impossible de charger la valeur à l'adresse %d dans DS (src=NULL)\n", addr);
                return 0;
            }
            printf("Valeur chargée depuis DS[%d] = %d\n", addr, *(int*)src);
        } else {
            src = resolve_addressing(cpu, instr->operand2);
            if (!src) {
                printf("operand2 '%s' n'est pas valide\n", instr->operand2);
                return 0;
            }
        }
        return handle_instruction(cpu, instr, src, dest);
    }

    if (strcmp(mnemonic, "PUSH") == 0) {
        if (strcmp(instr->operand2, "") != 0) {
            printf("Erreur : PUSH accepte un seul opérande\n");
            return 0;
        }
        src = register_addressing(cpu, instr->operand1);
        if (!src) {
            printf("operand1 '%s' n'est pas un registre valide pour PUSH\n", instr->operand1);
            return 0;
        }
        printf("Pushing value %d from %s\n", *(int*)src, instr->operand1);
        return handle_instruction(cpu, instr, src, NULL);
    }

    if (strcmp(mnemonic, "POP") == 0) {
        if (strcmp(instr->operand2, "") != 0) {
            printf("Erreur : POP accepte un seul opérande\n");
            return 0;
        }
        dest = register_addressing(cpu, instr->operand1);
        if (!dest) {
            printf("operand1 '%s' n'est pas un registre valide pour POP\n", instr->operand1);
            return 0;
        }
        printf("Popping value to %s\n", instr->operand1);
        return handle_instruction(cpu, instr, NULL, dest);
    }

    if (strcmp(mnemonic, "HALT") == 0) {
        if (strcmp(instr->operand2, "") != 0 || strcmp(instr->operand1, "") != 0) {
            printf("Erreur : HALT n'accepte aucun opérande\n");
            return 0;
        }
        return handle_instruction(cpu, instr, NULL, NULL);
    }
    
    if (strcmp(mnemonic, "ALLOC") == 0) {
        if (strcmp(instr->operand1, "") != 0 || strcmp(instr->operand2, "") != 0) {
            printf("Erreur : ALLOC n'accepte aucun opérande\n");
            return 0;
        }
        printf("Allocation du segment ES\n");
        return handle_instruction(cpu, instr, NULL, NULL);
    }

    if (strcmp(mnemonic, "FREE") == 0) {
        if (strcmp(instr->operand1, "") != 0 || strcmp(instr->operand2, "") != 0) {
            printf("Erreur : FREE n'accepte aucun opérande\n");
            return 0;
        }
        printf("Libération du segment ES\n");
        return handle_instruction(cpu, instr, NULL, NULL);
    }

    printf("Instruction '%s' non reconnue\n", mnemonic);
    return 0;
}

Instruction * fetch_next_instruction(CPU * cpu){
	
	if(!cpu){
		printf("CPU vide\n");
		return NULL;		
	}
	
	if(!cpu->memory_handler || !cpu->memory_handler->memory || !cpu->memory_handler->allocated){
		printf("memory_handler du CPU est vide\n");
		return NULL;		
	}
	
	if(!cpu->context){
		printf("Le context du CPU est vide\n");
		return NULL;		
	}
	
	Instruction * instr = NULL; //ce qu'on retourne
	//Vérifier si IP existe dans le context du CPU
	int * ip = (int*)hashmap_get(cpu->context, "IP");
	if(!ip){
        printf("Pas de IP dans le context du CPU\n");
        return NULL;
	}
	//Vérifier si CS existe dans la memoire du CPU, s'il est un segment alloué dans la memoire
	Segment * cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
	if(!cs){
        printf("CS n'est pas alloué dans la memoire\n");
        return NULL;
	}
	
	//Vérifier si IP est dans les limites
	//IP est toujours 0<= *ip < cs_fin
	int fin_cs = cs->size;
	if(*ip < 0 || *ip>=fin_cs){
		printf("IP n'est pas dans les limites\n");
		return NULL;
	}
	
	//Charger l'instruction depuis la mémoire
	instr = cpu->memory_handler->memory[*ip + cs->start];
	if(!instr){
		printf("Instruction à IP (%d) est nulle\n", *ip);
		return NULL;
	}
	
	//Incrémenter le IP pour pointer sur l'instruction suivante après avoir récupérer la précédente
	*ip += 1;
	
	return instr;
	
}


int run_program(CPU * cpu) {
    if (!cpu || !cpu->memory_handler || !cpu->memory_handler->memory || !cpu->memory_handler->allocated || !cpu->context) {
        printf("CPU ou ses composants sont vides\n");
        return 0;
    }

    // Afficher l'état initial de la mémoire
    printf("\nL'état initial de la mémoire : \n");
    Segment *ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (!ds) {
        printf("Segment de données DS non alloué\n");
    } else {
        printf("Le segment de données DS debut=%d et size=%d: \n", ds->start, ds->size);
        int fin_ds = ds->size;
        for (int i = 0; i < fin_ds; i++) {
            void *ptr = cpu->memory_handler->memory[i + ds->start];
            printf("memory[%d] = %d\n", i + ds->start, ptr ? *(int*)ptr : 0);
        }
    }
    Segment *es = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
    if (!es) {
        printf("Segment de données ES non alloué\n");
    } else {
        printf("Le segment de données ES debut=%d et size=%d: \n", es->start, es->size);
        int fin_es = es->size;
        for (int i = 0; i < fin_es; i++) {
            void *ptr = cpu->memory_handler->memory[i + es->start];
            printf("memory[%d] = %d\n", i + es->start, ptr ? *(int*)ptr : 0);
        }
    }

    // Afficher l'état initial des registres
    printf("\nL'état initial des registres : \n");
    int* ax = (int*)hashmap_get(cpu->context, "AX");
    int* bx = (int*)hashmap_get(cpu->context, "BX");
    int* cx = (int*)hashmap_get(cpu->context, "CX");
    int* dx = (int*)hashmap_get(cpu->context, "DX");
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    int* sf = (int*)hashmap_get(cpu->context, "SF");
    int* es_reg = (int*)hashmap_get(cpu->context, "ES");
    if (!ax || !bx || !cx || !dx || !ip || !zf || !sf || !es_reg) {
        printf("Un ou plusieurs registres non trouvés\n");
        return 0;
    }
    printf("AX = %d\nBX = %d\nCX = %d\nDX = %d\nIP = %d\nZF = %d\nSF = %d\nES = %d\n",
           *ax, *bx, *cx, *dx, *ip, *zf, *sf, *es_reg);

    int choix;
    Instruction *instr = NULL;
    while (1) {
        instr = fetch_next_instruction(cpu);
        if (!instr) {
            printf("Aucune instruction à exécuter ou fin du programme\n");
            break;
        }
        printf("L'instruction à exécuter est : %s", instr->mnemonic);
        if (instr->operand1 && strcmp(instr->operand1, "") != 0) {
            printf(" %s", instr->operand1);
            if (instr->operand2 && strcmp(instr->operand2, "") != 0) {
                printf(",%s", instr->operand2);
            }
        }
        printf("\n");

        do {
            printf("Si vous voulez exécuter cette instruction veuillez appuyer sur entrer(\n), ou sur 'q' pour quitter\n");
            choix = getchar();
            while (getchar() != '\n' && getchar() != EOF); // Vider le buffer
        } while (choix != '\n' && choix != 'q' && choix != 'Q');

        if (choix == 'q' || choix == 'Q') {
            int* ip_ptr = (int*)hashmap_get(cpu->context, "IP");
            if (ip_ptr) (*ip_ptr)--; // Restaurer IP
            break;
        }

        if (!execute_instruction(cpu, instr)) {
            printf("Erreur lors de l'exécution de l'instruction %s\n", instr->mnemonic);
            return 0;
        }

        if (strcmp(instr->mnemonic, "HALT") == 0) {
            printf("Programme terminé avec HALT\n");
            break;
        }

        printf("\nL'état de la mémoire après l'exécution de l'instruction %s : \n", instr->mnemonic);
        printf("AX = %d\nBX = %d\nCX = %d\nDX = %d\nIP = %d\nZF = %d\nSF = %d\nES = %d\n",
               *ax, *bx, *cx, *dx, *ip, *zf, *sf, *es_reg);

        es = (Segment*)hashmap_get(cpu->memory_handler->allocated, "ES");
        if (!es) {
            printf("Segment de données ES non alloué\n");
        } else {
            printf("Le segment de données ES debut=%d et size=%d: \n", es->start, es->size);
            int fin_es = es->size;
            for (int i = 0; i < fin_es; i++) {
                void *ptr = cpu->memory_handler->memory[i + es->start];
                printf("memory[%d] = %d\n", i + es->start, ptr ? *(int*)ptr : 0);
            }
        }

        ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
        if (!ds) {
            printf("Segment de données DS non alloué\n");
        } else {
            printf("Le segment de données DS debut=%d et size=%d: \n", ds->start, ds->size);
            int fin_ds = ds->size;
            for (int i = 0; i < fin_ds; i++) {
                void *ptr = cpu->memory_handler->memory[i + ds->start];
                printf("memory[%d] = %d\n", i + ds->start, ptr ? *(int*)ptr : 0);
            }
        }
    }

    // Afficher l'état final
    printf("\nL'état final de la mémoire : \n");
    if (!ds) {
        printf("Segment de données DS non alloué\n");
    } else {
        printf("Le segment de données DS debut=%d et size=%d: \n", ds->start, ds->size);
        int fin_ds = ds->size;
        for (int i = 0; i < fin_ds; i++) {
            void *ptr = cpu->memory_handler->memory[i + ds->start];
            printf("memory[%d] = %d\n", i + ds->start, ptr ? *(int*)ptr : 0);
        }
    }
    if (!es) {
        printf("Segment de données ES non alloué\n");
    } else {
        printf("Le segment de données ES debut=%d et size=%d: \n", es->start, es->size);
        int fin_es = es->size;
        for (int i = 0; i < fin_es; i++) {
            void *ptr = cpu->memory_handler->memory[i + es->start];
            printf("memory[%d] = %d\n", i + es->start, ptr ? *(int*)ptr : 0);
        }
    }

    printf("\nL'état final des registres : \n");
    printf("AX = %d\nBX = %d\nCX = %d\nDX = %d\nIP = %d\nZF = %d\nSF = %d\nES = %d\n",
           *ax, *bx, *cx, *dx, *ip, *zf, *sf, *es_reg);

    return 1;
}
