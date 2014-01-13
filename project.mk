PROJECT=adpadvertiser
PROJECT_NAME=adpadvertiser
PROJECT_VERSION?=20140113
PROJECT_EMAIL=jeffk@jdkoftinoff.com
PROJECT_LICENSE=BSD
PROJECT_MAINTAINER=jeffk@jdkoftinoff.com
PROJECT_COPYRIGHT=Copyright 2013
PROJECT_DESCRIPTION=
PROJECT_WEBSITE=https://github.com/jdkoftinoff/adpadvertiser
PROJECT_IDENTIFIER=com.statusbar.adpadvertiser
TOP_LIB_DIRS+=. 
CONFIG_TOOLS+=
PKGCONFIG_PACKAGES+=
TEST_OUT_SUFFIX=txt

MICROSUPPORT_DIR?=../microsupport

TOP_LIB_DIRS+=../microsupport

CONFIG_TOOLS=

COMPILE_FLAGS+=-Wall -W

