#include <LibKeybind.h>
#include <LibWM.h>
// #include <LibButton.h>
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

std::vector<Keybind> keybinds = {
    { modkey, XK_q, "kill_client", { .v = nullptr } },
    { modkey, XK_f, "toggle_fullscreen", { .v = nullptr } },
    { modkey, XK_p, "spawn", { .s = "echo" } },
};

}
