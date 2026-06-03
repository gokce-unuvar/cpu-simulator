#include "exo6.h"


//pattern est sous forme d'une expression régulière elle est composée par exemple de ^, $, ?
//Exemple d'utilisation : pour savoir si string est un nombre composée de chiffre on fait pour pattern ^[0-9]+$
//Ce qui veut dire contient au moins un chiffre donc peut etre 9 ou 57 ou 366546 etc
//////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////
//retourne 1 si tout va bien, 0 sinon, comme la fonction search_and_replace
int resolve_constants(ParserResult* result) {
    if (!result) {
        printf("Le parser est vide !\n");
        return 0;
    }

    const char* pattern = "^[ABCD]X$";

    printf("Débogage de resolve_constants:\n");
    for (int i = 0; i < result->code_count; i++) {
        printf("Instruction %d\n", i);
        Instruction* instr = result->code_instructions[i];
        if (!instr) {
            printf("Instruction %d est NULL\n", i);
            continue;
        }

        if (instr->operand2 && strcmp(instr->operand2, "") == 0) {
            if (instr->operand1 && !matches(pattern, instr->operand1)) {
                printf("Traitement label '%s'\n", instr->operand1);
                if (search_and_replace(&(instr->operand1), result->labels) == 0) {
                    printf("Erreur de remplacement labels pour '%s'\n", instr->operand1);
                    return 0;
                }
                printf("Label '%s' remplacé\n", instr->operand1);
            }
        } else {
            if (instr->operand1 && instr->operand2 && matches(pattern, instr->operand1)) {
                printf("Traitement variable '%s'\n", instr->operand2);
                if (search_and_replace(&(instr->operand2), result->memory_locations) == 0) {
                    printf("Erreur de remplacement memory_locations pour '%s'\n", instr->operand2);
                    return 0;
                }
                printf("Variable '%s' remplacée\n", instr->operand2);
            }
        }
    }

    return 1;
}

//////////////////////////////////////////////////////////////////
CPU* cpu_init(int memory_size){

  	CPU* cpu = (CPU*)malloc(sizeof(CPU));
  	if(!cpu){
    	printf("Erreur dans l'allocation du cpu \n");
    	return NULL;
  	}
  
  	cpu->memory_handler = memory_init(memory_size);
  	if(!cpu->memory_handler){
    	printf("Erreur dans l'allocation du memory Handler \n");
    	free(cpu);
    	return NULL;
  	}
  	
  	cpu->constant_pool = hashmap_create();
  	if(!cpu->constant_pool){
    	printf("Erreur dans l'allocation de la HashMap\n");
    	free_memory_handler(cpu->memory_handler);
    	free(cpu);
    	return NULL;
  	}
  
  	cpu->context = hashmap_create();
  	if(!cpu->context){
    	printf("Erreur dans l'allocation de la HashMap \n");
    	hashmap_destroy(cpu->constant_pool);	
    	free_memory_handler(cpu->memory_handler);
    	free(cpu);
    	return NULL;
  	}
  
  	int *ax = (int *)malloc(sizeof(int));
    int *bx = (int *)malloc(sizeof(int));
    int *cx = (int *)malloc(sizeof(int));
    int *dx = (int *)malloc(sizeof(int));
    int *ip = (int *)malloc(sizeof(int));
    int *zf = (int *)malloc(sizeof(int));
    int *sf = (int *)malloc(sizeof(int));
  	if(!ax || !bx || !cx || !dx || !ip || !zf || !sf){
        free(ax); 
        free(bx); 
        free(cx); 
        free(dx);
        free(ip); 
        free(zf); 
        free(sf);
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }
    
    *ax = 0; 
    *bx = 0; 
    *cx = 0; 
    *dx = 0;
    *ip = 0; 
    *zf = 0; 
    *sf = 0;
  
  	if(hashmap_insert(cpu->context, "AX", ax) != 0 ||
       hashmap_insert(cpu->context, "BX", bx) != 0 ||
       hashmap_insert(cpu->context, "CX", cx) != 0 ||
       hashmap_insert(cpu->context, "DX", dx) != 0 ||
       hashmap_insert(cpu->context, "IP", ip) != 0 ||
       hashmap_insert(cpu->context, "ZF", zf) != 0 ||
       hashmap_insert(cpu->context, "SF", sf) != 0){
       
        printf("Erreur dans l'insertion des registres dans la HashMap \n");
        free(ax); 
        free(bx); 
        free(cx); 
        free(dx);
        free(ip); 
        free(zf); 
        free(sf);
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }
  
  	return cpu;

}

