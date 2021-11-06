/*
   Hackflight class supporting safety checks and serial communication

   Copyright (c) 2021 Simon D. Levy

   MIT License
 */

#pragma once

#include "hf_boards/realboard.hpp"
#include "HF_pure.hpp"
#include "HF_serialtask.hpp"

namespace hf {

    class HackflightFull : public HackflightPure {

        private:

            // Safety
            bool _safeToArm = false;

            // Serial tasks
            SerialTask * _serial_tasks[10] = {};
            uint8_t _serial_task_count = 0;

            void checkSafety(State * state)
            {
                // Sync failsafe to open-loop-controller
                if (_receiver->lostSignal() && state->armed) {
                    _mixer->cut();
                    state->armed = false;
                    state->failsafe = true;
                    ((RealBoard *) _board)->showArmedStatus(false);
                    return;
                }

                // Disarm
                if (state->armed && !_receiver->inArmedState()) {
                    state->armed = false;
                } 

                // Avoid arming when controller is in armed state
                if (!_safeToArm) {
                    _safeToArm = !_receiver->inArmedState();
                }

                // Arm after lots of safety checks
                if (_safeToArm
                    && !state->armed
                    && !state->failsafe 
                    && state->safeToArm()
                    && _receiver->inactive()
                    && _receiver->inArmedState()
                    ) {
                    state->armed = true;
                }

                // Cut motors on inactivity
                if (state->armed && _receiver->inactive()) {
                    _mixer->cut();
                }

                // Set LED based on arming status
                ((RealBoard *)_board)->showArmedStatus(state->armed);

            } // checkSafety

            void startSensors(void) 
            {
                for (uint8_t k=0; k<_sensor_count; ++k) {
                    _sensors[k]->begin();
                }
            }

        public:

            HackflightFull(RealBoard * board, Receiver * receiver, Mixer * mixer)
                : HackflightPure(board, receiver, mixer)
            {
                _serial_task_count = 0;
            }

            void begin(void)
            {  
                _state.armed = false;

                // Start the board
                _board->begin();

                // Initialize the sensors
                startSensors();

                // Initialize the open-loop controller
                _receiver->begin();

                // Start the mixer
                _mixer->begin();

            } // begin

            void update(void)
            {
                HackflightPure::update();

                checkSafety(&_state);

                // Update serial tasks
                for (uint8_t k=0; k<_serial_task_count; ++k) {
                    _serial_tasks[k]->update(_board, _mixer, &_state);
                }
            }

            void addSerialTask(SerialTask * task)
            {
                _serial_tasks[_serial_task_count++] = task;

                task->init(_receiver, _mixer, &_state);
            }

    }; // class HackflightFull

} // namespace
