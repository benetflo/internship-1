#include <stdint.h>
#include <stdio.h>

#define STATUS_REG 	0x40021000U

static inline void set_bit(volatile uint32_t *reg, uint8_t bit)
{
	*reg |= (1U << bit);
}

static inline void clear_bit(volatile uint32_t *reg, uint8_t bit)
{
	*reg &= ~(1U << bit);
}



int main(){

	volatile uint32_t *status_reg = (volatile uint32_t*) STATUS_REG;
	set_bit(status_reg, 5);
//	printf("%ls\n", status_reg);
	clear_bit(status_reg, 5);
//	printf("%ls\n", status_reg);

	return 0;
}
