/*
 * detumbling.cpp
 *
 *  Created on: 25 Jan 2017
 *      Author: weclewski
 */

/* MATLAB:
 % ============================================================================
 %
 %                           detumbling procedure
 %
 % This procedure calculates the commanded magnetic dipole for the coils in
 % the detumbling mode based on the B-Dot control law. A high-pass filter for
 % the magnetic field time derivative is included.
 % NOTE: Current magnetic field time derivative output (mtmDot) is the input
 % in the next iteration (mtmDotPrev). Initialize the previous magnetic field
 % time derivative with 0.
 %
 % Author: Pawel Jaworski
 %         pawel.jaworski0@wp.pl
 % Date:   december 2016
 %
 %   inputs        :
 %       mtmMeas                  - [3x1], magnetometer measurement, [Gauss]
 %       mtmMeasPrev              - [3x1], previous (one iteration back) magnetometer measurement, [Gauss]
 %       mtmDotPrev               - [3x1], previous (one iteration back) magnetic field time derivative, [Gauss/s]
 %
 %   outputs       :
 %       commDipoleBdot           - [3x1], commanded magnetic dipole, [A m^2]
 %       mtmDot                   - [3x1], magnetic field time derivative, [Gauss/s]
 %
 %   globals       :
 %       ADCSParameters.DetumblingConst.dt       - [1x1], iteration time step, [s]
 %       ADCSParameters.DetumblingConst.wCutOff  - [1x1], high-pass filter cut off frequency, [rad/s]
 %       ADCSParameters.DetumblingConst.bDotGain - [1x1], B-dot gain, [kg m^2 / s]
 %       ADCSParameters.DetumblingConst.coilsOn  - boolean, [3x1], active magnetic coils
 %
 %   locals        :
 %
 %   subfunctions  :
 %       vectorNorm               - calculates [3x1] vector norm
 %
 %   references    :
 %
 %
 % ============================================================================
 C:

 Lsb of mtmMeas in int32 representation is 1e-9 [T] = 1e-5 [Gauss]
 */

#include "detumbling.hpp"
#include <system.h>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace adcs
{

Detumbling::Detumbling()
{
    //empty
}

void Detumbling::InitializeDetumbling(DetumblingState& state,
        const DetumblingParameters& parameters)
{
    UNUSED1(parameters);
    //Set the previous time derivative of the magnetic field to zeros.
    state.mtmDotPrevInMemory = RowVector3i::Zero();
    //Set the previous MTM measurement to zeros,
    state.mtmMeasPrevInMemory = RowVector3i::Zero();
}

void Detumbling::DoDetumbling(DipoleVec& dipole, const MagVec& magnetometer,
        DetumblingState& state)
{
    UNUSED(dipole, magnetometer, state);
}

}

