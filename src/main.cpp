/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <memory>

#include <LibWM.h>

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);

    if (argc == 2 && !strcmp("-v", argv[1])) {
        std::cout << "pluswm v" << VERSION << " -- Licensed under the GNU General Public License (GPL)" << std::endl;
        return EXIT_SUCCESS;
    }

    WindowManager* wm = WindowManager::get(XOpenDisplay(nullptr));
    if (!wm) {
        LOG(ERROR) << "Failed to initialize window manager. Mabybe there is already other one running?";
        return EXIT_FAILURE;
    }

    wm->run();

    return EXIT_SUCCESS;
}
