/*
   Angular-velocity-based PID controller for yaw

   Copyright (c) 2018 Juan Gallostra and Simon D. Levy

   MIT License
 */

#pragma once

#include "../HF_filters.hpp"

#include "../HF_pidcontroller.hpp"

namespace hf {

    class YawPid : public PidController {

        private: 

            // Constants set in constructor ----------------------------

            float _Kp = 0;
            float _Ki = 0;
            float _windupMax = 0;
            float _rateMax = 0;

            virtual void modifyDemands(float * state, float * demands) override
            {
                static float _errorI;

                // Compute error as difference between yaw demand and angular velocity
                float error = demands[DEMANDS_YAW] - state[State::DPSI];

                // Reset integral on quick angular velocity change
                if (fabs(error) > _rateMax) {
                    _errorI = 0;
                }

                // Compute I term
                _errorI = Filter::constrainAbs(_errorI + error, _windupMax);

                // Adjust yaw demand based on error
                demands[DEMANDS_YAW] = _Kp * error + _Ki * _errorI;
             }

        public:

            YawPid(const float Kp,
                   const float Ki,
                   const float windupMax=6.0,
                   const float rateMaxDegreesPerSecond=40) 
            {
                _Kp = Kp;
                _Ki = Ki;
                _windupMax = windupMax;
                _rateMax = Filter::deg2rad(rateMaxDegreesPerSecond);
            }

    };  // class YawPid

} // namespace hf
