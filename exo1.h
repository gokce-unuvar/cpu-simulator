#ifndef EXO1
#define EXO1

//Charger les bibliothèques dont on en a besoin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Definir les constantes
#define TABLE_SIZE 128
#define TOMBSTONE ((void *)-1)

//Déclarer la structure de la table de hachage
typedef struct hashentry{
	char* key;
	void* value;
}HashEntry;

typedef struct hashmap{
	int size;
	HashEntry* table;
}HashMap;

unsigned long simple_hash(const char *str);
HashMap *hashmap_create();
int hashmap_insert(HashMap *map, const char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);
int hashmap_remove(HashMap *map, const char *key);
void hashmap_destroy(HashMap *map);


#endif
