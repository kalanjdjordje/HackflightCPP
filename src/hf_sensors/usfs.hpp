/*
   Support for Pesky Products Unltimate Sensor Fusion Solution IMU

   Copyright (c) 2021 Simon D. Levy

   MIT License
   */

#pragma once

#include <HF_sensor.hpp>
#include <HF_filters.hpp>

#include <USFS_Master.h>

namespace hf {

    /*
       The most common aeronautical convention defines roll as acting about
       the longitudinal axis, positive with the starboard (right) wing
       down. Yaw is about the vertical body axis, positive with the nose to
       starboard. Pitch is about an axis perpendicular to the longitudinal
       plane of symmetry, positive nose up.

        https://en.wikipedia.org/wiki/Flight_dynamics_(fixed-wing_aircraft)

        https://emissarydrones.com/what-is-roll-pitch-and-yaw
    */

    class USFS : public Sensor {

        friend class HackflightFull;

        private:

            USFS_Master _usfs;

        protected:

            void begin(void)
            {
                // Start the USFS in master mode, no interrupt
                if (!_usfs.begin()) {
                    while (true) {
                        Serial.println(_usfs.getErrorString());
                        delay(100);
                    }
                }
            }

            virtual void modifyState(State * state, float time)
            {
                (void)time;

                _usfs.checkEventStatus();

                if (_usfs.gotError()) {
                    while (true) {
                        Serial.print("ERROR: ");
                        Serial.println(_usfs.getErrorString());
                    }
                }

                if (_usfs.gotGyrometer()) {

                    float gx=0, gy=0, gz=0;

                    // Returns degrees / sec
                    _usfs.readGyrometer(gx, gy, gz);

                    // Convert degrees / sec to radians / sec
                    state->x[State::DPHI] = radians(gx);
                    state->x[State::DTHETA] = radians(gy);
                    state->x[State::DPSI] = radians(gz);
                }

                if (_usfs.gotQuaternion()) {

                    float qw=0, qx=0, qy=0, qz=0;

                    _usfs.readQuaternion(qw, qx, qy, qz);

                    Filter::quat2euler(qw, qx, qy, qz, 
                            state->x[State::PHI], state->x[State::THETA], state->x[State::PSI]);

                    // Adjust rotation so that nose-up is positive
                    state->x[State::THETA] = -state->x[State::THETA];

                    // Convert heading from [-pi,+pi] to [0,2*pi]
                    if (state->x[State::PSI] < 0) {
                        state->x[State::PSI] += 2*M_PI;
                    }
                }

            } // modifyState

    };  // class USFS

} // namespace hf
