/*
Copyright (C) 2016 Aurelien Vallee

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <yubimgr/yubimgr.h>

#include <stdio.h>

int reset()
{
    FILE* in;

    static const char* commands =
        "scd reset\n"
        "scd serialno undefined\n"
        "scd apdu 00 A4 04 00 06 D2 76 00 01 24 01\n"
        "scd apdu 00 20 00 81 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 81 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 81 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 81 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 83 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 83 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 83 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 20 00 83 08 40 40 40 40 40 40 40 40\n"
        "scd apdu 00 e6 00 00\n"
        "scd reset\n"
        "scd serialno undefined\n"
        "scd apdu 00 A4 04 00 06 D2 76 00 01 24 01\n"
        "scd apdu 00 44 00 00\n"
        "/bye\n";

    in = popen("gpg-connect-agent >/dev/null", "w");
    fputs(commands, in);
    int err = pclose(in);

    if (!err)
        printf("Successfully reset the smartcard.\n");
    else
        printf("Error during smartcard reset (%d).\n", err);

    return err;
}
