cmake_minimum_required (VERSION 2.8)
project (adpadvertiser)

set( JDKSAVDECC_DIR jdksavdecc-c )
set( MICROSUPPORT_DIR microsupport )

include_directories( ${JDKSAVDECC_DIR}/include ${MICROSUPPORT_DIR}/include ./include )


set( JDKSAVDECC_FILES
${JDKSAVDECC_DIR}/src/jdksavdecc.c ${JDKSAVDECC_DIR}/src/jdksavdecc_acmp_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_adp_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_aecp_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_aem_descriptor.c ${JDKSAVDECC_DIR}/src/jdksavdecc_aem_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_allocator.c ${JDKSAVDECC_DIR}/src/jdksavdecc_app.c ${JDKSAVDECC_DIR}/src/jdksavdecc_app_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_descriptor_storage.c ${JDKSAVDECC_DIR}/src/jdksavdecc_descriptor_storage_gen.c ${JDKSAVDECC_DIR}/src/jdksavdecc_frame.c ${JDKSAVDECC_DIR}/src/jdksavdecc_maap.c ${JDKSAVDECC_DIR}/src/jdksavdecc_maap_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_pdu.c ${JDKSAVDECC_DIR}/src/jdksavdecc_pdu_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_print.c ${JDKSAVDECC_DIR}/src/jdksavdecc_jdks.c ${JDKSAVDECC_DIR}/src/jdksavdecc_adp_manager.c
${JDKSAVDECC_DIR}/include/jdksavdecc.h ${JDKSAVDECC_DIR}/include/jdksavdecc_acmp.h ${JDKSAVDECC_DIR}/include/jdksavdecc_acmp_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_adp.h ${JDKSAVDECC_DIR}/include/jdksavdecc_adp_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_aa.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_aem.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_avc.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_hdcp_apm.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aecp_vendor.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aem_command.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aem_descriptor.h ${JDKSAVDECC_DIR}/include/jdksavdecc_aem_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_allocator.h ${JDKSAVDECC_DIR}/include/jdksavdecc_app.h ${JDKSAVDECC_DIR}/include/jdksavdecc_app_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_avr.h ${JDKSAVDECC_DIR}/include/jdksavdecc_descriptor_storage.h ${JDKSAVDECC_DIR}/include/jdksavdecc_descriptor_storage_gen.h ${JDKSAVDECC_DIR}/include/jdksavdecc_frame.h ${JDKSAVDECC_DIR}/include/jdksavdecc_jdks.h ${JDKSAVDECC_DIR}/include/jdksavdecc_maap.h ${JDKSAVDECC_DIR}/include/jdksavdecc_maap_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_ms.h ${JDKSAVDECC_DIR}/include/jdksavdecc_msinttypes.h ${JDKSAVDECC_DIR}/include/jdksavdecc_msstdint.h ${JDKSAVDECC_DIR}/include/jdksavdecc_pdu.h ${JDKSAVDECC_DIR}/include/jdksavdecc_pdu_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_print.h ${JDKSAVDECC_DIR}/include/jdksavdecc_util.h ${JDKSAVDECC_DIR}/include/jdksavdecc_world.h ${JDKSAVDECC_DIR}/include/jdksavdecc_adp_manager.h
)

set( MICROSUPPORT_FILES ${MICROSUPPORT_DIR}/src/us_allocator.c  ${MICROSUPPORT_DIR}/src/us_buffer.c ${MICROSUPPORT_DIR}/src/us_buffer_print.c ${MICROSUPPORT_DIR}/src/us_getopt.c ${MICROSUPPORT_DIR}/src/us_logger.c ${MICROSUPPORT_DIR}/src/us_logger_printer.c ${MICROSUPPORT_DIR}/src/us_logger_stdio.c ${MICROSUPPORT_DIR}/src/us_logger_syslog.c  ${MICROSUPPORT_DIR}/src/us_parse.c ${MICROSUPPORT_DIR}/src/us_platform.c ${MICROSUPPORT_DIR}/src/us_print.c ${MICROSUPPORT_DIR}/src/us_rawnet.c ${MICROSUPPORT_DIR}/include/us_rawnet.h ${MICROSUPPORT_DIR}/src/us_time.c ${MICROSUPPORT_DIR}/src/us_rawnet_multi.c ${MICROSUPPORT_DIR}/include/us_rawnet_multi.h ${MICROSUPPORT_DIR}/src/us_world.c ${MICROSUPPORT_DIR}/include/us_net.h ${MICROSUPPORT_DIR}/src/us_net.c ${MICROSUPPORT_DIR}/include/us_getopt.h ${MICROSUPPORT_DIR}/include/us_socket_collection.h ${MICROSUPPORT_DIR}/src/us_socket_collection.c ${MICROSUPPORT_DIR}/include/us_time.h ${MICROSUPPORT_DIR}/include/us_daemon.h ${MICROSUPPORT_DIR}/src/posix/us_daemon.c ${MICROSUPPORT_DIR}/include/us_platform.h ${MICROSUPPORT_DIR}/include/us_options.h ${MICROSUPPORT_DIR}/include/us_net_router.h ${MICROSUPPORT_DIR}/src/us_net_router.c )

add_definitions( -DUS_ENABLE_RAW_ETHERNET=1 )
if(APPLE)
  add_definitions( -DTARGET_PLATFORM_POSIX=1)
elseif(UNIX)
  add_definitions( -DTARGET_PLATFORM_POSIX=1)
elseif(WIN32)
  add_definitions( -DTARGET_PLATFORM_WIN32=1 )
  include_directories( $ENV{WPCAP_DIR}/Include )
  if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    link_directories( $ENV{WPCAP_DIR}/Lib/x64 )
  elseif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    link_directories( $ENV{WPCAP_DIR}/Lib )
  endif()
endif()

add_library("adputil" STATIC ${ADVERTISER_FILES} ${MICROSUPPORT_FILES} ${JDKSAVDECC_FILES} ${PLATFORM_FILES} )

link_directories( ${ADPADVERTISER_BINARY_DIR} )

add_executable("adpadvertiserd" "tools/adpadvertiserd.h" "tools/adpadvertiserd.c" )

target_link_libraries("adpadvertiserd" adputil )

if(APPLE)
  target_link_libraries( "adpadvertiserd" pcap)
elseif(UNIX)
elseif(WIN32)
  target_link_libraries("adpadvertiserd" wpcap Iphlpapi Ws2_32)
endif()



