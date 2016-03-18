#include <common/random.hpp>

#include <time.h>
#include <stdlib.h>

namespace face2wind {

bool Random::has_set_seed = false;

int Random::RandomNum(int max_num)
{
  return Random::RandomNum(0, max_num);
}

int Random::RandomNum(int min_num, int max_num)
{
  if (min_num > max_num)
  {
    int tmp_num = min_num;
    min_num = max_num;
    max_num = tmp_num;
  }

  if (!has_set_seed)
  {
    srand((unsigned int)time(NULL));
    has_set_seed = true;
  }

  int interval_num = max_num - min_num;
  if (interval_num < RAND_MAX)
    return min_num + (rand() % interval_num);
  else
    return int(((rand() % RAND_MAX) * 1.0 / RAND_MAX) * interval_num);
}

}