///////////////////////////////////////////////////////////////////
void cpu_destroy(CPU* cpu) {

    if (!cpu) {
        printf("CPU vide\n");
        return;
    }
    
    if (cpu->context) {
        for (int i = 0; i < cpu->context->size; i++) {
            if (cpu->context->table[i].key &&
                cpu->context->table[i].value != TOMBSTONE) {
                free(cpu->context->table[i].value);
            }
        }
        hashmap_destroy(cpu->context);
    }

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
//Une fonction donnée pour créer une structure CPU et d’initialiser les valeurs des registres
CPU* setup_test_environment(){
	// Initialiser le CPU
	CPU* cpu = cpu_init(1024);
	if(!cpu){
		printf("Error: CPU initialization failed\n");
		return NULL;
	}
	
	// Initialiser les registres avec des valeurs specifiques
	int* ax = (int*)hashmap_get(cpu->context, "AX");
	int* bx = (int*)hashmap_get(cpu->context, "BX");
	int* cx = (int*)hashmap_get(cpu->context, "CX");
	int* dx = (int*)hashmap_get(cpu->context, "DX");

	*ax = 3;
	*bx = 6;
	*cx = 100;
	*dx = 0;

	// Creer et initialiser le segment de donnees
	if(!hashmap_get(cpu->memory_handler->allocated, "DS")){
	
		create_segment(cpu->memory_handler, "DS", 0, 20);

		// Initialiser le segment de donnes avec des valeurs de test
		for(int i = 0; i < 10; i ++){
			int* value = (int*)malloc(sizeof(int));
	*value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
	store(cpu->memory_handler, "DS", i, value);
		}
		
	}
	printf("Test environment initialized . \n");
	return cpu;
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

///////////////////////////////////////////////////////////////////
int handle_instruction(CPU * cpu, Instruction * instr, void * src, void * dest){

	if(!cpu || !instr || !instr->mnemonic){
		printf("Un des paramètres de la fonction handle_instruction est nul\n");
		return 0;		
	}	
	
	if(!cpu->context){
		printf("Le context du cpu est vide\n");
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
	
	printf("Opération non reconnue\n");
	return 0;
	
}

///////////////////////////////////////////////////////////////////
int execute_instruction(CPU * cpu, Instruction * instr){
	
	if(!cpu || !instr){
		printf("Un des paramètres de la fonction execute_instruction est nul\n");
		return 0;		
	}
	
	if(!cpu->context || !instr->mnemonic){
		printf("Contexte du CPU ou mnemonic de l'instruction est null\n");
		return 0;
	}
	
	char* mnemonic = instr->mnemonic;
	void* src = NULL;
	void* dest = NULL;
	
	//Vérifier si on a besoin de deux ou un seul operand selon mnemonic
	//Celles qui ont besoin de deux operandes
	if(strcmp(mnemonic, "MOV") == 0 || strcmp(mnemonic, "ADD") == 0 || strcmp(mnemonic, "CMP") == 0){
		if(!instr->operand1 || !instr->operand2){
			printf("Un des operandes est null\n");
			return 0;
		}
		//Comme dans l'exo5 on utilise une fonction qui detecte le mode d'adressage et retourne l'adresse dans le context du cpu qui convient qui est soit dest pour operand1 soit src pour operand2, on l'utilise ici
		//dst est obligatoirement un registre
		dest = register_addressing(cpu, instr->operand1);
		if(!dest){
			printf("operand1 n'est pas valide\n");
			return 0;
		}
		//src peut etre un registre ou une valeur immediate, mais il est une adresse memoire aussi avec la fnct resolve_constant
		// Vérifier si operand2 est une adresse mémoire (numérique)
        char* pattern = "^[0-9]+$";
        if (matches(pattern, instr->operand2)) {
            int addr = atoi(instr->operand2);
            printf("Chargement depuis DS à l'adresse %d\n", addr);
            src = load(cpu->memory_handler, "DS", addr);
            if (!src) {
                printf("Impossible de charger la valeur à l'adresse %d dans DS\n", addr);
                return 0;
            }
        } else {
            // Autres modes d'adressage
            printf("Tentative de résolution d'adressage pour operand2=%s\n", instr->operand2);
            src = resolve_addressing(cpu, instr->operand2);
            if (!src) {
                printf("operand2 n'est pas valide\n");
                return 0;
            }
        }
		//Appeler la fnct de la qst precedente pour executer l'instruction, avec src et dest des adresse qui peuvent etre caster en *(int*)
		return handle_instruction(cpu, instr, src, dest);
	}
	
	//Celles qui ont besoin d'un seul operande
	if(strcmp(mnemonic, "JMP") == 0 || strcmp(mnemonic, "JZ") == 0 || strcmp(mnemonic, "JNZ") == 0){
		if(!instr->operand1){
			printf("Erreur car operand1 est null\n");
			return 0;
		}
		//On a JMP adress, donc adress doit etre un entier(int) car on a utilisé resolve_constants, donc c'est un adressage immédiat, et operand2 doit etre ""
		src = NULL;
		if(strcmp(instr->operand2, "") != 0){
			printf("Erreur car operand2 n'est pas "" (JMP accepte un seul operand)\n");
			return 0;
		}
		char* pattern = "^[0-9]+$";
    	if (!matches(pattern, instr->operand1)) {
        	printf("operand1 doit être un indice numérique pour JMP\n");
        	return 0;
    	}
    	int target = atoi(instr->operand1);
    	int* dest = (int*)malloc(sizeof(int));
		if(!dest){
			printf("operand1 n'est pas valide\n");
			return 0;
		}
		*dest = target;
    	int result = handle_instruction(cpu, instr, NULL, dest);
    	free(dest);
    	return result;
	}
	
	//Celles qui n'ont besoin d'aucun operand
	if(strcmp(mnemonic, "HALT") == 0){
		if(instr->operand1 || instr->operand2){
			printf("HALT n'accepte aucun operand\n");
			return 0;
		}
		src = NULL;
		dest = NULL;
		return handle_instruction(cpu, instr, src, dest);
	}
	
	printf("Instruction non reconnue\n");
	return 0;
}

///////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////
//L'état de cpu c'est les données présentes dans le cpu qui sont exactement dans le segment de données DS
int run_program(CPU * cpu){
	
	if(!cpu){
		printf("CPU vide\n");
		return 0;		
	}

	if(!cpu->memory_handler || !cpu->memory_handler->memory || !cpu->memory_handler->allocated){
		printf("memory_handler du CPU est vide\n");
		return 0;		
	}
	
	if(!cpu->context){
		printf("Le context du CPU est vide\n");
		return 0;		
	}
	
	//Afficher l'état initiale de la mémoire
	printf("\nL'état initiale de la mémoire : \n");
	//DS
	Segment * ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
	if(!ds){
		printf("Segment de données DS non alloué\n");
	}
	else{
		printf("Le segment de données DS debut=%d et size=%d: \n", ds->start, ds->size);
		int fin_ds = ds->size;
		for(int i = 0; i<fin_ds; i++){
			printf("memory[%d] = %d\n", i+ds->start, *(int*)(cpu->memory_handler->memory[i+ds->start]));
		}
	}
	//Les registres
	printf("\nL'état initiale des registres de la memoire : \n");
	int* ax = (int*)hashmap_get(cpu->context, "AX");
	if(!ax){
		printf("Registre AX non trouvé\n");
		return 0;
	}
	printf("AX = %d\n", *ax);
	int* bx = (int*)hashmap_get(cpu->context, "BX");
	if(!bx){
		printf("Registre BX non trouvé\n");
		return 0;
	}
	printf("BX = %d\n", *bx);
	int* cx = (int*)hashmap_get(cpu->context, "CX");
	if(!cx){
		printf("Registre CX non trouvé\n");
		return 0;
	}
	printf("CX= %d\n", *cx);
	int* dx = (int*)hashmap_get(cpu->context, "DX");
	if(!dx){
		printf("Registre DX non trouvé\n");
		return 0;
	}
	printf("DX = %d\n", *dx);
	int* ip = (int*)hashmap_get(cpu->context, "IP");
	if(!ip){
		printf("Registre IP non trouvé\n");
		return 0;
	}
	printf("IP = %d\n", *ip);
	int* zf = (int*)hashmap_get(cpu->context, "ZF");
	if(!zf){
		printf("Registre ZF non trouvé\n");
		return 0;
	}
	printf("ZF = %d\n", *zf);
	int* sf = (int*)hashmap_get(cpu->context, "SF");
	if(!sf){
		printf("Registre SF non trouvé\n");
		return 0;
	}
	printf("SF = %d\n", *sf);
	
	//L'exécution des instructions
	int choix;
	Instruction * instr = NULL;
	
	do{
		//Récupérer l'instruction à exécuter
		instr = fetch_next_instruction(cpu);
		if(!instr){
			printf("Aucune instruction à exécuter\n");
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
        
        //Demander à l'utilisateur
        do{
			printf("Si vous voulez exécuter cette instruction veuillez appuyer sur entrer(\n), ou sur 'q' pour quitter\n");
			choix = getchar();
			while(getchar() != '\n' && getchar() != EOF); // Vider le buffer, surtout si l'utilisateur a saisie plus de 1 caractère
		}while(choix!='\n' && choix!='q' && choix!='Q');
		
		if(choix=='q' || choix=='Q'){
			// Restaurer IP pour refléter l'instruction actuelle
            int* ip_ptr = (int*)hashmap_get(cpu->context, "IP");
            (*ip_ptr)--;
            break;
		}
		
		//Exécution de l'instruction
        if(!execute_instruction(cpu, instr)){
            printf("Erreur lors de l'exécution de l'instruction %s\n", instr->mnemonic);
            return 0;
        }
        printf("\nL'état des registres après l'exécution de l'instruction %s : \n", instr->mnemonic);
        printf("AX = %d\n", *ax);
        printf("BX = %d\n", *bx);
        printf("CX = %d\n", *cx);
        printf("DX = %d\n", *dx);
        printf("IP = %d\n", *ip);
        printf("ZF = %d\n", *zf);
        printf("SF = %d\n", *sf);

	}while(1);
	
	//Afficher l'état finale de la mémoire
	printf("\nL'état finale de la mémoire : \n");
	if(!ds){
		printf("Segment de données DS non alloué\n");
	}
	else{
		printf("Le segment de données DS debut=%d et size=%d: \n", ds->start, ds->size);
		int fin_ds = ds->size;
		for(int i = 0; i<fin_ds; i++){
			printf("memory[%d] = %d\n", i+ds->start, *(int*)(cpu->memory_handler->memory[i+ds->start]));
		}
	}
	
	printf("\nL'état finale des registres de la memoire : \n");
	printf("AX = %d\n", *ax);
    printf("BX = %d\n", *bx);
    printf("CX = %d\n", *cx);
    printf("DX = %d\n", *dx);
    printf("IP = %d\n", *ip);
    printf("ZF = %d\n", *zf);
    printf("SF = %d\n", *sf);
    
    return 1;

}

///////////////////////////////////////////////////////////////////
void allocate_variables(CPU* cpu, Instruction** data_instructions,
int data_count){
	
	if(!cpu){
		printf("CPU vide\n");
		return;
	}
	
	if(!data_instructions || data_count<=0){
		printf("data_instructions vide\n");
		return;
	}
	
	//Calcul de l’espace nécessaire au stockage des variables
	int taille = 0;
    for(int i = 0; i < data_count; i++){
        Instruction* tmp = data_instructions[i];
        char str[256];  
        strcpy(str, tmp->operand2);  
        int j = 0;
        while(str[j] != '\0'){
            if(str[j] == ','){
                taille++;
            }
            j++;
        }
        taille++;  //nb de virgules + 1
    }
	
	//Déclaration d’un segment nommé DS dans le MemoryHandler
	if(taille > 0){
		if(create_segment(cpu->memory_handler, "DS", 0, taille) == 1){
			printf("Impossible d'allouer le segment DS\n");
			return;
		}
	}else{
		printf("La taille du segment DS est de 0\n");
		return;
	 }
	
	//Remplissage du segment de données avec les valeurs déclarées dans la section .DATA du code
	int pos = 0; //La position dans le segment DS
	for(int i = 0; i < data_count; i++){
        Instruction* tmp = data_instructions[i];
        char str[256]; 
        strcpy(str, tmp->operand2); 
        char num_buffer[256];  //Pour construire les nombres ex : 57
        int j = 0, k = 0; 
        while(str[j] != '\0'){
        
            if(str[j] != ','){
                num_buffer[k++] = str[j]; 
            }
            
            if(str[j] == ',' || str[j + 1] == '\0'){
                num_buffer[k] = '\0';  // Terminer la chaîne
                int* value = (int*)malloc(sizeof(int));
                if(!value){
                    printf("Erreur d'allocation pour %s\n", tmp->mnemonic);
                    continue; //Continuer les autres Instructions
                }
                *value = atoi(num_buffer);
                store(cpu->memory_handler, "DS", pos, (void*)value);
                pos++;
                k = 0;  //Réinitialisation 
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
	
	printf("Aucun adressage n'est trouvé pour l'opérande %s\n", operand);
	return NULL;
	
}
