#ifndef SMC_SCRIPTING_MRB_SPRITE_HPP
#define SMC_SCRIPTING_MRB_SPRITE_HPP
#include "../../scripting.hpp"

namespace SMC {
	namespace Scripting {
		extern struct RClass* p_rcSprite;

		void Init_Sprite(mrb_state* p_state);
	}
}
#endif
