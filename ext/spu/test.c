
# include <stdint.h>

# include "sha256.h"
# include "util.h"

int main(uint64_t speid, uint64_t argp, uint64_t envp)
{
  hash_t hash;
  const message_t abc = { {
      0x61626380, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000018
    } };

  spu_id = speid;
  debugging = 1;

  hash = sha256_update(abc, H0);
  debug_hash(&hash, "abc");

  return 0;
}
