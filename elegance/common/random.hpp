#pragma once

namespace face2wind {

class Random
{
 public:
  static int RandomNum(int max_num);
  static int RandomNum(int min_num, int max_num);

 private:
  static bool has_set_seed;
};

}

