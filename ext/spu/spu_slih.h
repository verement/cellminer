
# ifndef SPU_SLIH_H
# define SPU_SLIH_H

typedef unsigned int (*spu_slih_func)(unsigned int);

void spu_slih_register(unsigned int mask, spu_slih_func func);

# endif
