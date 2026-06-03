#include "exo1.h"


//Definir notre fonction de hashage
unsigned long simple_hash(const char *str){

	unsigned long s = 0;
	int i = 0;
	
	while(str[i] != '\0'){
		s += str[i];
		i++;
	}
	
	/*afin que le résultat ne dépasse pas la taille du tableau
	(on veut un résultat entre 0 et 127 qui est (TABLE_SIZE - 1)) */
	
	return s % TABLE_SIZE;
}

HashMap *hashmap_create(){

	HashMap* h = (HashMap*)malloc(sizeof(HashMap));
	
	//Vérifier que l'espace en mémoire est bien alloué
	if(!h){
		printf("Erreur allocation hasmap \n");
		return NULL;
	}
	
	h -> size = TABLE_SIZE;
	//Utilisation de calloc qui alloue et initialise à 0(NULL) au meme temps key et value
	h -> table = (HashEntry*) calloc(TABLE_SIZE, sizeof(HashEntry));
	if(!h -> table){
		printf("Erreur allocation hasmap \n");
		free(h);
		return NULL;
	}
	
	for (int i = 0; i < TABLE_SIZE; i++) {
        h->table[i].key = NULL;
        h->table[i].value = NULL;
    }

	return h;
}

//Retourne 0 si l'insertion est faite avec succès
int hashmap_insert(HashMap *map, const char *key, void *value){

    if(!map || !map->table){
        printf("La structure est vide\n");
        return 1;
    }

    int index = (int)simple_hash(key);
    int start = index;

    //Parcourir la table pour trouver une case libre ou la clé existante donc juste remplacer sa valeur
    do{
        //Si la clé existe déjà, remplacer la valeur
        if(map->table[index].key && strcmp(map->table[index].key, key) == 0){
            free(map->table[index].value); // Libérer l'ancienne valeur
            map->table[index].value = value;
            printf("Mise à jour de la clé %s à l'index %d\n", key, index);
            return 0;
        }
        //Si case vide ou TOMBSTONE, insérer ici
        if(!map->table[index].key || map->table[index].value == TOMBSTONE){
            map->table[index].key = strdup(key);
            if(!map->table[index].key){
                printf("Erreur allocation clé\n");
                return 1;
            }
            map->table[index].value = value;
            printf("L'insertion de la clé %s dans la table est faite\n", key);
            return 0;
        }
        index = (index + 1) % TABLE_SIZE;
    }while(index != start);  //Tant qu'on n'a pas parcouru toute la table

    printf("La table de hachage est pleine, impossible d'insérer l'élément\n");
    return 1;
}

//Récupérer la valeur d'une clé en utilisant le principe de l'adressage ouvert pour gérer les collisions vu qu'on n'utilise pas les listes chainées pour stocker les éléments dans la table
void *hashmap_get(HashMap *map, const char *key){
    if(!map || !map->table){
        printf("La table de hachage est vide\n");
        return NULL;
    }

    int index = (int)simple_hash(key);
    int start = index;

    do {
        if(map->table[index].key && strcmp(map->table[index].key, key) == 0) {
            if (map->table[index].value != TOMBSTONE) {
                return map->table[index].value;
            } else {
                break; // Clé trouvée mais supprimée
            }
        }

        index = (index + 1) % TABLE_SIZE;
    } while(index != start);

    printf("Clé non trouvée : %s\n", key);
    return NULL;
}


int hashmap_remove(HashMap *map, const char *key){

    if(!map || !map->table){
        printf("La table de hachage est vide\n");
        return 1;
    }

    int index = (int)simple_hash(key);
    int start = index; //L'utiliser en cas de collision

    //Recherche de la clé dans la table (en suivant l'adressage ouvert)
    while(map->table[index].key) {
    	//Si la clé est trouvée
        if(strcmp(map->table[index].key, key) == 0) {
            //Libérer la mémoire allouée pour la clé
            free(map->table[index].key);
            map->table[index].key = NULL;

            //Marquer la case comme supprimée
            map->table[index].value = TOMBSTONE;
            //Sortir de la fonction
            printf("La suppression de la clé %s de la table est faite\n", key);
            return 0;
        }
	//La clé n'est pas encore trouvée
        //Passer à la case suivante
        index = (index + 1) % TABLE_SIZE;

        //Si on revient au point de départ, clé non trouvée
        if(index == start) {
            break;
        }
    }

    //Clé non trouvée
    printf("Clé non trouvée\n");
    return 1;
    
}

void hashmap_destroy(HashMap *map){

	if(!map){
		return;
	}
	int freed = 0;
	//Parcourir la table et libérer les clés allouées dynamiquement, les valeur on ne les libere pas car on ne sait pas si elle sont louee dynamiquement ou statiquement
	
	for(int i = 0; i < map -> size; i++){
	
		if (map->table[i].key) {
    printf("exo 1 Libération de la clé %s à l'index %d\n", map->table[i].key, i);
    free(map->table[i].key);
    map->table[i].key = NULL;
    freed++;
}
		
	}
	
	printf("exo 1 HashMap destroyed: %d keys freed\n", freed);
	
	//Libérer la table ensuite la structure
	free(map -> table);
	free(map);
}
