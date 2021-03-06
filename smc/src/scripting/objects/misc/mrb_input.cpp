#include "../../events/event.hpp"
#include "../../../input/keyboard.hpp"
#include "../mrb_eventable.hpp"
#include "mrb_input.hpp"

/**
 * Class: InputClass
 *
 * The sole instance of this class is the `Input` singleton which allows
 * you to register for events regarding direct user interaction. Other
 * than that, this class is pretty useless.
 *
 * Events
 * ------
 *
 * Key_Down
 * : Triggered when the player presses one of the SMC-relevant keys,
 *   e.g. the action or jump key. The event handler gets passed the name
 *   of the key in question as a string, i.e. "action", "jump",
 *   etc. Instead of listing all possible keys here I encourage you to
 *   register for the event and print out the handler’s argument in the
 *   console.
 *
 *   The event is also fired for joystick input.
 */

using namespace SMC;
using namespace SMC::Scripting;

// Extern
struct RClass* SMC::Scripting::p_rcInput = NULL;

/***************************************
 * Events
 ***************************************/

MRUBY_IMPLEMENT_EVENT(key_down);

/***************************************
 * Methods
 ***************************************/

static mrb_value Initialize(mrb_state* p_state,  mrb_value self)
{
	mrb_raise(p_state, MRB_NOTIMP_ERROR(p_state), "Cannot create instances of this class.");
	return self; // Not reached
}

/***************************************
 * Binding
 ***************************************/

void SMC::Scripting::Init_Input(mrb_state* p_state)
{
	p_rcInput = mrb_define_class(p_state, "InputClass", p_state->object_class);
	mrb_include_module(p_state, p_rcInput, p_rmEventable);
	MRB_SET_INSTANCE_TT(p_rcInput, MRB_TT_DATA);

	// Make the Input constant the only instance of InputClass
	mrb_define_const(p_state, p_state->object_class, "Input", pKeyboard->Create_MRuby_Object(p_state));

	// Methods
	mrb_define_method(p_state, p_rcInput, "initialize", Initialize, MRB_ARGS_NONE());

	// Event handlers
	mrb_define_method(p_state, p_rcInput, "on_key_down", MRUBY_EVENT_HANDLER(key_down), MRB_ARGS_NONE());
}
