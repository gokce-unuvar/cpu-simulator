#include "exo1.h"

int main(){

    HashMap *map = hashmap_create();
    if(!map){
        printf("Erreur d'allocation de la hashmap\n");
        return 1;
    }

    // Insertion de valeurs
    char *val1 = "Valeur1";
    char *val2 = "Valeur2";
    char *val3 = "Valeur3";

    hashmap_insert(map, "clé1", (void*)val1);
    hashmap_insert(map, "clé2", (void*)val2);
    hashmap_insert(map, "clé3", (void*)val3);

    // Récupération des valeurs avec vérification
    void *ret;
    ret = hashmap_get(map, "clé1");
    printf("clé1 a comme valeur %s\n", ret ? (char*)ret : "NULL");
    ret = hashmap_get(map, "clé2");
    printf("clé2 a comme valeur %s\n", ret ? (char*)ret : "NULL");
    ret = hashmap_get(map, "clé3");
    printf("clé3 a comme valeur %s\n", ret ? (char*)ret : "NULL");

    // Suppression d'une clé
    hashmap_remove(map, "clé2");
    printf("Après la suppression de clé2 :\n");
    ret = hashmap_get(map, "clé2");
    printf("clé2 a comme valeur %s\n", ret ? (char*)ret : "NULL");

    // Libération de la mémoire
    hashmap_destroy(map);

    return 0;
}
