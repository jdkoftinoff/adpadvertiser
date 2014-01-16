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

uint8_t adpadvertiserd_first_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

bool option_help=false;
bool option_help_default=false;
bool option_dump=false;
bool option_dump_default=false;
#if US_ENABLE_DAEMON==1
uint16_t option_daemon=0;
uint16_t option_daemon_default=0;
#endif
#if US_ENABLE_SYSLOG==1
uint16_t option_syslog=0;
uint16_t option_syslog_default=0;
#endif
uint16_t option_log_level=0;
uint16_t option_log_level_default=US_LOG_LEVEL_INFO;
uint16_t option_log_others=0;
uint16_t option_log_others_default=0;
uint16_t option_udp=0;
uint16_t option_udp_default=0;
uint16_t option_avtp=0;
uint16_t option_avtp_default=1;
uint64_t option_entity_id=0xffffffffffffffffULL;
uint64_t option_entity_id_default=0xffffffffffffffffULL;
uint64_t option_entity_model_id=0xffffffffffffffffULL;
uint64_t option_entity_model_id_default=0xffffffffffffffffULL;
uint64_t option_entity_capabilities = 0;
uint64_t option_entity_capabilities_default = 0;
uint16_t option_valid_time = 0;
uint16_t option_valid_time_default = 10;


us_getopt_option_t adpadvertiserd_main_option[] = {
    {"dump","Dump settings to stdout", US_GETOPT_FLAG, &option_dump_default, &option_dump },
    {"help","Show help", US_GETOPT_FLAG, &option_help_default, &option_help },
    {"log_level", "Log Level 0-5", US_GETOPT_INT16, &option_log_level_default, &option_log_level },
#if US_ENABLE_DAEMON==1
    {"daemon","daemonize", US_GETOPT_UINT16, &option_daemon_default, &option_daemon },
#endif
#if US_ENABLE_SYSLOG==1
    {"syslog","send logging to syslog", US_GETOPT_UINT16, &option_syslog_default, &option_syslog },
#endif
    {"udp","enable 1722.1 over UDP (IPv4 and IPv6)", US_GETOPT_UINT16, &option_udp_default, &option_udp },
    {"avtp","enable 1722.1 over AVTP (Ethertype 0x22f0)", US_GETOPT_UINT16, &option_avtp_default, &option_avtp },
    {"log_others", "Log other entities available/departing messages", US_GETOPT_UINT16, &option_log_others_default, &option_log_others },
    {0,0,US_GETOPT_NONE,0,0}
};

