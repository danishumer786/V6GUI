// James Butcher
// 2/15/24
//
// Allows assigning a motor to a pair of keys on the keyboard so you can then
// control the motor with the keys.
//
// 4 slots:
//	- Up + Down
//	- Left + Right
//	- W + S
//	- A + D

#pragma once

#include "MotorControlPanel.h"


class MotorKeyboardControl {

public:
	MotorKeyboardControl(MotorControlPanel* _motor);



private:
	MotorControlPanel* motor;


};

