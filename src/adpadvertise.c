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

#include "adpadvertise.h"

#include "us_world.h"
#include "us_net.h"
#include "us_rawnet.h"
#include "us_getopt.h"
#include "jdksavdecc_world.h"
#include "jdksavdecc_pdu.h"
#include "jdksavdecc_util.h"


bool adpadvertiser_init(
    struct adpadvertiser *self,
    void *context,
    void (*frame_send)( struct adpadvertiser *self, void *context, uint8_t const *buf, uint16_t len )
    ) {
    self->last_time_in_ms = 0;
    self->early_tick = true;
    self->do_send_entity_available = true;
    self->context = context;
    self->frame_send = frame_send;
    self->received_entity_available_or_departing = 0;

    memset(&self->adpdu, 0, sizeof(self->adpdu));
    self->adpdu.header.cd = 1;
    self->adpdu.header.subtype = JDKSAVDECC_SUBTYPE_ADP;
    self->adpdu.header.version = 0;
    self->adpdu.header.sv = 0;
    self->adpdu.header.message_type = JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_AVAILABLE;
    self->adpdu.header.valid_time = 10;
    self->adpdu.header.control_data_length = JDKSAVDECC_ADPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;

    return true;
}

void adpadvertiser_destroy(
    struct adpadvertiser *self ) {
}

bool adpadvertiser_receive(
    struct adpadvertiser *self,
    uint64_t time_in_milliseconds,
    uint8_t const *buf,
    uint16_t len ) {
    struct jdksavdecc_adpdu incoming;
    bool r=false;
    if( jdksavdecc_adpdu_read(&incoming, buf, 0, len)>0 ) {
        r=true;
        switch(incoming.header.message_type) {
        case JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER:
            if( jdksavdecc_eui64_compare(&incoming.header.entity_id, &self->adpdu.header.entity_id )
                || jdksavdecc_eui64_convert_to_uint64(&incoming.header.entity_id)==0 ) {
                self->do_send_entity_available = true;
                self->early_tick = true;
            }
            break;
        case JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_AVAILABLE:
            if( self->received_entity_available_or_departing ) {
                self->received_entity_available_or_departing( self, self->context, &incoming );
            }
            break;
        case JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DEPARTING:
            if( self->received_entity_available_or_departing ) {
                self->received_entity_available_or_departing( self, self->context, &incoming );
            }
            break;
        default:
            break;
        }
    }
    return r;
}


void adpadvertiser_tick(
    struct adpadvertiser *self,
    uint64_t cur_time_in_ms ) {
    uint64_t next_time_in_ms = self->last_time_in_ms + (self->adpdu.header.valid_time*1000)/2;
    if( (cur_time_in_ms > next_time_in_ms) || (self->early_tick && self->do_send_entity_available) ) {
        self->early_tick = false;
        self->last_time_in_ms = cur_time_in_ms;
        adpadvertiser_send_entity_available(self);
        self->do_send_entity_available = false;
    }
}


void adpadvertiser_send_entity_available( struct adpadvertiser *self ) {
    uint8_t buf[128];
    ssize_t len;
    self->adpdu.header.message_type = JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_AVAILABLE;
    len = jdksavdecc_adpdu_write(&self->adpdu, buf, 0, sizeof(buf) );

    if( len>0 ) {
        self->frame_send( self, self->context, buf, (uint16_t)len );
        self->adpdu.available_index++;
    }
}


void adpadvertiser_send_entity_departing( struct adpadvertiser *self ) {
    uint8_t buf[128];
    ssize_t len;
    self->adpdu.header.message_type = JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DEPARTING;
    len = jdksavdecc_adpdu_write(&self->adpdu, buf, 0, sizeof(buf) );

    if( len>0 ) {
        self->frame_send( self, self->context, buf, (uint16_t)len );
        self->adpdu.available_index++;
    }
}

void adpadvertiser_send_entity_discover( struct adpadvertiser *self ) {
    uint8_t buf[128];
    ssize_t len;
    struct jdksavdecc_adpdu adpdu;
    memset( &adpdu, 0, sizeof(adpdu) );
    adpdu.header.cd = 1;
    adpdu.header.subtype = JDKSAVDECC_SUBTYPE_ADP;
    adpdu.header.control_data_length = JDKSAVDECC_ADPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    adpdu.header.message_type = JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER;
    adpdu.header.sv = 0;
    adpdu.header.version = 0;
    adpdu.header.valid_time = 0;
    jdksavdecc_eui64_init_from_uint64(&adpdu.header.entity_id, 0 );

    len = jdksavdecc_adpdu_write(&adpdu, buf, 0, sizeof(buf) );

    if( len>0 ) {
        self->frame_send( self, self->context, buf, (uint16_t)len );
        self->adpdu.available_index++;
    }
}



