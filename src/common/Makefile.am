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

# Common support code used all over the codebase.

include $(top_srcdir)/Makefile.common

# Pass the git version information to the compiler

VER_REVDESC = $(shell cat $(top_builddir)/gitstamp/gitstamp)
VERSION_FLAGS = $(shell if test x$(VER_REVDESC) != "x"; then echo "-DPHAETHON_REVDESC=\\\"$(VER_REVDESC)\\\""; fi)

VER_REV = $(shell cat $(top_builddir)/gitstamp/gitstamp | cut -d '+' -f 2-)
VERSION_FLAGS += $(shell if test x$(VER_REV) != "x"; then echo "-DPHAETHON_REV=\\\"$(VER_REV)\\\""; fi)

VER_DATE = $(shell date -u '+%Y-%m-%dT%H:%M:%S')
VERSION_FLAGS += $(shell if test x$(VER_DATE) != "x"; then echo "-DPHAETHON_BUILDDATE=\\\"$(VER_DATE)\\\""; fi)

AM_CXXFLAGS += $(VERSION_FLAGS)
AM_CFLAGS   += $(VERSION_FLAGS)

# Rebuild the version.lo when the git version changed
version.lo: $(top_builddir)/gitstamp/gitstamp

SUBDIRS = \
          $(EMPTY)

noinst_LTLIBRARIES = libcommon.la

noinst_HEADERS = \
                 endianness.h \
                 system.h \
                 types.h \
                 util.h \
                 strutil.h \
                 encoding.h \
                 platform.h \
                 readstream.h \
                 memreadstream.h \
                 writestream.h \
                 memwritestream.h \
                 maths.h \
                 noncopyable.h \
                 singleton.h \
                 error.h \
                 ustring.h \
                 version.h \
                 hash.h \
                 md5.h \
                 blowfish.h \
                 readfile.h \
                 writefile.h \
                 filepath.h \
                 filelist.h \
                 filetree.h \
                 zipfile.h \
                 bitstream.h \
                 huffman.h \
                 sinewindows.h \
                 cosinetables.h \
                 fft.h \
                 mdct.h \
                 atomic.h \
                 mutex.h \
                 thread.h \
                 $(EMPTY)

libcommon_la_SOURCES = \
                       util.cpp \
                       strutil.cpp \
                       encoding.cpp \
                       platform.cpp \
                       readstream.cpp \
                       memreadstream.cpp \
                       writestream.cpp \
                       memwritestream.cpp \
                       maths.cpp \
                       md5.cpp \
                       blowfish.cpp \
                       error.cpp \
                       ustring.cpp \
                       version.cpp \
                       readfile.cpp \
                       writefile.cpp \
                       filepath.cpp \
                       filelist.cpp \
                       filetree.cpp \
                       zipfile.cpp \
                       huffman.cpp \
                       sinewindows.cpp \
                       cosinetables.cpp \
                       fft.cpp \
                       mdct.cpp \
                       mutex.cpp \
                       thread.cpp \
                       $(EMPTY)

libcommon_la_LIBADD = \
                      $(EMPTY)
