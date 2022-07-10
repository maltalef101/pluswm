#include <LibKeybind.h>
#include <LibWM.h>
#include <LibButton.h>
#include <X11/X.h>
#include <vector>

namespace Config {

/* What key will be used as the Super key? */
/* Options:
 * - winkey
 * - l_alt
 */
static constexpr int modkey = l_alt; // VERY IMPORTANT !!

static const unsigned int snap_distance_in_px = 32;

static const unsigned int border_width_in_px = 5;

static const Gaps gaps = Gaps(15, 15, 15, 15);
static const bool smart_gaps = true;

static const std::vector<Rule> rules = {};

static const std::vector<Keybind> keybinds = {
    { modkey, XK_p, KeyAction::Spawn, { .s = "echo" } },
    { modkey, XK_q, KeyAction::KillClient, { .v = nullptr } },
	{ modkey, XK_f, KeyAction::ToggleFullscreen, { .v = nullptr } },
};


static const std::vector<Button> buttons = {
	{ modkey, Button1, ButtonAction::Move },
};

static const std::map<Colors, const char*> colors = {
	{ Colors::WindowBorderActive, "#689d6a" },
	{ Colors::WindowBorderInactive, "#1d2021" }
};

}
