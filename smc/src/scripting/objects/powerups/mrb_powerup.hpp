#ifndef SMC_SCRIPTING_POWERUP_HPP
#define SMC_SCRIPTING_POWERUP_HPP
#include "../../scripting.hpp"

namespace SMC {
  namespace Scripting {
    extern struct RClass* p_rcPowerup;
    void Init_Powerup(mrb_state* p_state);
  }
}
#endif
