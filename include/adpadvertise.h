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

struct adpadvertiser {
    struct jdksavdecc_adpdu adpdu;
    uint64_t last_time_in_ms;
    bool early_tick;
    bool do_send_entity_available;
    void *context;
    void (*frame_send)( struct adpadvertiser *self, void *context, uint8_t const *buf, uint16_t len );
};

bool adpadvertiser_init(
    struct adpadvertiser *self,
    void *context,
    void (*frame_send)( struct adpadvertiser *self, void *context, uint8_t const *buf, uint16_t len )
    );

void adpadvertiser_destroy(struct adpadvertiser *self );

bool adpadvertiser_receive(
    struct adpadvertiser *self,
    struct timeval const *tv,
    uint8_t const *buf,
    uint16_t len );

void adpadvertiser_tick(
    struct adpadvertiser *self,
    uint64_t cur_time_in_ms );

void adpadvertiser_send_entity_available( struct adpadvertiser *self );

