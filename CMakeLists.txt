cmake_minimum_required (VERSION 2.8)
project (adpadvertiser)

include_directories( "." "../jdksavdecc-c/include" "../microsupport/include" "./include" )

set( ADVERTISER_FILES src/adpadvertise.c include/adpadvertise.h )

set( JDKSAVDECC_FILES
../jdksavdecc-c/src/jdksavdecc.c ../jdksavdecc-c/src/jdksavdecc_acmp_print.c ../jdksavdecc-c/src/jdksavdecc_adp_print.c ../jdksavdecc-c/src/jdksavdecc_aecp_print.c ../jdksavdecc-c/src/jdksavdecc_aem_descriptor.c ../jdksavdecc-c/src/jdksavdecc_aem_print.c ../jdksavdecc-c/src/jdksavdecc_allocator.c ../jdksavdecc-c/src/jdksavdecc_app.c ../jdksavdecc-c/src/jdksavdecc_app_print.c ../jdksavdecc-c/src/jdksavdecc_descriptor_storage.c ../jdksavdecc-c/src/jdksavdecc_descriptor_storage_gen.c ../jdksavdecc-c/src/jdksavdecc_frame.c ../jdksavdecc-c/src/jdksavdecc_maap.c ../jdksavdecc-c/src/jdksavdecc_maap_print.c ../jdksavdecc-c/src/jdksavdecc_pdu.c ../jdksavdecc-c/src/jdksavdecc_pdu_print.c ../jdksavdecc-c/src/jdksavdecc_print.c ../jdksavdecc-c/src/jdksavdecc_jdks.c
../jdksavdecc-c/include/jdksavdecc.h ../jdksavdecc-c/include/jdksavdecc_acmp.h ../jdksavdecc-c/include/jdksavdecc_acmp_print.h ../jdksavdecc-c/include/jdksavdecc_adp.h ../jdksavdecc-c/include/jdksavdecc_adp_print.h ../jdksavdecc-c/include/jdksavdecc_aecp.h ../jdksavdecc-c/include/jdksavdecc_aecp_aa.h ../jdksavdecc-c/include/jdksavdecc_aecp_aem.h ../jdksavdecc-c/include/jdksavdecc_aecp_avc.h ../jdksavdecc-c/include/jdksavdecc_aecp_hdcp_apm.h ../jdksavdecc-c/include/jdksavdecc_aecp_print.h ../jdksavdecc-c/include/jdksavdecc_aecp_vendor.h ../jdksavdecc-c/include/jdksavdecc_aem_command.h ../jdksavdecc-c/include/jdksavdecc_aem_descriptor.h ../jdksavdecc-c/include/jdksavdecc_aem_print.h ../jdksavdecc-c/include/jdksavdecc_allocator.h ../jdksavdecc-c/include/jdksavdecc_app.h ../jdksavdecc-c/include/jdksavdecc_app_print.h ../jdksavdecc-c/include/jdksavdecc_avr.h ../jdksavdecc-c/include/jdksavdecc_descriptor_storage.h ../jdksavdecc-c/include/jdksavdecc_descriptor_storage_gen.h ../jdksavdecc-c/include/jdksavdecc_frame.h ../jdksavdecc-c/include/jdksavdecc_jdks.h ../jdksavdecc-c/include/jdksavdecc_maap.h ../jdksavdecc-c/include/jdksavdecc_maap_print.h ../jdksavdecc-c/include/jdksavdecc_ms.h ../jdksavdecc-c/include/jdksavdecc_msinttypes.h ../jdksavdecc-c/include/jdksavdecc_msstdint.h ../jdksavdecc-c/include/jdksavdecc_pdu.h ../jdksavdecc-c/include/jdksavdecc_pdu_print.h ../jdksavdecc-c/include/jdksavdecc_print.h ../jdksavdecc-c/include/jdksavdecc_util.h ../jdksavdecc-c/include/jdksavdecc_world.h
)

set( MICROSUPPORT_FILES ../microsupport/src/us_allocator.c  ../microsupport/src/us_buffer.c ../microsupport/src/us_buffer_print.c ../microsupport/src/us_getopt.c ../microsupport/src/us_logger.c ../microsupport/src/us_logger_printer.c ../microsupport/src/us_logger_stdio.c ../microsupport/src/us_logger_syslog.c  ../microsupport/src/us_parse.c ../microsupport/src/us_platform.c ../microsupport/src/us_print.c ../microsupport/src/us_rawnet.c ../microsupport/include/us_rawnet.h ../microsupport/src/us_time.c ../microsupport/src/us_rawnet_multi.c ../microsupport/include/us_rawnet_multi.h ../microsupport/src/us_world.c ../microsupport/include/us_net.h ../microsupport/src/us_net.c ../microsupport/include/us_getopt.h ../microsupport/include/us_socket_collection.h ../microsupport/src/us_socket_collection.c ../microsupport/include/us_time.h ../microsupport/include/us_daemon.h ../microsupport/src/posix/us_daemon.c )

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



