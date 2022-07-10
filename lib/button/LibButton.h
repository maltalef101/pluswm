/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

enum class ButtonAction {
	Resize = 0,
	Move,
};

class Button {
public:
    Button(unsigned int, unsigned int, ButtonAction);

    ~Button() = default;

    unsigned int modmask() const;
    unsigned int button() const;
	ButtonAction action() const;

private:
    unsigned int m_modmask;
    unsigned int m_button;
    ButtonAction m_action;

};
