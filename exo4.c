#include "exo4.h"

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
  
  	cpu->context = hashmap_create();
  	if(!cpu->context){
    	printf("Erreur dans l'allocation de la HashMap \n");
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
  
  	hashmap_destroy(cpu->context);
 
    
  	//Libérer après MemoryHandler
  	free_memory_handler(cpu->memory_handler);
  
  	free(cpu);
  	
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
