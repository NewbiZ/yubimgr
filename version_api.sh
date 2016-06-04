#!/bin/bash

# Copyright (C) 2016 Aurelien Vallee
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

if [ ! -f $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h ]; then
    echo "version.sh: error: $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h does not exist" 1>&2
    exit 1
fi

MAJOR=`egrep '^#define +YUBIMGR_VERSION_API_MAJOR +[0-9]+$' $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h`
MINOR=`egrep '^#define +YUBIMGR_VERSION_API_MINOR +[0-9]+$' $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h`
PATCH=`egrep '^#define +YUBIMGR_VERSION_API_PATCH +[0-9]+$' $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h`

if [ -z "$MAJOR" -o -z "$MINOR" -o -z "$PATCH" ]; then
    echo "version.sh: error: could not extract version from $(dirname $0)/yubimgr-lib/include/yubimgr/yubimgr.h" 1>&2
    exit 1
fi

MAJOR=`echo $MAJOR | awk '{ print $3 }'`
MINOR=`echo $MINOR | awk '{ print $3 }'`
PATCH=`echo $PATCH | awk '{ print $3 }'`

echo $MAJOR.$MINOR.$PATCH | tr -d '\n'
