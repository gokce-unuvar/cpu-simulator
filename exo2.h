#ifndef EXO2
#define EXO2

//Car on a besoin de la structure utilisée dans l'exercice1 (HashMap)
#include "exo1.h"

//Les structures dont on en a besoin dans cet exercice
typedef struct segment{
	int start; //Position de debut (adresse) du segment dans la memoire
	int size; //Taille du segment en unites de memoire
	struct segment* next; //Pointeur vers le segment suivant dans la liste chainee
}Segment;

typedef struct memoryHandler{
	void** memory; //Tableau de pointeurs vers la memoire allouee
	int total_size; //Taille totale de la memoire geree
	Segment* free_list; //Liste chainee des segments de memoire libres
	HashMap* allocated; //Table de hachage (nom, segment)
}MemoryHandler;


//Les signatures des fonctions utilisées
Segment* segment_init(int size);
MemoryHandler* memory_init(int size);
Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev);
void liberer_segment(Segment* s);
int create_segment(MemoryHandler* handler, const char* name, int start, int size);
int remove_segment(MemoryHandler* handler, const char* name);
void free_memory_handler(MemoryHandler * handler);


#endif
