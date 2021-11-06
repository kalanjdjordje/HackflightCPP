{--
  PID controller for altitude hold

  Copyright(C) 2021 Simon D.Levy

  MIT License
--}

{-# LANGUAGE RebindableSyntax #-}

module AltHoldPid

where

import Language.Copilot

import PidController
import State
import Demands
import Utils(constrain_abs, in_band)


altHoldController :: Stream Float
                  -> Stream Float
                  -> Stream Float
                  -> Stream Float
                  -> Stream Float 
                  -> PidController

altHoldController kp ki windupMax pilotVelZMax stickDeadband = 
    makePidController (altHoldFun kp ki windupMax pilotVelZMax stickDeadband)


altHoldFun :: Stream Float
           -> Stream Float
           -> Stream Float
           -> Stream Float
           -> Stream Float
           -> PidFun

altHoldFun kp
           ki
           windupMax
           pilotVelZMax
           stickDeadband
           state
           demands =

    Demands (error' * kp + errorIntegral * ki ) 0 0 0

    where

       -- NED => ENU
       altitude = -(z state)

       throttleDemand = throttle demands

       -- inband = in_band throttleDemand stickDeadband
       inband = in_band throttleDemand stickDeadband

       -- Reset controller when moving into deadband
       altitudeTarget = if inband && not (in_band throttleDemand' stickDeadband)
                        then altitude
                        else altitudeTarget'

       targetVelocity = if inband
                        then altitudeTarget - altitude
                        else pilotVelZMax * throttleDemand

       -- Compute error as altTarget velocity minus actual velocity, after
       -- negating actual to accommodate NED
       error' = targetVelocity + (dz state)

       -- Accumualte error integral
       errorIntegral = constrain_abs (errorIntegral' + error') windupMax

       -- Maintain controller state between calls
       errorIntegral' = [0] ++ errorIntegral
       altitudeTarget' = [0] ++ altitudeTarget
       throttleDemand' = [0] ++ throttleDemand
