#pragma once

#include "scheduler.h"
#include "timer.h"

namespace arvin
{
  class IOManager:public Scheduler,public TimerManager
  {
    
  };
}