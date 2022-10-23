#ifndef GRAND_ENTIER_H
#define GRAND_ENTIER_H

#include <stdint.h>

/* A COMPLETER */
/* Définir ici le type grand_entier_t */

typedef struct GrandEntier
{
    uint32_t current;
    struct GrandEntier* next;
} grand_entier_t;

grand_entier_t *ge_cree(void);

void ge_libere(grand_entier_t *);

grand_entier_t *ge_set_bit(grand_entier_t *e, uint32_t x);

grand_entier_t *ge_clr_bit(grand_entier_t *e, uint32_t x);

char ge_get_bit(grand_entier_t *e, uint32_t x);

int ge_nb_bits(grand_entier_t *e);

void ge_clear(grand_entier_t *e);

grand_entier_t *ge_add(grand_entier_t *b, grand_entier_t *a);

grand_entier_t *ge_shift(grand_entier_t *a, int nb_bits);

grand_entier_t *ge_mul(grand_entier_t *b, grand_entier_t *a);

#endif
