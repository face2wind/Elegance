#pragma once

#include <time.h>

namespace face2wind {

  class Timer
  {
   public:
    static void Sleep(unsigned int milliseconds);
    static time_t GetNowTimeS();
    static unsigned long long GetNowTimeMS();
  };

}

