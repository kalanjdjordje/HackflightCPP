/*
   Support for VL53L1X time-of-flight distance sensor

   Copyright (c) 2021 Simon D. Levy

   MIT License
   */

#pragma once

#include <HF_sensor.hpp>
#include <HF_filters.hpp>

#include <VL53L1X.h>

namespace hf {

    class Vl53l1xRangefinder : public Sensor {

        friend class Hackflight;

        private:

            VL53L1X _vl53l1x;

            // Low-pass filter for altitude
            // XXX Should we also filter the velocity?
            LowPassFilter _lpf;

            float _period = 0;

            // Support using temporal first different to compute State::DZ
            float _dist_curr = 0;
            float _time_prev = 0;

        protected:

            virtual void modifyState(State * state, float time) override
            {
                State * hfstate = (State *)state;

                hfstate->x[State::DZ] = (_dist_curr - hfstate->x[State::Z]) / _period;

                hfstate->x[State::Z] = _dist_curr;
            }

            virtual void begin(void) override
            {
                _vl53l1x.begin();

                _lpf.begin();
            }

            virtual bool ready(float time) override
            {
                // Sensor must have new data
                if (!_vl53l1x.newDataReady()) {
                    return false;
                }

                // Get distance from sensor, convert it to meters, and 
                // run it through the low-pass filter
                _dist_curr = _lpf.update(_vl53l1x.getDistance() / 1000.); // mm => m

                // Enough time must also elapse between readings
                if (time - _time_prev > _period) {
                    _time_prev = time;
                    return true;
                }

                return false;
            }

        public:

            Vl53l1xRangefinder(uint16_t freq=100)
            {

                _period = 1. / freq;

                _time_prev = 0;
            }


    };  // class Vl53l1xRangefinder 

} // namespace hf
