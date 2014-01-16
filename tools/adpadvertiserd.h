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

#include "jdksavdecc_adp_manager.h"
#include "us_rawnet_multi.h"
#include "us_allocator.h"
#include "us_getopt.h"
#include "us_logger_stdio.h"
#include "us_print.h"
#include "us_time.h"
#include "us_socket_collection.h"
#include "us_daemon.h"
#if US_ENABLE_SYSLOG==1
#include "us_logger_syslog.h"
#endif
#include "jdksavdecc_pdu_print.h"
#include "jdksavdecc_adp_print.h"
#include "jdksavdecc_print.h"

#ifndef ADPADVERTISERD_IDENTITY
#define ADPADVERTISERD_IDENTITY "adpadvertiserd"
#endif

void adpadvertiserd_message_readable(
        struct us_socket_collection_s *self,
        void * context,
        int fd,
        uint64_t current_time_in_milliseconds,
        struct sockaddr const *from_addr,
        socklen_t from_addrlen,
        uint8_t const *buf,
        ssize_t len );

void adpadvertiserd_frame_send(
    struct jdksavdecc_adp_manager *self,
    void *context,
    uint8_t const *buf,
    uint16_t len );

void adpadvertiserd_initialize_udp_sockets_on_port(
    us_socket_collection_t *self,
    const char *port_name,
    struct sockaddr *addr );

bool adpadvertiserd_is_network_port_interesting( struct ifaddrs *port );

void adpadvertiserd_initialize_udp_sockets_on_all_ports(
    us_socket_collection_t *self );

void adpadvertiserd_initialize_sockets_on_port(
    us_socket_collection_group_t *self,
    const char *port_name );

void adpadvertiserd_initialize_sockets(
    us_socket_collection_group_t *self );

void adpadvertiserd_initialize_entity_info(
    struct jdksavdecc_adpdu *adpdu );

void adpadvertiserd_receive_entity_available_or_departing(
    struct jdksavdecc_adp_manager *self,
    void *context,
    void const *source_address,
    int source_address_len,
    struct jdksavdecc_adpdu *adpdu );
    
bool adpadvertiserd_process_options(
    const char **argv );






