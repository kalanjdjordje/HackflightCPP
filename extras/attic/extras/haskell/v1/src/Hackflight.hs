{--
  Hackflight core algorithm

  Copyright(C) 2021 Simon D.Levy

  MIT License
--}

module Hackflight(HackflightFun, hackflight)
where

import Demands
import Receiver(Receiver, getDemands)
import Sensor(Sensor, modifyState)
import Mixer(Mixer, getMotors)
import Motor(Motors)
import PidControllers
import VehicleState(zeroVehicleState)

type HackflightFun = Receiver ->
                     [Sensor] ->
                     [PidController] ->
                     Mixer ->
                     (Motors, [PidController])

hackflight :: HackflightFun

hackflight receiver sensors pidControllers mixer =

    let 

        -- Inject the receiver demands into the PID controllers
        pidControllers' = map (\p -> (PidController (pidFun p) 
                                                    (pidState p)
                                                    (getDemands receiver)))
                              pidControllers

        -- Get the vehicle state by running the sensors
        vehicleState = foldr modifyState zeroVehicleState sensors

         -- Map the PID update function to the pid controllers
        pidControllers'' = map (pidUpdate vehicleState) pidControllers'

        -- Sum over the list of demands to get the final demands
        demands = foldr addDemands zeroDemands (map pidDemands pidControllers'')

    -- Send the final demands to the mixer, returning the resulting motor
    -- values and the new PID controller states
    in ((getMotors mixer demands), pidControllers'')
