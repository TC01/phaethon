# xoreos - A reimplementation of BioWare's Aurora engine
#
# xoreos is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# xoreos is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# xoreos is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xoreos. If not, see <http://www.gnu.org/licenses/>.

# Sound (music, sound effects) subsystem.

include $(top_srcdir)/Makefile.common

SUBDIRS = \
          decoders \
          $(EMPTY)

noinst_LTLIBRARIES = libsound.la

noinst_HEADERS = \
                 types.h \
                 audiostream.h \
                 sound.h \
                 $(EMPTY)

libsound_la_SOURCES = \
                      audiostream.cpp \
                      sound.cpp \
                      $(EMPTY)

libsound_la_LIBADD = \
                     decoders/libdecoders.la \
                     $(EMPTY)
