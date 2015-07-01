/*
 * Copyright 2015 by Jin Choi <jsc@alum.mit.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef SingleStepper_h
#define SingleStepper_h

#include <Adafruit_MotorShield.h>

class SingleStepper {
 public:
  SingleStepper(Adafruit_StepperMotor *sm);
  void step(uint8_t dir);
  void release() { stepper->release(); }

  volatile int pos;
  int targetPos;

 protected:
  Adafruit_StepperMotor *stepper;
};

#endif
