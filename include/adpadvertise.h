/*
  Copyright (c) 2014, J.D. Koftinoff Software, Ltd.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   3. Neither the name of J.D. Koftinoff Software, Ltd. nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "jdksavdecc_world.h"
#include "jdksavdecc_adp.h"

#define ADPADVERTISER_UNASSIGNED_IPV4 "0.0.0.0"
#define ADPADVERTISER_UNASSIGNED_IPV6 "0::0"
#define ADPADVERTISER_AVDECC_UDP_PORT "17221"
#define ADPADVERTISER_MDNS_MULTICAST_IPV4 "224.0.0.251"
#define ADPADVERTISER_MDNS_MULTICAST_IPV6 "ff02::fb"


/// adpadvertiser object manages scheduling and sending adp entity available messages
/// and responding to adp entity discover messages.
struct adpadvertiser {

    /// The current ADPDU that is sent
    struct jdksavdecc_adpdu adpdu;

    /// The system time in milliseconds that the last ADPDU was sent
    uint64_t last_time_in_ms;

    /// A flag to notify higher level code that the state machine is requesting an immediate tick again
    bool early_tick;

    /// A flag used internally to the state machine to trigger the sending of an entity available message immediately
    bool do_send_entity_available;

    /// The context that the adp advertiser is used in.
    void *context;

    /// The function that the adpadvertiser calls in order to send an ADPDU.
    void (*frame_send)(
        struct adpadvertiser *self,
        void *context,
        uint8_t const *buf,
        uint16_t len );

    /// The function that the adpadvertiser calls if it received an entity available or entity available
    /// for some other entity on the network.  May be set to 0 if the user does not care.
    void (*received_entity_available_or_departing)(
        struct adpadvertiser *self,
        void *context,
        struct jdksavdecc_adpdu *adpdu );
};

/// Initialize an adpadvertiser with the specified context and frame_send function and
/// received_entity_available_or_departing function
bool adpadvertiser_init(
    struct adpadvertiser *self,
    void *context,
    void (*frame_send)(
        struct adpadvertiser *self,
        void *context,
        uint8_t const *buf,
        uint16_t len ),
    void (*received_entity_available_or_departing)(
        struct adpadvertiser *self,
        void *context,
        struct jdksavdecc_adpdu *adpdu )
    );

void adpadvertiser_destroy(struct adpadvertiser *self );

bool adpadvertiser_receive(
    struct adpadvertiser *self,
    uint64_t time_in_milliseconds,
    uint8_t const *buf,
    uint16_t len );

void adpadvertiser_tick(
    struct adpadvertiser *self,
    uint64_t cur_time_in_ms );

void adpadvertiser_send_entity_available( struct adpadvertiser *self );

void adpadvertiser_send_entity_departing( struct adpadvertiser *self );

void adpadvertiser_send_entity_discover( struct adpadvertiser *self );

