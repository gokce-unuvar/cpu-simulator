#include "exo5.h"

//Une fonction donnée pour tester des expressions régulières

//EXPLICATION DU FONCTIONNEMENT DE CETTE FONCTION

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
  	if(!ax || !bx || !cx || !dx){
        free(ax); 
        free(bx); 
        free(cx); 
        free(dx);
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
  
  	if(hashmap_insert(cpu->context, "AX", ax) != 0 ||
       hashmap_insert(cpu->context, "BX", bx) != 0 ||
       hashmap_insert(cpu->context, "CX", cx) != 0 ||
       hashmap_insert(cpu->context, "DX", dx) != 0){
       
        printf("Erreur dans l'insertion des registres dans la HashMap \n");
        free(ax); 
        free(bx); 
        free(cx); 
        free(dx);
        hashmap_destroy(cpu->context);
        hashmap_destroy(cpu->constant_pool);
        free_memory_handler(cpu->memory_handler);
        free(cpu);
        return NULL;
    }
  
  	return cpu;

}

void cpu_destroy(CPU * cpu){

  	if(!cpu){
  		printf("CPU vide\n");
    	return;
  	}
  
  //On detruit les valeurs car hashmap_destroy detruit que les key et non pas value
  	int* ax = (int*)hashmap_get(cpu->context, "AX");
    int* bx = (int*)hashmap_get(cpu->context, "BX");
    int* cx = (int*)hashmap_get(cpu->context, "CX");
    int* dx = (int*)hashmap_get(cpu->context, "DX");
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    int* sf = (int*)hashmap_get(cpu->context, "SF");
    
  	if(ax){
    	free(ax);
  	}
  	if(bx){
    	free(bx);
  	}
  	if(cx){
  		free(cx);
  	}
  	if(dx){
    	free(dx);
  	}
  	if(ip){
    	free(ip);
  	}
  	if(zf){
    	free(zf);
  	}
  	if(sf){
    	free(sf);
  	}
  	hashmap_destroy(cpu->context);
  	
  	// Libérer les constantes dans constant_pool
    if(cpu->constant_pool){
        for(int i = 0; i < TABLE_SIZE; i++){
            if(cpu->constant_pool->table[i].key && 
               cpu->constant_pool->table[i].value != TOMBSTONE){
                free(cpu->constant_pool->table[i].value);
            }
        }
        hashmap_destroy(cpu->constant_pool);
    }
  
  	free_memory_handler(cpu->memory_handler);
  
  	free(cpu);
  	
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
