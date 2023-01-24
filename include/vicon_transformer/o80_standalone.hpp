// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief o80 Standalone implementation for Vicon system.
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <o80/standalone.hpp>
#include <o80/void_state.hpp>

#include "o80_driver.hpp"
#include "types.hpp"

namespace vicon_transformer
{
constexpr int STANDALONE_QUEUE_SIZE = 50000;
//! Number of actuators (zero, as the Vicon system does not have actuation).
constexpr int STANDALONE_N_ACTUATORS = 0;

/**
 * @brief o80 standalone over the @ref o80Driver.
 *
 * The Vicon system is a passive sensor without actuation, so the number of
 * actuators is zero and o80::VoidState is used for the states (i.e. one cannot
 * send commands to the system).
 * All data provided by Vicon is written to the extended state.
 *
 * @tparam Driver An actual implementation of @ref o80Driver.
 */
template <typename Driver>
class o80Standalone
    : public o80::Standalone<STANDALONE_QUEUE_SIZE,
                             STANDALONE_N_ACTUATORS,
                             Driver,
                             o80::VoidState,               // State
                             typename Driver::DRIVER_OUT>  // ExtendedState
{
public:
    using o80::Standalone<STANDALONE_QUEUE_SIZE,
                          STANDALONE_N_ACTUATORS,
                          Driver,
                          o80::VoidState,
                          typename Driver::DRIVER_OUT>::Standalone;

    o80::States<STANDALONE_N_ACTUATORS, o80::VoidState> convert(
        const typename Driver::DRIVER_OUT&) override
    {
        return o80::States<STANDALONE_N_ACTUATORS, o80::VoidState>();
    }

    None convert(
        const o80::States<STANDALONE_N_ACTUATORS, o80::VoidState>&) override
    {
        return None();
    }

    void enrich_extended_state(
        typename Driver::DRIVER_OUT& extended_state,
        const typename Driver::DRIVER_OUT& observation) override
    {
        extended_state = observation;
    }
};

}  // namespace vicon_transformer
