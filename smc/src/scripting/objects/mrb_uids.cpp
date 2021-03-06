#include "mrb_uids.hpp"
#include "../../level/level.hpp"
#include "../../objects/sprite.hpp"
#include "../../core/sprite_manager.hpp"

/*****************************************************************************
 Sprite management

Each active sprite in SMC is assigned a unique number, the UID, available
via the `m_uid' member of cSprite instances (sprites are considered active
if they belong to the cSprite_Manager instance of a cLevel). By default,
these sprites have no associated MRuby objects, allowing a fast level start.
However, as soon as the user starts to index the global `UIDS' object, things
change: Each call to UIDS::[] with a not-yet encountered valid UID (i.e. a
UID actually belonging to an active sprite) will create an MRuby object
wrapping the specific cSprite instance and return it. Additionally, this
MRuby object is cached in an internal `cache' module instance variable so
that further calls to UIDS::[] with the same UID will actually return
the same object (this is very important for event handling). The MRuby
object will continue to exist until the sprite goes inactive, i.e. is
removed from the cSprite_Manager instance, which requests the cache to
delete that specific UID via Delete_UID_From_cache().

There is no static mapping between the C++ cSprite subclasses and the
MRuby Sprite subclasses. Instead, each cSprite subclass (and cSprite
itself) defines a virtual method Create_MRuby_Object() which is supposed
to create an MRuby object of the proper class for the cSprite subclass
instance itself, so that e.g. cEato can create an instance of the Eato
class rather than just an instance of Enemy. Having static lookup tables
for this is bad style, and the method apporach additionally allows you
to “hide” an object from the UIDS hash by just returning mrb_nil_value().
When UIDS::[] is called with a not-yet encountered valid UID as described
above, this will result in a call to the Create_MRuby_Object() method of
the sprite corresponding to the passed UID.

Note that adding the MRuby object directly to the sprite (thus
creating a circular reference between the two) is a bad idea, because
MRuby’s GC won’t know about the C++-side reference. It would just see
an object referenced from nowhere in the Ruby environment, and collect
it. When later the user requests this object, it will not be there,
causing a segfault. Sttoring the MRuby instances in the global constant
`UIDS' ensures that the GC knows about them and won’t collect them.

*****************************************************************************/

/**
 * Module: UIDS
 *
 * The `UIDS` module (yes, really, it’s a module) is a simple way to
 * refer to existing instances of class `Sprite` and its subclasses.
 * It basically just offers the module method `[]` that allows you
 * to retrieve any sprite you wish from the level, identified by its
 * unique identifier (UID):
 *
 * ~~~~ ruby
 * # Move the sprite with the UID 25 away
 * UIDS[25].warp(-100, 0)
 * ~~~~
 *
 * The `UIDS` module maintains a cache for the sprite objects so that it
 * doesn’t have to create MRuby objects for all the sprites right at the
 * beginning of a level, but rather when you first access them. This means
 * that while level loading is fast, referencing a bunch of not-yet-seen
 * sprites will probably cause a noticable pause in the gameplay, so be
 * careful when doing this. After a sprite has first been mapped to MRuby
 * land, referencing it will just cause a lookup in the internal cache
 * and therefore is quite fast.
 */

using namespace SMC;

// Extern
struct RClass* SMC::Scripting::p_rmUIDS = NULL;

// Try to retrieve the given index UID from the cache, and if
// that doesn’t work, do the long shot and insert that sprite
// into the cache, then return the mruby object for it.
// p_state: mruby state
// cache: The UID-sprite cache
// ruid: The mruby fixnum index
static mrb_value _Index(mrb_state* p_state, mrb_value cache, mrb_value ruid)
{
	// Try to retrieve the sprite from the cache
	mrb_value sprite = mrb_hash_get(p_state, cache, ruid);

	// If we already have an object for this UID in the
	// cache, return it.
	if (!mrb_nil_p(sprite))
		return sprite;

	// Otherwise, allocate a new MRuby object for it and store
	// that new object in the cache.
	cSprite_List objs = pActive_Level->m_sprite_manager->objects; // Shorthand
	mrb_int uid = mrb_fixnum(ruid);
	for(cSprite_List::const_iterator iter = objs.begin(); iter != objs.end(); iter++){
		if ((*iter)->m_uid == uid) {
			// Ask the sprite to create the correct type of MRuby object
			// so we don’t have to maintain a static C++/MRuby type mapping table
			mrb_value obj = (*iter)->Create_MRuby_Object(p_state);
			// Store it in the cache
			mrb_hash_set(p_state, cache, ruid, obj);

			return obj;
		}
	}

	return mrb_nil_value();
}

