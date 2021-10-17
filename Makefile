#      This file is part of the KoraOS project.
#  Copyright (C) 2015-2021  <Fabien Bavent>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

include $(topdir)/make/global.mk
srcdir = $(topdir)/src

all: libsnd

include $(topdir)/make/build.mk
include $(topdir)/make/check.mk
include $(topdir)/make/targets.mk

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
disto ?= kora

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
SRCS += $(wildcard $(srcdir)/*.c)
SRCS += $(srcdir)/addons/disto-$(disto).c

CFLAGS ?= -Wall -Wextra -Wno-unused-parameter -ggdb
CFLAGS += -I$(topdir)/include
CFLAGS += -fPIC

LFLAGS += -lm


ifeq ($(disto),kora)
CFLAGS += -D_GNU_SOURCE
endif


$(eval $(call link_shared,snd,SRCS,LFLAGS))


# install-headers:
# 	$(S) mkdir -p $(prefix)/include
# 	$(S) cp -RpP -f $(topdir)/snd.h $(prefix)/include/snd.h

check: $(patsubst %,val_%,$(CHECKS))

ifeq ($(NODEPS),)
include $(call fn_deps,SRCS)
endif
