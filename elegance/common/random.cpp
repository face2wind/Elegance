#include <elegance/common/random.hpp>

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
    srand((unsigned int)time(nullptr));
    has_set_seed = true;
  }

  int interval_num = max_num - min_num;
  if (interval_num < RAND_MAX)
  {
    return min_num + (rand() % interval_num);
  }
  else
  {
    int rand_num = (rand() << 16) + rand();
    if (rand_num < 0)
      rand_num *= -1;
    return min_num + int(rand_num % interval_num);
  }
}

}
