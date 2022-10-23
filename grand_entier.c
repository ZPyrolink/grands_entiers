#include <stdlib.h>
#include <stdbool.h>

#include "grand_entier.h"

const size_t ELEMENT_SIZE = sizeof(grand_entier_t) * 8;

grand_entier_t *ge_cree(void)
{
    grand_entier_t *result = malloc(sizeof(grand_entier_t));

    if (result == NULL)
        return NULL;

    result->current = 0;
    result->next = NULL;

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

grand_entier_t *ge_GetNode(grand_entier_t *e, uint32_t x, uint32_t *offset, uint32_t *rest, bool createNew)
{
    GetOffset(x, offset, rest);

    grand_entier_t *node = e;

    for (uint32_t i = 0; i <= *offset; i++)
    {
        if (node->next == NULL)
        {
            if (!createNew)
                return NULL;

            node->next = ge_cree();

            if (node->next == NULL)
                return NULL;
        }

        node = node->next;
    }

    return node;
}

grand_entier_t *ge_set_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToSet = ge_GetNode(e, x, &offset, &rest, true);

    if (nodeToSet == NULL)
        return NULL;

    nodeToSet->current = nodeToSet->current | (1 << rest);

    return e;
}

grand_entier_t *ge_clr_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToClear = ge_GetNode(e, x, &offset, &rest, false);

    if (nodeToClear == NULL)
        return e;

    nodeToClear->current = nodeToClear->current & ~(1 << rest);
    return e;
}

char ge_get_bit(grand_entier_t *e, uint32_t x)
{
    uint32_t offset, rest;
    grand_entier_t *nodeToGet = ge_GetNode(e, x, &offset, &rest, true);

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

    for (int i = ELEMENT_SIZE - 1; i < 0; i--)
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
        return 0;
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

    if (b->next != NULL || a->next != NULL)
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
        bool firstNode = true, lastNode = false;
        grand_entier_t *currentNode = a;
        char currentSave, lastSave;

        while (!lastNode)
        {
            if (currentNode->next == NULL)
                lastNode = true;

            if (!lastNode)
                currentSave = ge_get_bit(a, ELEMENT_SIZE - 1);

            currentNode->current = currentNode->current << 1;

            if (!firstNode)
            {
                if (lastSave == 0b0)
                    ge_clr_bit(currentNode, 0);

                if (lastSave == 0b1)
                    ge_set_bit(currentNode, 0);
            }

            lastSave = currentSave;
            currentNode = currentNode->next;
            firstNode = false;
        }
    }
    else
    {
        for (int i = 0; i < nb_bits; i++)
            ge_shift(a, 1);
    }

    return a;
}

grand_entier_t *ge_mul(grand_entier_t *b, grand_entier_t *a)
{
    grand_entier_t *result = ge_cree();

    for (int i = 0; i < ge_nb_bits(b); i++)
    {
        if (ge_get_bit(b, i) == 0b1)
            result = ge_add(result, ge_shift(a, i));
    }

    return result;
}
