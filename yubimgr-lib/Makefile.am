# Copyright (C) 2016 Aurelien Vallee
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

AM_CPPFLAGS = \
	-I$(top_srcdir)/yubimgr-lib/include \
	${gpgme_CPPFLAGS}

AM_CFLAGS = \
	-pedantic \
	-fstack-protector -Wl,-z,relro -Wl,-z,now -Wformat-security \
	-ffast-math \
	-W -Wall \
	-Werror \
	-Wpointer-arith \
	-Winit-self \
	-Wformat=2 \
	-Wcast-align \
	-Wlogical-op \
	-Wshadow \
	-fvisibility=hidden \
	-Wl,--discard-all \
	-g \
	-rdynamic

lib_LTLIBRARIES = \
	libyubimgr.la

libyubimgr_la_SOURCES = \
	$(top_srcdir)/yubimgr-lib/src/yubimgr.c

# moduleincludedir = $(pkgincludedir)/module

pkginclude_HEADERS = \
	$(top_srcdir)/yubimgr-lib/include/yubimgr/yubimgr.h

#moduleinclude_HEADERS = \
#	$(top_srcdir)/yubimgr-lib/include/yubimgr/module/file1.h \
#	$(top_srcdir)/yubimgr-lib/include/yubimgr/module/file2.h

libyubimgr_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LTVER)

libyubimgr_la_LIBADD = \
	${gpgme_LIBS}

MAINTAINERCLEANFILES = \
	Makefile.in
