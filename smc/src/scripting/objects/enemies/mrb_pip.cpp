#include "../../../enemies/pip.hpp"
#include "../../../level/level.hpp"
#include "../../../core/sprite_manager.hpp"
#include "../../../core/property_helper.hpp"
#include "mrb_enemy.hpp"
#include "mrb_pip.hpp"

/**
 * Class: Pip
 *
 * Parent: [Enemy](enemy.html)
 * {: .superclass}
 *
 * _Pip_ is a large worm that slowly moves over the ground and should be
 * quite easy to avoid. Jumping with enough weight onto it will split
 * it up into two small worms, that are moving much faster and will
 * fling you away upwards when killed.
 */

using namespace SMC;
using namespace SMC::Scripting;

// Extern
struct RClass* SMC::Scripting::p_rcPip = NULL;

/**
 * Method: Pip::new
 *
 *   new() → a_pip
 *
 * Creates a new pip with the default values.
 */
static mrb_value Initialize(mrb_state* p_state,  mrb_value self)
{
	cPip* p_pip = new cPip(pActive_Level->m_sprite_manager);
	DATA_PTR(self) = p_pip;
	DATA_TYPE(self) = &rtSMC_Scriptable;

	// This is a generated object
	p_pip->Set_Spawned(true);

	// Let SMC manage the memory
	pActive_Level->m_sprite_manager->Add(p_pip);

	return self;
}

void SMC::Scripting::Init_Pip(mrb_state* p_state)
{
	p_rcPip = mrb_define_class(p_state, "Pip", p_rcEnemy);
	MRB_SET_INSTANCE_TT(p_rcPip, MRB_TT_DATA);

	mrb_define_method(p_state, p_rcPip, "initialize", Initialize, MRB_ARGS_NONE());
}