/**
 * Method: UIDS::[]
 *
 *   [uid] → a_sprite
 *   [range] → an_array
 *
 * Retrieve an MRuby object for the sprite with the unique identifier
 * `uid`. The first time you call this method with a given UID, it
 * will cycle through _all_ sprite objects in the level, so it will
 * take relatively long. The sprite object is then cached internally,
 * causing later lookups to be fast.
 *
 * #### Parameter
 * uid
 * : The unique identifier of the sprite you want to retrieve. You can
 *   look this up in the SMC editor. May also be a range.
 *
 * #### Return value
 * Returns an instance of class `Sprite` or one of its subclasses, as
 * required. If the requested UID can’t be found, returns `nil`.
 *
 * If you requested a range, you’ll get an array containing the
 * requested `Sprite` subclass instances instead. The array may
 * contain `nil` values for sprites that could not be found.
 */
static mrb_value Index(mrb_state* p_state, mrb_value self)
{
	mrb_value arg;
	mrb_get_args(p_state, "o", &arg);

	mrb_value cache = mrb_iv_get(p_state, self, mrb_intern_cstr(p_state, "cache"));

	// C++ does not allow us to declare variables inside a `case:' :-(
	mrb_value start = mrb_nil_value();
	mrb_value end   = mrb_nil_value();
	mrb_value ary   = mrb_nil_value();

	switch (mrb_type(arg))  {
	case MRB_TT_FIXNUM: // Single UID requested
		return _Index(p_state, cache, arg);
	case MRB_TT_RANGE: // UID range requested
		start = mrb_funcall(p_state, arg, "first", 0);
		end   = mrb_funcall(p_state, arg, "last", 0);

		// Ensure we get an integer range, and not string or so
		if (mrb_type(start) != MRB_TT_FIXNUM || mrb_type(end) != MRB_TT_FIXNUM) {
			mrb_raise(p_state, MRB_TYPE_ERROR(p_state), "Invalid UID range type.");
			return mrb_nil_value(); // Not reached
		}

		ary = mrb_ary_new(p_state);
		for(mrb_int i=mrb_fixnum(start); i <= mrb_fixnum(end); i++)
			mrb_ary_push(p_state, ary, _Index(p_state, cache, mrb_fixnum_value(i)));

		return ary;
	default:
		mrb_raise(p_state, MRB_TYPE_ERROR(p_state), "Invalid UID type.");
		return mrb_nil_value(); // Not reached
	}
}

/**
 * Method: UIDS::cache_size
 *
 *   cache_size() → an_integer
 *
 * The current size of the UID cache. This method is mainly
 * useful for debugging purposes.
 */
static mrb_value Cache_Size(mrb_state* p_state, mrb_value self)
{
	mrb_value keys = mrb_hash_keys(p_state, mrb_iv_get(p_state, self, mrb_intern_cstr(p_state, "cache")));
	return mrb_fixnum_value(mrb_ary_len(p_state, keys));
}

/**
 * Method: UIDS::cached_uids
 *
 *   cached_uids() → an_array
 *
 * Returns an unsorted array of all UIDs currently cached.
 * This method is mainly useful for debugging purposes.
 */
static mrb_value Cached_UIDs(mrb_state* p_state, mrb_value self)
{
	return mrb_hash_keys(p_state, mrb_iv_get(p_state, self, mrb_intern_cstr(p_state, "cache")));
}

// FIXME: Call Scripting::Delete_UID_From_Cache for sprites
// being removed from a level’s cSprite_Manager!
void SMC::Scripting::Delete_UID_From_Cache(mrb_state* p_state, int uid)
{
	mrb_value cache = mrb_iv_get(p_state, mrb_obj_value(p_rmUIDS), mrb_intern_cstr(p_state, "cache"));
	mrb_hash_delete_key(p_state, cache, mrb_fixnum_value(uid));
}

void SMC::Scripting::Init_UIDS(mrb_state* p_state)
{
	p_rmUIDS = mrb_define_module(p_state, "UIDS");

	// Create a `cache' instance variable invisible from Ruby.
	// This is where the cached sprite instances will be stored,
	// visible for the GC.
	mrb_value cache = mrb_hash_new(p_state);
	mrb_iv_set(p_state, mrb_obj_value(p_rmUIDS), mrb_intern_cstr(p_state, "cache"), cache);

	// UID 0 is always the player
	mrb_hash_set(p_state, cache, mrb_fixnum_value(0), mrb_const_get(p_state, mrb_obj_value(p_state->object_class), mrb_intern_cstr(p_state, "Player")));

	mrb_define_class_method(p_state, p_rmUIDS, "[]", Index, MRB_ARGS_REQ(1));
	mrb_define_class_method(p_state, p_rmUIDS, "cache_size", Cache_Size, MRB_ARGS_NONE());
	mrb_define_class_method(p_state, p_rmUIDS, "cached_uids", Cached_UIDs, MRB_ARGS_NONE());
}
