#include "exo2.h"

int main(){

    MemoryHandler* handler = memory_init(1000);

    if(handler == NULL){
        printf("Erreur lors de l'initialisation de la mémoire.\n");
        return 1;
    }

    //Création de segments dans la mémoire
    int result = create_segment(handler, "segment1", 0, 200);
    if(result == 0){
        printf("Segment 'segment1' créé avec succès.\n");
    }else{
        printf("Erreur lors de la création du segment 'segment1'.\n");
    }

    result = create_segment(handler, "segment2", 200, 300);
    if(result == 0){
        printf("Segment 'segment2' créé avec succès.\n");
    }else{
        printf("Erreur lors de la création du segment 'segment2'.\n");
    }

    //Afficher les segments alloués
    printf("Les segments alloués:\n");
	for(int i = 0; i < (handler->allocated->size); ++i){
    	HashEntry entry = handler->allocated->table[i];  // Récupérer l'entrée de la table de hachage
    	if(entry.key != NULL && entry.value != TOMBSTONE){  // Vérifier que l'entrée n'est pas vide ou marquée comme supprimée
        	Segment* segment = (Segment*)entry.value;  // Convertir la valeur en un pointeur vers un Segment
        	printf("Nom du segment: %s, Taille: %d\n", entry.key, segment->size);
    	}
	}

    //Libération d'un segment
    result = remove_segment(handler, "segment1");
    if(result == 0){
        printf("Segment 'segment1' supprimé avec succès.\n");
    }else{
        printf("Erreur lors de la suppression du segment 'segment1'.\n");
    }

    //Libérer la mémoire utilisée par le gestionnaire
    free_memory_handler(handler);

    return 0;
}

