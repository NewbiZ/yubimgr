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

# Start the configuration phase
# =============================
AC_PREREQ(2.59)

AC_INIT([yubimgr], [m4_esyscmd([./version_api.sh])],
    [vallee.aurelien@gmail.com], [yubimgr],
    [https://github.com/NewbiZ/yubimgr])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([foreign])
LT_INIT

m4_ifdef([AM_SILENT_RULES], [
    AM_SILENT_RULES([yes])
])

# API & ABI
# =========
LTVER=m4_esyscmd([./version_abi.sh])
AC_SUBST(LTVER)

# Check build tools
# =================
AC_PROG_CC
AC_LANG([C])
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Check dependencies
# ==================
AC_CONFIG_MACRO_DIR([m4])

# Finish the configuration phase
# ==============================
AC_CONFIG_FILES(Makefile \
                yubimgr-lib/Makefile \
                yubimgr-bin/Makefile \
                yubimgr-tests/Makefile )

AC_OUTPUT
