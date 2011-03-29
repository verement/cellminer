
# include <stdint.h>

# include "sha256.h"
# include "util.h"

int main(uint64_t speid, uint64_t argp, uint64_t envp)
{
  spu_id = speid;

  hash_t hash;
  uint32_t abc[16] = {
    0x61626380, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000018
  };    

  sha256_round(hash, abc, H0);
  debug_hash((const hash_t *) &hash, "abc");

  return 0;
}
