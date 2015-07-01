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

#ifndef DualStepper_h
#define DualStepper_h

#include "SingleStepper.h"

class DualStepper
{
 public:
  DualStepper(SingleStepper *xStepper, SingleStepper *yStepper);
  void moveTo(int x, int y, float speed);
  void setMaxSpeed(float ms);
  inline int xPos() { return xStepper->pos; }
  inline int yPos() { return yStepper->pos; }
  inline int xTargetPos() { return xStepper->targetPos; }
  inline int yTargetPos() { return yStepper->targetPos; }
  
 protected:
  // Actual X and Y steppers
  SingleStepper *xStepper;
  SingleStepper *yStepper;

  // Virtual X and Y steppers for Bresenham (transformed to first octant).
  SingleStepper *_xStepper;
  SingleStepper *_yStepper;
  // Stepper direction for transformed steppers.
  uint8_t xdir;
  uint8_t ydir;

  // Bresenham line algorithm. Blocks until finished moving.
  void plotLine(unsigned int dx, unsigned int dy);

  float majorAxisSpeed;
  float maxSpeed;
};
  
#endif
