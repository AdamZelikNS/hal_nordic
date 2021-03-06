/*
 * Copyright (c) 2020, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**@file nrf_802154_callouts.h
 * @brief Provides function prototypes required by nRF 802.15.4 Radio Diver
 *
 * Functions whose prototypes are defined in this file are to be implemented
 * by an application using the nRF 802.15.4 Radio Driver.
 */

/**
 * @defgroup nrf_802154_callouts
 * 802.15.4 radio driver callouts
 * @{
 *
 */

#ifndef NRF_802154_CALLOUTS_H_
#define NRF_802154_CALLOUTS_H_

#include <stdint.h>
#include <stdbool.h>

#include "nrf_802154_types.h"

/**
 * @brief Notifies that the CCA procedure has finished.
 *
 * @param[in]  channel_free  Indication if the channel is free.
 */
extern void nrf_802154_cca_done(bool channel_free);

/**
 * @brief Notifies that the CCA procedure failed.
 *
 * @param[in]  error  Reason of the failure.
 */
extern void nrf_802154_cca_failed(nrf_802154_cca_error_t error);

/**
 * @brief Notifies that the energy detection procedure finished.
 *
 * @note This function passes the EnergyLevel defined in the 802.15.4-2006 specification:
 *       0x00 - 0xff, proportionally to the detected energy level (dBm above receiver sensitivity).
 *       To calculate the result in dBm, use @ref nrf_802154_dbm_from_energy_level_calculate.
 *
 * @param[in]  result  Maximum energy detected during the energy detection procedure.
 */
extern void nrf_802154_energy_detected(uint8_t result);

/**
 * @brief Notifies that the energy detection procedure failed.
 *
 * @param[in]  error  Reason of the failure.
 */
extern void nrf_802154_energy_detection_failed(nrf_802154_ed_error_t error);

/**
 * @brief Notifies about the start of the ACK frame transmission.
 *
 * @note If a call to this function is performed by the same CPU running core
 * of nRF 802.15.4 Radio Driver (non serialized call to @ref nrf_802154_tx_ack_started)
 * the function must be very short to prevent dropping frames by the driver.
 * If a call to this function is performed by a CPU through a serialization layer
 * the call can be slightly delayed. The call can happen even after an ACK frame
 * is fully transmitted. It is guaranteed that a call to @ref nrf_802154_tx_ack_started
 * occurs before a call to @ref nrf_802154_received_timestamp_raw related to
 * the same received frame.
 *
 * @param[in]  p_data  Pointer to a buffer with PHR and PSDU of the ACK frame.
 */
extern void nrf_802154_tx_ack_started(const uint8_t * p_data);

/**
 * @brief Notifies that a frame was received at a given time.
 *
 * This function works like @ref nrf_802154_received_raw and adds a timestamp to the parameter
 * list.
 *
 * @note The received frame usually contains a timestamp. However, due to a race condition,
 *       the timestamp may be invalid. This erroneous situation is indicated by
 *       the @ref NRF_802154_NO_TIMESTAMP value of the @p time parameter.
 *
 * @param[in]  p_data  Pointer to a buffer that contains PHR and PSDU of the received frame.
 *                     The first byte in the buffer is the length of the frame (PHR). The following
 *                     bytes contain the frame itself (PSDU). The length byte (PHR) includes FCS.
 *                     FCS is already verified by the hardware and may be modified by the hardware.
 * @param[in]  power   RSSI of the received frame.
 * @param[in]  lqi     LQI of the received frame.
 * @param[in]  time    Timestamp taken when the last symbol of the frame was received, in
 *                     microseconds (us), or @ref NRF_802154_NO_TIMESTAMP if the timestamp
 *                     is invalid.
 */
extern void nrf_802154_received_timestamp_raw(uint8_t * p_data,
                                              int8_t    power,
                                              uint8_t   lqi,
                                              uint32_t  time);

/**
 * @brief Notifies that the reception of a frame failed.
 *
 * @param[in]  error  Error code that indicates the reason of the failure.
 */
extern void nrf_802154_receive_failed(nrf_802154_rx_error_t error);

/**
 * @brief Notifies that a frame was transmitted.
 *
 * @note If ACK was requested for the transmitted frame, this function is called after a proper ACK
 *       is received. If ACK was not requested, this function is called just after transmission has
 *       ended.
 * @note The buffer pointed to by @p p_ack is not modified by the radio driver (and cannot be used
 *       to receive a frame) until @ref nrf_802154_buffer_free_raw is called.
 * @note The buffer pointed to by @p p_ack may be modified by the function handler (and other
 *       modules) until @ref nrf_802154_buffer_free_raw is called.
 *
 * @param[in]  p_frame  Pointer to a buffer that contains PHR and PSDU of the transmitted frame.
 * @param[in]  p_ack    Pointer to a buffer that contains PHR and PSDU of the received ACK.
 *                      The first byte in the buffer is the length of the frame (PHR). The following
 *                      bytes contain the ACK frame itself (PSDU). The length byte (PHR) includes
 *                      FCS. FCS is already verified by the hardware and may be modified by the
 *                      hardware. If ACK was not requested, @p p_ack is set to NULL.
 * @param[in]  power    RSSI of the received frame or 0 if ACK was not requested.
 * @param[in]  lqi      LQI of the received frame or 0 if ACK was not requested.
 */
extern void nrf_802154_transmitted_raw(const uint8_t * p_frame,
                                       uint8_t       * p_ack,
                                       int8_t          power,
                                       uint8_t         lqi);

/**
 * @brief Notifies that a frame was not transmitted due to a busy channel.
 *
 * This function is called if the transmission procedure fails.
 *
 * @param[in]  p_frame  Pointer to a buffer that contains PHR and PSDU of the frame that was not
 *                      transmitted.
 * @param[in]  error    Reason of the failure.
 */
extern void nrf_802154_transmit_failed(const uint8_t       * p_frame,
                                       nrf_802154_tx_error_t error);

#endif /* NRF_802154_CALLOUTS_H_ */

/** @} */
