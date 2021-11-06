/*
   Timer task for serial comms

   MIT License
 */

#pragma once

#include "HF_parser.hpp"
#include "HF_timertask.hpp"
#include "HF_state.hpp"
#include "HF_receiver.hpp"
#include "HF_mixer.hpp"
#include "hf_boards/realboard.hpp"

namespace hf {

    class SerialTask : public TimerTask, Parser {

        friend class HackflightFull;

        private:

            uint8_t _payload[128] = {};

            // Store so we don't have to pass them on update
            State * _state = NULL;
            Receiver * _receiver = NULL;
            Mixer * _mixer = NULL;

             void handle_RECEIVER_Request(float & c1, float & c2, float & c3, float & c4, float & c5, float & c6)
            {
                c1 = _receiver->getRawval(0);
                c2 = _receiver->getRawval(1);
                c3 = _receiver->getRawval(2);
                c4 = _receiver->getRawval(3);
                c5 = _receiver->getRawval(4);
                c6 = _receiver->getRawval(5);
             }

            void handle_STATE_Request(float & x, float & dx, float & y, float & dy, float & z, float & dz, float & phi, float & dphi, float & theta, float & dtheta, float & psi, float & dpsi)
            {
                x = _state->x[State::X];
                dx = _state->x[State::DX];
                y = _state->x[State::Y];
                dy = _state->x[State::DY];
                z = _state->x[State::Z];
                dz = _state->x[State::DZ];
                phi = _state->x[State::PHI];
                dphi = _state->x[State::DPHI];
                theta = _state->x[State::THETA];
                dtheta = _state->x[State::DTHETA];
                psi = _state->x[State::PSI];
                dpsi = _state->x[State::DPSI];
            }

            void handle_ACTUATOR_TYPE_Request(uint8_t & mtype)
            {
                mtype = _mixer->getType();
            }

            void handle_SET_MOTOR(float  m1, float  m2, float  m3, float  m4)
            {
                _mixer->setMotorDisarmed(0, m1);
                _mixer->setMotorDisarmed(1, m2);
                _mixer->setMotorDisarmed(2, m3);
                _mixer->setMotorDisarmed(3, m4);
            }

        protected:

            virtual void collectPayload(uint8_t index, uint8_t value) override
            {
                _payload[index] = value;
            }

            virtual void dispatchMessage(uint8_t command) override
            {
                switch (command) {

                    case 121:
                        {
                            float c1 = 0;
                            float c2 = 0;
                            float c3 = 0;
                            float c4 = 0;
                            float c5 = 0;
                            float c6 = 0;
                            handle_RECEIVER_Request(c1, c2, c3, c4, c5, c6);
                            prepareToSendFloats(command, 6);
                            sendFloat(c1);
                            sendFloat(c2);
                            sendFloat(c3);
                            sendFloat(c4);
                            sendFloat(c5);
                            sendFloat(c6);
                            completeSend();
                        } break;

                    case 122:
                        {
                            float x = 0;
                            float dx = 0;
                            float y = 0;
                            float dy = 0;
                            float z = 0;
                            float dz = 0;
                            float phi = 0;
                            float dphi = 0;
                            float theta = 0;
                            float dtheta = 0;
                            float psi = 0;
                            float dpsi = 0;
                            handle_STATE_Request(x, dx, y, dy, z, dz,
                                    phi, dphi, theta, dtheta, psi, dpsi);
                            prepareToSendFloats(command, 12);
                            sendFloat(x);
                            sendFloat(dx);
                            sendFloat(y);
                            sendFloat(dy);
                            sendFloat(z);
                            sendFloat(dz);
                            sendFloat(phi);
                            sendFloat(dphi);
                            sendFloat(theta);
                            sendFloat(dtheta);
                            sendFloat(psi);
                            sendFloat(dpsi);
                            completeSend();
                        } break;

                    case 123:
                        {
                            uint8_t mtype = 0;
                            handle_ACTUATOR_TYPE_Request(mtype);
                            prepareToSendBytes(command, 1);
                            sendByte(mtype);
                            completeSend();
                        } break;

                    case 215:
                        {
                            float m1 = 0;
                            memcpy(&m1,  &_payload[0], sizeof(float));

                            float m2 = 0;
                            memcpy(&m2,  &_payload[4], sizeof(float));

                            float m3 = 0;
                            memcpy(&m3,  &_payload[8], sizeof(float));

                            float m4 = 0;
                            memcpy(&m4,  &_payload[12], sizeof(float));

                            handle_SET_MOTOR(m1, m2, m3, m4);
                        } break;

                } // switch (_command)

            } // dispatchMessage 

            void init( Receiver * receiver, Mixer * mixer, State * state)
            {
                _receiver = receiver;
                _mixer = mixer;
                _state = state;
            }
    public:

            static constexpr float FREQ = 66;

            bool _useTelemetryPort = false;

            SerialTask(bool secondaryPort=false)
                : TimerTask(FREQ)
            {
                _useTelemetryPort = secondaryPort;
            }

            void update(Board * board, Mixer * mixer, State * state)
            {
                if (!TimerTask::ready(board)) {
                    return;
                }

                RealBoard * realboard = (RealBoard *)board;

                while (realboard->serialAvailable(_useTelemetryPort) > 0) {
                    Parser::parse(realboard->serialRead(_useTelemetryPort));
                }

                while (Parser::availableBytes() > 0) {
                    realboard->serialWrite(Parser::readByte(),
                                           _useTelemetryPort);
                }

                // Support motor testing from GCS
                if (!state->armed) {
                    mixer->runDisarmed();
                }
            }

        }; // class SerialTask

} // namespace hf
