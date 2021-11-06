/*
   Mixer subclass for X-configuration quadcopters following the MultiWii numbering convention:

   4cw   2ccw
      \ /
       ^
      / \
   3ccw  1cw

   Copyright (c) 2021 Simon D. Levy

   MIT License
 */

#pragma once

#include "../quad.hpp"

namespace hf {

    class MixerQuadXMW : public MixerQuad {

        public:

            MixerQuadXMW(
                    RotaryMotor * motor1,
                    RotaryMotor * motor2,
                    RotaryMotor * motor3,
                    RotaryMotor * motor4) 

                : MixerQuad(motor1, motor2, motor3, motor4)
            {
                //                     Th  RR  PF  YR
                motorDirections[0] = { +1, -1, +1, -1 };    // 1 right rear
                motorDirections[1] = { +1, -1, -1, +1 };    // 2 right front
                motorDirections[2] = { +1, +1, +1, +1 };    // 3 left rear
                motorDirections[3] = { +1, +1, -1, -1 };    // 4 left front
            }
    };

} // namespace hf
