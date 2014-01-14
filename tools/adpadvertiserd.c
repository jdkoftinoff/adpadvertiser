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
const char *port0_name = "en0";
const char *port1_name = "en1";
struct addrinfo *ipv4_multicast_addr;
struct addrinfo *ipv4_listen_addr;
int ipv4_socket0;
int ipv4_socket1;
struct addrinfo *ipv6_multicast_addr;
struct addrinfo *ipv6_listen_addr;
int ipv6_socket0;
int ipv6_socket1;


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


int main( int argc, const char **argv ) {
    us_rawnet_multi_open(&rawnet,JDKSAVDECC_AVTP_ETHERTYPE,jdksavdecc_multicast_adp_acmp.value, 0);
    // TODO: Parse args

    if( adpadvertiser_init(&advertiser,0,adpadvertiserd_frame_send) ) {

        memset(&advertiser.adpdu,0,sizeof(advertiser.adpdu));
        jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.header.entity_id, 0x70b3d5ffffedcf0);
        jdksavdecc_eui64_init_from_uint64(&advertiser.adpdu.entity_model_id, 0x70b3d5edc0000000);
        advertiser.adpdu.header.valid_time = 10; /* 20 seconds */


        ipv4_listen_addr = us_net_get_addrinfo("0.0.0.0", "17221", SOCK_DGRAM, true );
        ipv4_multicast_addr = us_net_get_addrinfo("224.0.0.251", "17221", SOCK_DGRAM, false );
        ipv4_socket0 = us_net_create_multicast_rx_udp_socket(
                                  ipv4_listen_addr,
                                  ipv4_multicast_addr,
                                  port0_name);
        ipv4_socket1 = us_net_create_multicast_rx_udp_socket(
                                  ipv4_listen_addr,
                                  ipv4_multicast_addr,
                                  port1_name);

        ipv6_listen_addr = us_net_get_addrinfo("0::0", "17221", SOCK_DGRAM, true );
        ipv6_multicast_addr = us_net_get_addrinfo("ff02::fb", "17221", SOCK_DGRAM, false );
        ipv6_socket0 = us_net_create_multicast_rx_udp_socket(
                                  ipv6_listen_addr,
                                  ipv6_multicast_addr,
                                  port0_name);
        ipv6_socket1 = us_net_create_multicast_rx_udp_socket(
                                  ipv6_listen_addr,
                                  ipv6_multicast_addr,
                                  port1_name);


        while(true) {
            time_t cur_time = time(0);
            if( us_platform_sigint_seen || us_platform_sigterm_seen ) {
                break;
            }

            adpadvertiser_tick( &advertiser, (uint64_t)cur_time*1000 );

#if defined(WIN32)
            Sleep(100);
#else
            fd_set readable;
            int largest_fd=-1;
            int s;
            struct timeval tv;
            FD_ZERO(&readable);
            largest_fd=us_rawnet_multi_set_fdset(&rawnet, &readable);

            if( ipv4_socket0!=-1 ) {
                FD_SET(ipv4_socket0, &readable);
                if( ipv4_socket0 > largest_fd ) largest_fd = ipv4_socket0;
            }
            if( ipv4_socket1!=-1) {
                FD_SET(ipv4_socket1, &readable);
                if( ipv4_socket1 > largest_fd ) largest_fd = ipv4_socket1;
            }
            if( ipv6_socket0!=-1) {
                FD_SET(ipv6_socket0, &readable);
                if( ipv6_socket0 > largest_fd ) largest_fd = ipv6_socket0;
            }
            if( ipv6_socket1!=-1 ) {
                FD_SET(ipv6_socket1, &readable);
                if( ipv6_socket1 > largest_fd ) largest_fd = ipv6_socket1;
            }

            tv.tv_sec = 0;
            tv.tv_usec = 200000; // 200 ms

            do {
                s=select(largest_fd+1, &readable, 0, 0, &tv );
            } while( s<0 && (errno==EINTR || errno==EAGAIN) );

            if( s<0 ) {
                us_log_error( "Unable to select" );
                break;
            }
#endif
            // poll even if select thinks there are no readable sockets
            us_rawnet_multi_rawnet_poll_incoming(
                &rawnet,
                cur_time,
                128,
                &advertiser,
                adpadvertiserd_incoming_raw_packet_handler );
            // handle any packets coming in from UDP

            if( ipv4_socket0!=-1 ) {
                if( FD_ISSET(ipv4_socket0, &readable) ) {
                    adpadvertiserd_incoming_udp_packet_handler(&advertiser,ipv4_socket0);
                }
            }
            if( ipv4_socket1!=-1) {
                if( FD_ISSET(ipv4_socket1, &readable) ) {
                    adpadvertiserd_incoming_udp_packet_handler(&advertiser,ipv4_socket1);
                }
            }
            if( ipv6_socket0!=-1) {
                if( FD_ISSET(ipv6_socket0, &readable) ) {
                    adpadvertiserd_incoming_udp_packet_handler(&advertiser,ipv6_socket0);
                }
            }
            if( ipv6_socket1!=-1 ) {
                if( FD_ISSET(ipv6_socket1, &readable) ) {
                    adpadvertiserd_incoming_udp_packet_handler(&advertiser,ipv6_socket1);
                }
            }

        }


        adpadvertiser_destroy(&advertiser);
    }
    return 0;
}

