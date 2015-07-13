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

#include "DualStepper.h"

DualStepper::DualStepper(SingleStepper *xs, SingleStepper *ys, unsigned int stepsPerRev)
{
  xStepper = xs;
  yStepper = ys;
  majorAxisSpeed = 0.0;
  maxSpeed = 100.0;
  xStepsPerRev = stepsPerRev;
}

void
DualStepper::setMaxSpeed(float s)
{
  maxSpeed = s;
}

static int
octant(int dx, int dy)
{
  if (dx > 0) {
    if (dy > 0) {
      if (dy < dx)
	return 0;
      else
	return 1;
    } else if (-dy < dx)
      return 7;
    else
      return 6;
  } else if (dy > 0) {
    if (dy < -dx)
      return 3;
    else
      return 2;
  } else if (-dy < -dx)
    return 4;
  else
    return 5;
}

void
DualStepper::travelTo(int ax, int ay, float speed)
{
  int halfRevSteps = xStepsPerRev / 2;
  int dx = abs(ax - xStepper->pos);

  if (dx > halfRevSteps) {
    if (ax > xStepper->pos) {
      xStepper->pos += xStepsPerRev;
    } else {
      xStepper->pos -= xStepsPerRev;
    }
  }
  moveTo(ax, ay, speed);
}

void
DualStepper::moveTo(int ax, int ay, float speed)
{
  xStepper->targetPos = ax;
  yStepper->targetPos = ay;

  long dx = ax - xStepper->pos;
  long dy = ay - yStepper->pos;

  majorAxisSpeed = min(speed, maxSpeed) * max(abs(dx), abs(dy)) / sqrt(dx * dx + dy * dy);

  switch (octant(dx, dy)) {
  case 0:
    _xStepper = xStepper;
    _yStepper = yStepper;
    xdir = FORWARD;
    ydir = FORWARD;
    plotLine(dx, dy);
    break;
  case 1:
    _xStepper = yStepper;
    _yStepper = xStepper;
    xdir = FORWARD;
    ydir = FORWARD;
    plotLine(dy, dx);
    break;
  case 2:
    _xStepper = yStepper;
    _yStepper = xStepper;
    xdir = FORWARD;
    ydir = BACKWARD;
    plotLine(dy, -dx);
    break;
  case 3:
    _xStepper = xStepper;
    _yStepper = yStepper;
    xdir = BACKWARD;
    ydir = FORWARD;
    plotLine(-dx, dy);
    break;
  case 4:
    _xStepper = xStepper;
    _yStepper = yStepper;
    xdir = BACKWARD;
    ydir = BACKWARD;
    plotLine(-dx, -dy);
    break;
  case 5:
    _xStepper = yStepper;
    _yStepper = xStepper;
    xdir = BACKWARD;
    ydir = BACKWARD;
    plotLine(-dy, -dx);
    break;
  case 6:
    _xStepper = yStepper;
    _yStepper = xStepper;
    xdir = BACKWARD;
    ydir = FORWARD;
    plotLine(-dy, dx);
    break;
  case 7:
    _xStepper = xStepper;
    _yStepper = yStepper;
    xdir = FORWARD;
    ydir = BACKWARD;
    plotLine(dx, -dy);
    break;
  }
}

void
DualStepper::plotLine(unsigned int dx, unsigned int dy)
{
  unsigned long usPerStep = 1000000L / majorAxisSpeed;

  // Serial.print("delay: ");
  // Serial.println(usPerStep);

  if (usPerStep >= 3790)
    usPerStep = usPerStep - 3790; // loop takes 3.79 ms without any delay.
  else
    usPerStep = 0;

  int error = 2 * dy - dx;

  unsigned int y = 0;

  // unsigned long time = millis();

  for (int x = 1; x <= dx; x++) {
    _xStepper->step(xdir);
    if (error > 0) {
      _yStepper->step(ydir);
      error += 2 * dy - 2 * dx;
    } else {
      error += 2 * dy;
    }
    if (usPerStep > 0)
      delayMicroseconds(usPerStep);
  }

  // Serial.print("average delay in ms: ");
  // Serial.println((millis() - time) / (float) dx);
}
