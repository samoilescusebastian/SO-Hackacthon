#include "hashmap.h"
#include <stdio.h>
#include <string.h>

typedef struct student {
    char nume[20];
    char clasa[5];
    int nota;
} student_t;

char * print_value(void* data) {
        printf("%p\n", data);

    student_t *st = (student_t *)data;
    char *s = malloc(100);
    printf("%d\n", st->nota);
    strcpy(s, st->nume);
    // strcpy(s + 20, " \n\0");

    return s;
}

void free_value(void* data) {
    free(data);
}

char compare(void* data_1, void* data_2) {
    student_t *st_1 = (student_t *)data_1;
    student_t *st_2 = (student_t *)data_2;
    printf("%s %s\n", st_1->clasa, st_2->clasa);
    printf("%s %s\n", st_1->nume, st_2->nume);

    printf("%d %d\n", st_1->nota, st_2->nota);

    return (st_1->nota == st_2->nota);
}

int main () {
    hashmap_t* hash = create_hashmap(5, compare, free_value, print_value, sizeof(student_t));
    student_t st_1;
    student_t st_2;
    strcpy(st_1 . nume, "Popescu");
    strcpy(st_1.clasa, "12C");
    st_1.nota = 1;

    strcpy(st_2.nume, "Ionel");
    strcpy(st_2.clasa, "12B");
    st_2.nota = 9;

    push(hash, "cheie1", &st_1);
    push(hash, "ch2", &st_2);

    printf("%d\n", remove_key(hash, "cheie1"));

    push(hash, "cheie1", &st_1);



    hashmap_iterator_t it = get_hashmap_it(hash);

    while(has_next_hashmap_it(it)) {
        student_t *node = get_next_hashmap_it(&it);
        if (node == NULL)
            break;

        print_hashmap_node(node);
    }

    free(it);

    free_hashmap(hash);

}