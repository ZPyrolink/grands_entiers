#include <stdlib.h>
#include <stdbool.h>

#include "grand_entier.h"

const size_t ELEMENT_SIZE = sizeof(uint32_t) * 8;
const size_t CURRENT_SIZE = 1024;

grand_entier_t *ge_cree(void)
{
    grand_entier_t *result = malloc(sizeof(grand_entier_t));

    if (result == NULL)
        return NULL;

    grand_entier_t *currentNode = result;

    currentNode->current = 0;
    currentNode->next = NULL;

    for (int i = ELEMENT_SIZE; i < CURRENT_SIZE; i += ELEMENT_SIZE)
    {
        currentNode->next = malloc(sizeof(grand_entier_t));
        currentNode = currentNode->next;

        if (currentNode == NULL)
        {
            ge_libere(result);
            return NULL;
        }

        currentNode->current = 0;
        currentNode->next = NULL;
    }

    return result;
}

void ge_libere(grand_entier_t *e)
{
    if (e->next != NULL)
        ge_libere(e->next);

    e->next = NULL;
    free(e);
}

void GetOffset(uint32_t x, uint32_t *offset, uint32_t *rest)
{
    *offset = x / ELEMENT_SIZE;
    *rest = x % ELEMENT_SIZE;
}

grand_entier_t *ge_GetNode(grand_entier_t *e, uint32_t x, uint32_t *offset, uint32_t *rest)
{
    GetOffset(x, offset, rest);

    grand_entier_t *node = e;

    for (uint32_t i = 0; i < *offset; i++)
        node = node->next;

    return node;
}

grand_entier_t *ge_set_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToSet = ge_GetNode(e, x, &offset, &rest);

    nodeToSet->current = nodeToSet->current | (1 << rest);

    return e;
}

grand_entier_t *ge_clr_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToClear = ge_GetNode(e, x, &offset, &rest);

    nodeToClear->current = nodeToClear->current & ~(1 << rest);
    return e;
}

char ge_get_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToGet = ge_GetNode(e, x, &offset, &rest);

    return (nodeToGet->current >> rest) & 1;
}

int ge_nb_bits_recursive(grand_entier_t *e, int offset)
{
    if (e->next != NULL)
    {
        int32_t result = ge_nb_bits_recursive(e->next, offset + 1);

        if (result > 0)
            return result;
    }

    for (int i = ELEMENT_SIZE - 1; i > 0; i--)
        if (ge_get_bit(e, i) == 0b1)
            return i + 1 + ELEMENT_SIZE * offset;

    return -1;
}

int ge_nb_bits(grand_entier_t *e)
{
    int result = ge_nb_bits_recursive(e, 0);

    if (result < 0)
        return 1;
    else
        return result;
}

void *ge_add_recursive(grand_entier_t *b, grand_entier_t *a, bool deduction)
{
    for (int i = 0; i < ELEMENT_SIZE; i++)
    {
        int currentBBit = ge_get_bit(b, i);
        int currentABit = ge_get_bit(a, i);

        if (!deduction)
        {
            if ((currentBBit == 0b0 && currentABit == 0b0) ||   // 0b0 + 0b0 = 0b0
                (currentBBit == 0b1 && currentABit == 0b0))     // 0b1 + 0b0 = 0b1
            {
                continue;               // Deduction is already
            }

            if (currentBBit == 0b0 && currentABit == 0b1)       // 0b0 + 0b1 = 0b1
                ge_set_bit(b, i); // Deduction is already false

            if (currentBBit == 0b1 && currentABit == 0b1)       // 0b1 + 0b1 = 0b10
            {
                ge_clr_bit(b, i);
                deduction = true;       // Keep deduction for next
            }
        }
        else
        {
            if (currentBBit == 0b0 && currentABit == 0b0)       // <=> currentBBit == 0b0 && currentABit == 0b1
            {
                ge_set_bit(b, i);
                deduction = false;      // Remove deduction
            }

            if ((currentBBit == 0b0 && currentABit == 0b1) ||   // <=> currentBBit == 0b1 && currentABit == 0b1
                (currentBBit == 0b1 && currentABit == 0b0))
            {
                ge_clr_bit(b, i); // Deduction is already true
            }

            if (currentBBit == 0b1 && currentABit == 0b1)       // <=> currentBBit == 0b1 && currentABit == 0b10
                continue; // 0b01 + 0b10 = 0b11, b bit is already correct and deduction is already true
        }
    }

    if (!(b->next == NULL || a->next == NULL))
        b->next = ge_add_recursive(b->next, a->next, deduction);

    return b;
}

grand_entier_t *ge_add(grand_entier_t *b, grand_entier_t *a)
{
    return ge_add_recursive(b, a, false);
}

grand_entier_t *ge_shift(grand_entier_t *a, int nb_bits)
{
    if (nb_bits == 1)
    {
        grand_entier_t *currentNode = a;
        char currentSave, lastSave = -1;

        while (currentNode != NULL)
        {
            if (currentNode->next != NULL)
                currentSave = ge_get_bit(a, ELEMENT_SIZE - 1);

            currentNode->current = currentNode->current << 1;

            if (lastSave >= 0)
            {
                if (lastSave == 0b0)
                    ge_clr_bit(currentNode, 0);

                if (lastSave == 0b1)
                    ge_set_bit(currentNode, 0);
            }

            lastSave = currentSave;
            currentNode = currentNode->next;
        }

        return a;
    }
    else
    {
        for (int i = 0; i < nb_bits; i++)
            a = ge_shift(a, 1);
    }

    return a;
}

grand_entier_t *ge_mul(grand_entier_t *b, grand_entier_t *a)
{
    grand_entier_t *result = ge_cree();

    for (int i = 0; i < CURRENT_SIZE; i++)
    {
        if (ge_get_bit(b, i) == 0b1)
            result = ge_add(result, ge_shift(a, i));
    }

    return result;
}
