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



void adpadvertiserd_incoming_raw_packet_handler(
    us_rawnet_multi_t *self,
    int ethernet_port,
    void *context,
    uint8_t *buf,
    uint16_t len ) {
    struct timeval tv;
    us_gettimeofday(&tv);
    struct adpadvertiser *adv = (struct adpadvertiser *)context;
    (void)ethernet_port;

    if( buf[JDKSAVDECC_FRAME_HEADER_LEN+0]==JDKSAVDECC_1722A_SUBTYPE_ADP ) {
        adpadvertiser_receive(
            adv,
            &tv,
            buf+JDKSAVDECC_FRAME_HEADER_LEN,
            len-JDKSAVDECC_FRAME_HEADER_LEN );
    }
}


void adpadvertiserd_incoming_udp_packet_handler(
    struct adpadvertiser *adv,
    int fd) {
    uint8_t buf[1500];
    ssize_t len;
    struct sockaddr_storage source_addr;
    socklen_t source_addr_len = sizeof(struct sockaddr_storage);

    struct timeval tv;
    us_gettimeofday(&tv);

    len = recvfrom(fd,buf,sizeof(buf),0,(struct sockaddr*)&source_addr,&source_addr_len);

    if( len>0 ) {
        if( buf[JDKSAVDECC_FRAME_HEADER_LEN+0]==JDKSAVDECC_1722A_SUBTYPE_ADP ) {
            adpadvertiser_receive(
                adv,
                &tv,
                buf,
                len );
        }
    }
}


us_rawnet_multi_t rawnet;
struct adpadvertiser advertiser;
us_socket_collection_t udp_sockets;
us_socket_collection_t rawnet_sockets;

const char *port0_name = "en0";
const char *port1_name = "en1";
struct addrinfo *ipv4_multicast_addr;
struct addrinfo *ipv4_listen_addr;
struct addrinfo *ipv6_multicast_addr;
struct addrinfo *ipv6_listen_addr;

void adpadvertiserd_frame_send(
    struct adpadvertiser *self,
    void *context,
    uint8_t const *buf,
    uint16_t len ) {


}

#if 0
void adpadvertiserd_frame_send(
    struct adpadvertiser *self,
    void *context,
    uint8_t const *buf,
    uint16_t len )
{
    if( ipv4_socket0 != -1 ) {
        sendto(ipv4_socket0, buf, len, 0, ipv4_multicast_addr->ai_addr, ipv4_multicast_addr->ai_addrlen );
    }
    if( ipv4_socket1 != -1 ) {
        sendto(ipv4_socket1, buf, len, 0, ipv4_multicast_addr->ai_addr, ipv4_multicast_addr->ai_addrlen );
    }
    if( ipv6_socket0 != -1 ) {
        sendto(ipv6_socket0, buf, len, 0, ipv6_multicast_addr->ai_addr, ipv6_multicast_addr->ai_addrlen );
    }
    if( ipv6_socket1 != -1 ) {
        sendto(ipv6_socket1, buf, len, 0, ipv6_multicast_addr->ai_addr, ipv6_multicast_addr->ai_addrlen );
    }
    {
        uint8_t header[JDKSAVDECC_FRAME_HEADER_LEN];
        memcpy( &header[JDKSAVDECC_FRAME_HEADER_DA_OFFSET], jdksavdecc_multicast_adp_acmp.value, 6 );
        jdksavdecc_uint16_set(JDKSAVDECC_AVTP_ETHERTYPE,header,JDKSAVDECC_FRAME_HEADER_ETHERTYPE_OFFSET );
        us_rawnet_multi_send_all(&rawnet, header, JDKSAVDECC_FRAME_HEADER_LEN, buf, len, 0, 0);
    }
}
#endif

int main( int argc, const char **argv ) {
    us_rawnet_multi_open(&rawnet,JDKSAVDECC_AVTP_ETHERTYPE,jdksavdecc_multicast_adp_acmp.value, 0);
    // TODO: Parse args

    if( adpadvertiser_init(&advertiser,0,adpadvertiserd_frame_send) ) {
        int i;
        memset(&advertiser.adpdu,0,sizeof(advertiser.adpdu));
        jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.header.entity_id, 0x70b3d5ffffedcf0);
        jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.entity_model_id, 0x70b3d5edc0000000);
        advertiser.adpdu.header.valid_time = 10; /* 20 seconds */

        us_socket_collection_init(&udp_sockets);
        us_socket_collection_init(&rawnet_sockets);

        us_socket_collection_add_multicast_udp(&udp_sockets, "0.0.0.0", "17221", "224.0.0.251", "17221", port0_name, 0 );
        us_socket_collection_add_multicast_udp(&udp_sockets, "0::0", "17221", "ff02::fb", "17221", port0_name, 0 );

        for( i=0; i<rawnet.ethernet_port_count; ++i ) {
            us_socket_collection_add_rawnet(&rawnet_sockets, &rawnet.ethernet_ports[i]);
        }

        while(true) {
            uint64_t cur_time;

            if( us_platform_sigint_seen || us_platform_sigterm_seen ) {
                break;
            }

            {
                struct timeval tv;
                us_gettimeofday(&tv);
                cur_time = ((uint64_t)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
            }

            adpadvertiser_tick( &advertiser, (uint64_t)cur_time*1000 );

            us_socket_collection_tick(&udp_sockets,cur_time);
            us_socket_collection_tick(&rawnet_sockets,cur_time);
            us_socket_collection_cleanup(&rawnet_sockets);
            us_socket_collection_cleanup(&udp_sockets);

#if defined(WIN32)
            Sleep(100);
#else
            fd_set readable;
            fd_set writable;
            int largest_fd=-1;
            int s;
            struct timeval tv;
            FD_ZERO(&readable);
            largest_fd=us_rawnet_multi_set_fdset(&rawnet, &readable);

            largest_fd = us_socket_collection_fill_read_set(&udp_sockets, &readable, largest_fd );
            largest_fd = us_socket_collection_fill_write_set(&udp_sockets, &writable, largest_fd );

            largest_fd = us_socket_collection_fill_read_set(&rawnet_sockets, &readable, largest_fd );
            largest_fd = us_socket_collection_fill_write_set(&rawnet_sockets, &writable, largest_fd );

            tv.tv_sec = 0;
            tv.tv_usec = 200000; // 200 ms

            if( udp_sockets.do_early_tick ) {
                tv.tv_usec = 0;
                udp_sockets.do_early_tick = false;
            }

            if( rawnet_sockets.do_early_tick ) {
                tv.tv_usec = 0;
                rawnet_sockets.do_early_tick = false;
            }

            do {
                s=select(largest_fd+1, &readable, &writable, 0, &tv );
            } while( s<0 && (errno==EINTR || errno==EAGAIN) );

            if( s<0 ) {
                us_log_error( "Unable to select" );
                break;
            }
#endif

#if 0
            // poll even if select thinks there are no readable sockets
            us_rawnet_multi_rawnet_poll_incoming(
                &rawnet,
                cur_time,
                128,
                &advertiser,
                adpadvertiserd_incoming_raw_packet_handler );
#endif

            us_socket_collection_handle_readable_set(&udp_sockets,&readable,cur_time);
            us_socket_collection_handle_writable_set(&udp_sockets,&writable,cur_time);
            us_socket_collection_handle_readable_set(&rawnet_sockets,&readable,cur_time);
            us_socket_collection_handle_writable_set(&rawnet_sockets,&writable,cur_time);

        }


        adpadvertiser_destroy(&advertiser);
    }
    return 0;
}