us_getopt_option_t adpadvertiserd_entity_option[] = {
    {"id","entity_id", US_GETOPT_HEX64, &option_entity_id_default, &option_entity_id },
    {"model_id","entity_model_id", US_GETOPT_HEX64, &option_entity_model_id_default, &option_entity_model_id },
    {"capabilities","entity_capabilities", US_GETOPT_HEX32, &option_entity_capabilities_default, &option_entity_capabilities },
    {"valid_time","valid_time in seconds", US_GETOPT_INT16, &option_valid_time_default, &option_valid_time },
    {0,0,US_GETOPT_NONE,0,0}
};


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
    (void)context;
    (void)fd;
    (void)from_addr;
    (void)from_addrlen;
    if( len>0 ) {
        if( buf[0]==JDKSAVDECC_1722A_SUBTYPE_ADP ) {
            //us_log_debug("incoming ADP on socket %d, sa_family=%d", fd, (int)from_addr->sa_family );

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
    (void)self;
    (void)context;

    us_socket_collection_group_t *g = (us_socket_collection_group_t *)&sockets;
    
    (void)self;
    (void)context;

    for ( i=0; i<g->num_collections; ++i ) {
        us_socket_collection_t *c = g->collection[i];
        int j;
        for (j=0; j<c->num_sockets; ++j ) {
            int fd = c->socket_fd[j];
            void *socket_context = c->socket_context[j];
            if( fd!=-1 && c->send_data!=0) {
                c->send_data( c, socket_context, fd, 0, 0, buf, len );
            }
        }
    }
}


void adpadvertiserd_initialize_udp_sockets_on_port(
    us_socket_collection_t *self,
    const char *port_name,
    struct sockaddr *addr ) {

    const char *multicast_addr = ADPADVERTISER_MDNS_MULTICAST_IPV4;
    const char *local_addr = "0";
    if( addr->sa_family == AF_INET6 ) {
        multicast_addr = ADPADVERTISER_MDNS_MULTICAST_IPV6;
        local_addr = "0::0";
    }

    us_socket_collection_add_multicast_udp(
        self,
        local_addr, ADPADVERTISER_AVDECC_UDP_PORT,       // The address and port to listen on
        multicast_addr, ADPADVERTISER_AVDECC_UDP_PORT,   // The multicast group to join
        port_name,                                       // the ethernet device to use
        us_net_get_addrinfo(multicast_addr, ADPADVERTISER_AVDECC_UDP_PORT, SOCK_DGRAM, false ) // The default address to send to
        );
}

bool adpadvertiserd_is_network_port_interesting( struct ifaddrs *port ) {

    bool r=false;
    if( port->ifa_name && *port->ifa_name &&
        (port->ifa_addr->sa_family == AF_INET
        || port->ifa_addr->sa_family == AF_INET6 ) ) {
#if defined(__APPLE__)
        if( port->ifa_name[0] =='e' && port->ifa_name[1] =='n' && isdigit(port->ifa_name[2]))
#elif defined(__linux__)
        if( port->ifa_name[0] =='e' && port->ifa_name[1] =='t' && port->ifa_name[2]=='h' && isdigit(port->ifa_name[3]) )
#endif
        {
            r=true;
        }
    }
    return r;
}


void adpadvertiserd_initialize_udp_sockets_on_all_ports(
    us_socket_collection_t *self ) {

    struct ifaddrs *addrs;

    if( getifaddrs(&addrs)==0 ) {
        struct ifaddrs *cur = addrs;
        while( cur ) {
            if( adpadvertiserd_is_network_port_interesting(cur) ) {
                adpadvertiserd_initialize_udp_sockets_on_port(self,cur->ifa_name,cur->ifa_addr);
            }
            cur = cur->ifa_next;
        }
        freeifaddrs(addrs);
    }
}


void adpadvertiserd_initialize_sockets(
    us_socket_collection_group_t *self ) {

    int i;

    // Create the socket collection group that encapsulates all the sockets
    us_socket_collection_group_init(self);

    // create the udp sockets collection and add all the ethernet ports and ip protocols
    if( option_udp ) {
        us_socket_collection_init_udp_multicast(&udp_sockets);
        udp_sockets.readable = adpadvertiserd_message_readable;
        udp_sockets.user_context = &advertiser;
        adpadvertiserd_initialize_udp_sockets_on_all_ports(&udp_sockets);
        us_socket_collection_group_add(self,&udp_sockets);
    }

    // Create the rawnet sockets collection and add all the sockets that the rawnet_multi found for us.
    if( option_avtp ) {
        us_socket_collection_init_rawnet(&rawnet_sockets);
        us_rawnet_multi_open(&rawnet,JDKSAVDECC_AVTP_ETHERTYPE,jdksavdecc_multicast_adp_acmp.value, 0);
        if( rawnet.ethernet_port_count>0 ) {
            memcpy( adpadvertiserd_first_mac_address, rawnet.ethernet_ports[0].m_my_mac, 6 );
        }

        rawnet_sockets.readable = adpadvertiserd_message_readable;
        rawnet_sockets.user_context = &advertiser;

        for( i=0; i<rawnet.ethernet_port_count; ++i ) {
            us_rawnet_context_t *c = &rawnet.ethernet_ports[i];
            us_socket_collection_add_rawnet(
                &rawnet_sockets,
                c);
        }
        us_socket_collection_group_add(self,&rawnet_sockets);
    }
}

void adpadvertiserd_initialize_entity_info( struct jdksavdecc_adpdu *adpdu ) {

    if( option_entity_id==0xffffffffffffffffULL ) {
        adpdu->header.entity_id.value[0] = adpadvertiserd_first_mac_address[0];
        adpdu->header.entity_id.value[1] = adpadvertiserd_first_mac_address[1];
        adpdu->header.entity_id.value[2] = adpadvertiserd_first_mac_address[2];
        adpdu->header.entity_id.value[3] = 0xff;
        adpdu->header.entity_id.value[4] = 0xfe;
        adpdu->header.entity_id.value[5] = adpadvertiserd_first_mac_address[3];
        adpdu->header.entity_id.value[6] = adpadvertiserd_first_mac_address[4];
        adpdu->header.entity_id.value[7] = adpadvertiserd_first_mac_address[5];
    }
    jdksavdecc_eui64_init_from_uint64(&adpdu->entity_model_id,  option_entity_model_id);
    adpdu->entity_capabilities = option_entity_capabilities;

    adpdu->header.valid_time = option_valid_time / 2;
}

void adpadvertiserd_receive_entity_available_or_departing(
    struct adpadvertiser *self,
    void *context,
    struct jdksavdecc_adpdu *adpdu ) {

    (void)self;
    (void)context;
    if( option_log_others ) {
        const char *type = "Available";
        if( adpdu->header.message_type == JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DEPARTING ) {
            type="Departing";
        }
        us_log_debug("Received %s index 0x%08lx for entity_id 0x%016llx",
                  type,
                  adpdu->available_index,
                  jdksavdecc_eui64_convert_to_uint64(&adpdu->header.entity_id));
    }
}

bool adpadvertiserd_process_options( const char **argv ) {
    bool r=false;
    us_malloc_allocator_t allocator;
    us_getopt_t opt;
    us_print_file_t p;
    us_print_file_init(&p, stdout);

    us_malloc_allocator_init( &allocator );

    if( us_getopt_init( &opt, &allocator.m_base ) ) {
        us_getopt_add_list( &opt, adpadvertiserd_main_option, 0, "adpadvertiserd main options" );
        us_getopt_add_list( &opt, adpadvertiserd_entity_option, "adp", "ADP Entity Information"  );
        us_getopt_fill_defaults( &opt );
        if( us_getopt_parse_args( &opt, argv+1 ) ) {
            r=true;
        }

        if( option_dump ) {
            us_getopt_dump(&opt, &p.m_base, "dump" );
            r=false;
        } else if( option_help ) {
            us_getopt_dump(&opt, &p.m_base, "help" );
            r=false;
        }
#if US_ENABLE_SYSLOG==1
        if( option_syslog ) {

        }
#endif
        us_getopt_destroy(&opt);
    }
    return r;
}

int main( int argc, const char **argv ) {

    (void)argc;
    us_logger_stdio_start(stdout, stderr);

    if( adpadvertiserd_process_options(argv) ) {

#if US_ENABLE_DAEMON==1
        us_daemon_daemonize(option_daemon, "adpadvertiserd", 0, 0, 0);
#endif
        if( adpadvertiser_init(
                &advertiser,
                0,
                adpadvertiserd_frame_send,
                adpadvertiserd_receive_entity_available_or_departing) ) {

            adpadvertiserd_initialize_sockets( &sockets );
            advertiser.context = &sockets;
            advertiser.received_entity_available_or_departing = adpadvertiserd_receive_entity_available_or_departing;
            adpadvertiserd_initialize_entity_info(&advertiser.adpdu);
            adpadvertiser_send_entity_discover(&advertiser);

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
                        advertiser.early_tick ? 0 : 500) ) {
                    break;
                }
            }

            adpadvertiser_destroy(&advertiser);
            us_socket_collection_group_destroy(&sockets);
        }
    }
    return 0;
}

