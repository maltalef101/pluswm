/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

class Button {
public:
    Button(unsigned int, unsigned int, const char*);

    ~Button() = default;

    unsigned int modmask() const;
    unsigned int button() const;
    std::string action() const;

private:
    unsigned int m_modmask;
    unsigned int m_button;
    const char* m_action;

    enum class Action {
        Resize = 0,
        Move,
    };
};
