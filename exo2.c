#include "exo2.h"

Segment* segment_init(int size){
	Segment* s = (Segment*)malloc(sizeof(Segment));
	if(!s){
		printf("Erreur à l'allocation dynamique de Segment\n");
		return NULL;
	}
	s -> start = 0;
	s -> size = size;
	s -> next = NULL;
	return s;
}

MemoryHandler* memory_init(int size){
	MemoryHandler* MH = (MemoryHandler*)malloc(sizeof(MemoryHandler));
	if(!MH){
		printf("Erreur à l'allocation dynamique de MemoryHandler\n");
		return NULL;
	}
	
	MH -> memory = (void**)malloc(size * sizeof(void*));
	if(!MH -> memory){
		printf("Erreur à l'allocation dynamique\n");
		free(MH);
		return NULL;
	}

	// Initialiser toutes les cases à NULL
	for(int i = 0; i < size; i++){
    	MH -> memory[i] = NULL;
	}
	
	MH -> total_size = size;
	MH -> free_list = segment_init(size);
	if(!MH -> free_list){
		free(MH -> memory);
		free(MH);
		return NULL;
	}
	
	MH -> allocated = hashmap_create();
	if(!MH -> allocated){
		free(MH -> memory);
		free(MH -> free_list);
		free(MH);
		return NULL;
	}
	
	return MH;
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
	*prev = NULL; // Initialisation de prev à NULL si aucun segment n'est trouvé
	
	if(!handler){
		printf("MemoryHandler est vide\n");
		return NULL;
	}
	
	Segment* prec = NULL;
	Segment* act = handler -> free_list;
	int seg_end;
	
	while(act){
		seg_end = act -> start + act -> size;
		if((act -> start <= start) && (seg_end >= size + start)){
			*prev = prec;
			return act;
		}	
		prec = act;
		act = act -> next;
	}
	return NULL;
}

int create_segment(MemoryHandler* handler, const char* name, int start, int size){

    if(!handler || !name){
        printf("MemoryHandler ou nom de segment invalide\n");
        return 1;
    }
    
    if(start < 0 || start + size > handler->total_size){ 
        printf("Adresse ou taille invalide\n");
        return 1;
    }
    
    // Vérifier s'il y a un espace mémoire libre suffisant
    Segment* prev;
    Segment* seg_libre = find_free_segment(handler, start, size, &prev);
    if(!seg_libre){
        printf("Aucun espace mémoire libre suffisant est disponible à l’adresse indiquée\n");
        return 1;
    }
    
    // Créer un nouveau segment
    Segment* new_seg = segment_init(size);
    if(!new_seg){
        return 1;
    }
    new_seg->start = start;  // Ajuster start ici car segment_init ne le prend pas
    
    // Ajouter à la table de hachage
    int res = hashmap_insert(handler->allocated, name, new_seg);
    if(res == 1){
        free(new_seg);  // Correction : libérer si l’insertion échoue
        return 1;
    }

    // Stocker dans memory, une boucle for car le nb de case occupé par un segment depend de sa taille
    for(int i = start; i < start + size; i++){
        handler->memory[i] = new_seg;
    }
    
    // Ajuster la liste des segments libres
    if(seg_libre->start == start && seg_libre->size == size){
        // Si le segment libre est entièrement consommé, on le supprime
        if(prev){
            prev->next = seg_libre->next;
        }else{
            handler->free_list = seg_libre->next;
        }
        free(seg_libre);
    }else{
        // Simplifié : on ajuste seulement la partie après, sans créer de segment avant
        int original_end = seg_libre->start + seg_libre->size;
        seg_libre->start = start + size;
        seg_libre->size = original_end - seg_libre->start;  // Correction : calculer la taille restante
    }
    return 0;
}

int remove_segment(MemoryHandler* handler, const char* name) {
	if(!handler || !name) {
		printf("MemoryHandler ou nom de segment invalide\n");
		return 1;
	}

	// Trouver le segment
	Segment* seg_to_remove = hashmap_get(handler -> allocated, name);
	if(!seg_to_remove){
		printf("Le segment avec le nom '%s' n'a pas été trouvé\n", name);
		return 1;
	}
	
	if(seg_to_remove->start + seg_to_remove->size > handler->total_size){  
        printf("Erreur : segment hors limites\n");
        return 1;
    }

	// Retirer de la table de hachage
	hashmap_remove(handler -> allocated, name);

	// Supprimé du tableau memory, en mettant ses cases à NULL, pas de free car c'est deja fait par la fonction hashmap_remove
	for(int i = seg_to_remove->start; i < seg_to_remove->start + seg_to_remove->size; i++){
		handler -> memory[i] = NULL;
	}

	// Ajouter à la liste des segments libres
	Segment* prev = NULL;
	Segment* current = handler -> free_list;

	while (current && current -> start < seg_to_remove -> start){
		prev = current;
		current = current -> next;
	}

	// Insérer dans free_list
	seg_to_remove -> next = current;
	if(prev){
		prev -> next = seg_to_remove;
	}else{
		handler -> free_list = seg_to_remove;
	}

	// Fusion avec les segments adjacents
	if(prev && prev -> start + prev -> size == seg_to_remove -> start){
		prev -> size += seg_to_remove -> size;
		prev -> next = seg_to_remove -> next;
		free(seg_to_remove);
		seg_to_remove = prev;
	}

	if(seg_to_remove -> next && seg_to_remove -> start + seg_to_remove -> size == seg_to_remove -> next -> start){
		seg_to_remove -> size += seg_to_remove -> next -> size;
		Segment* temp = seg_to_remove -> next;
		seg_to_remove -> next = seg_to_remove -> next -> next;
		free(temp);
	}

	return 0;
}

void free_memory_handler(MemoryHandler* handler) {
    if (!handler) return;

    // Libérer les int* dans memory DS et SS
    if (handler->memory && handler->allocated) {
        Segment* ds = (Segment*)hashmap_get(handler->allocated, "DS");
        Segment* cs = (Segment*)hashmap_get(handler->allocated, "CS");
        Segment* ss = (Segment*)hashmap_get(handler->allocated, "SS");
        //Je ne le fais pas pour CS car lui il stocke des Segment* qui sont deja libérés par une autre fnct et non pas des int*
        if (ds) {
            for (int i = ds->start; i < ds->start + ds->size; i++) {
                if (handler->memory[i] && handler->memory[i] != ds && (!cs || handler->memory[i] != cs)) {
                    free(handler->memory[i]); // Libère int*
                    handler->memory[i] = NULL;
                }
            }
        }
        
        // Libérer SS
        if (ss) {
            for (int i = ss->start; i < ss->start + ss->size; i++) {
                if (handler->memory[i] && handler->memory[i] != ds && 
                    (!cs || handler->memory[i] != cs) && handler->memory[i] != ss) {
                    free(handler->memory[i]);
                    handler->memory[i] = NULL;
                }
            }
        }
    }

    // Libérer allocated
    if (handler->allocated) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            if (handler->allocated->table[i].key && 
                handler->allocated->table[i].value != TOMBSTONE) {
                free(handler->allocated->table[i].value); // Libère Segment*
                handler->allocated->table[i].value = NULL;
            }
        }
        hashmap_destroy(handler->allocated);
    }

    // Libérer free_list
    Segment* tmp = handler->free_list;
    while (tmp) {
        Segment* next = tmp->next;
        free(tmp);
        tmp = next;
    }

    free(handler->memory);
    free(handler);
}
