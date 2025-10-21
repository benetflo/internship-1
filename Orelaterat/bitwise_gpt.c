#include <stdio.h>
#include <stdint.h>

static inline void set_bit(uint8_t *reg, uint8_t bit)
{
    *reg |= (1 << bit);
}

static inline void clear_bit(uint8_t *reg, uint8_t bit)
{
    *reg &= ~(1 << bit);
}

static inline void toggle_bit(uint8_t *reg, uint8_t bit)
{
    *reg ^= (1 << bit);
}

static inline int bit_set(uint8_t *reg, uint8_t bit)
{
    if(*reg & (1 << bit))
    {
        return 1;
    }else
    {
        return 0;
    }
}

static inline void keep_only_bit(uint8_t *reg, uint8_t bit)
{
    *reg = *reg & (1 << bit); 
}

static inline void set_bits(uint8_t *reg, uint8_t mask)
{
    *reg = *reg | (mask);
}

static inline int count_ones(uint8_t *reg)
{
    uint8_t count = 0;
    uint8_t temp = *reg;

    for(int i = 0; i < 8; i++)
    {
        if((temp & 1) == 1)
        {
            count++;
            temp = temp >> 1;
        }else
        {
            temp = temp >> 1;
        }
    }
    return count; 
}


/*

Isolera lägsta 1-bit
Skriv en funktion som returnerar ett tal där endast den lägsta 1-biten i x är kvar, t.ex. 0b10110000 → 0b00010000.

Räkna ledande nollor
Skriv en funktion som returnerar antalet ledande nollor i ett 8-bitars värde.

Sätt bitar mellan index a och b
Skriv en funktion som sätter alla bitar mellan index a och b (inklusive) i ett tal.
*/

int main()
{
    uint8_t reg_add = 0;
    uint8_t *reg = &reg_add;
    set_bit(reg, 3);
    printf("%d\n", *reg);

    printf("Ones: %d\n", count_ones(reg));
    printf("%d\n", *reg);
    //printf("%d\n", bit_set(reg, 2));
    //keep_only_bit(reg, 2);
    //printf("%d\n", *reg);
    

    return 0;
}