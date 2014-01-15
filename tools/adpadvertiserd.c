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

#include "adpadvertiserd.h"


us_rawnet_multi_t rawnet;
struct adpadvertiser advertiser;
us_socket_collection_t udp_sockets;
us_socket_collection_t rawnet_sockets;
us_socket_collection_group_t sockets;

const char *port0_name = "en0";
const char *port1_name = "en5";


void adpadvertiserd_message_readable(
        struct us_socket_collection_s *self,
        void * context,
        int fd,
        uint64_t current_time_in_milliseconds,
        struct sockaddr const *from_addr,
        socklen_t from_addrlen,
        uint8_t const *buf,
        ssize_t len ) {

    struct adpadvertiser *adv = (struct adpadvertiser *)self->user_context;
    if( len>0 ) {
        if( buf[0]==JDKSAVDECC_1722A_SUBTYPE_ADP ) {
            adpadvertiser_receive(
                adv,
                current_time_in_milliseconds,
                buf,
                len );
        }
    }

}

void adpadvertiserd_frame_send(
    struct adpadvertiser *self,
    void *context,
    uint8_t const *buf,
    uint16_t len ) {
    int i;
    us_socket_collection_group_t *g = (us_socket_collection_group_t *)&sockets;
    for ( i=0; i<g->num_collections; ++i ) {
        us_socket_collection_t *c = g->collection[i];
        int j;
        for (j=0; j<c->num_sockets; ++j ) {
            int fd = c->socket_fd[j];
            void *socket_context = c->socket_context[j];
            if( fd!=-1 && c->send_data!=0) {
                ssize_t sent = c->send_data( c, socket_context, fd, 0, 0, buf, len );
                if( sent<0 ) {
                    fprintf(stdout, "errno = %d (%s)\n", errno, strerror(errno) );
                }
                fprintf( stdout, "Sent %d to fd %d\n", (int)sent, fd );
            }
        }
    }
}


void adpadvertiserd_initialize_sockets_on_port(
    us_socket_collection_group_t *self,
    const char *port_name ) {

   if( port_name && *port_name) {
        us_socket_collection_add_multicast_udp(
            &udp_sockets,
            UNASSIGNED_IPV4, AVDECC_UDP_PORT,       // The address and port to listen on
            MDNS_MULTICAST_IPV4, AVDECC_UDP_PORT,   // The multicast group to join
            port_name,                              // the ethernet device to use
            us_net_get_addrinfo(MDNS_MULTICAST_IPV4, AVDECC_UDP_PORT, SOCK_DGRAM, false ) // The default address to send to
            );

        us_socket_collection_add_multicast_udp(
            &udp_sockets,
            UNASSIGNED_IPV6, AVDECC_UDP_PORT,       // The address and port to listen on
            MDNS_MULTICAST_IPV6, AVDECC_UDP_PORT,   // The multicast group to join
            port_name,                              // the ethernet device to use
            us_net_get_addrinfo(MDNS_MULTICAST_IPV6, AVDECC_UDP_PORT, SOCK_DGRAM, false ) // The default address to send to
            );
    }

}

void adpadvertiserd_initialize_sockets(
    us_socket_collection_group_t *self,
    const char *port0_name,
    const char *port1_name ) {
    int i;
    us_socket_collection_init_udp_multicast(&udp_sockets);
    udp_sockets.readable = adpadvertiserd_message_readable;
    udp_sockets.user_context = &advertiser;

    adpadvertiserd_initialize_sockets_on_port( &sockets, port0_name );
    adpadvertiserd_initialize_sockets_on_port( &sockets, port1_name );

    // Create the rawnet sockets collection and add all the sockets that the rawnet_multi found for us.
    us_rawnet_multi_open(&rawnet,JDKSAVDECC_AVTP_ETHERTYPE,jdksavdecc_multicast_adp_acmp.value, 0);

    us_socket_collection_init_rawnet(&rawnet_sockets);
    rawnet_sockets.readable = adpadvertiserd_message_readable;
    rawnet_sockets.user_context = &advertiser;

    for( i=0; i<rawnet.ethernet_port_count; ++i ) {
        us_rawnet_context_t *c = &rawnet.ethernet_ports[i];
        us_socket_collection_add_rawnet(
            &rawnet_sockets,
            c);
    }

    // Create the socket collection group that encapsulates all the sockets
    us_socket_collection_group_init(self);
    us_socket_collection_group_add(self,&udp_sockets);
    us_socket_collection_group_add(self,&rawnet_sockets);
}

void adpadvertiserd_initialize_entity_info( struct jdksavdecc_adpdu *adpdu ) {
    jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.header.entity_id, 0x70b3d5ffffedcf00);
    jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.entity_model_id,  0x70b3d5edc0000000);
    advertiser.adpdu.header.valid_time = 10; /* 20 seconds */
}

int main( int argc, const char **argv ) {
    // TODO: Parse args

    if( adpadvertiser_init(&advertiser,0,adpadvertiserd_frame_send) ) {
        adpadvertiserd_initialize_sockets( &sockets, port0_name, port1_name );
        advertiser.context = &sockets;
        adpadvertiserd_initialize_entity_info(&advertiser.adpdu);

        while(us_socket_collection_group_count_sockets(&sockets)>0) {
            uint64_t cur_time = us_time_in_milliseconds();

            if( us_platform_sigint_seen || us_platform_sigterm_seen ) {
                break;
            }

            adpadvertiser_tick( &advertiser, (uint64_t)cur_time );
            us_socket_collection_group_tick(&sockets,cur_time);

            if( !us_socket_collections_group_select(
                    &sockets,
                    cur_time,
                    advertiser.early_tick ? 0 : 200) ) {
                break;
            }
        }

        adpadvertiser_destroy(&advertiser);
        us_socket_collection_group_destroy(&sockets);
    }
    return 0;
}

