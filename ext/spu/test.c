
# include <stdint.h>

# include "sha256.h"
# include "util.h"

int main(uint64_t speid, uint64_t argp, uint64_t envp)
{
  hash_t hash;
  uint32_t abc[16] = {
    0x61626380, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000018
  };    

  spu_id = speid;
  debugging = 1;

  sha256_update(&hash, abc, H0);
  debug_hash(&hash, "abc");

  return 0;
}
