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

enum LOG_LEVEL {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
};

YUBIMGR_EXPORT
void set_log_level(enum LOG_LEVEL level);

YUBIMGR_EXPORT
enum LOG_LEVEL get_log_level();

YUBIMGR_EXPORT
void set_log_file(FILE* file);

YUBIMGR_EXPORT
FILE* get_log_file();

YUBIMGR_EXPORT
void log_trace(const char* str, ...);

YUBIMGR_EXPORT
void log_debug(const char* str, ...);

YUBIMGR_EXPORT
void log_info(const char* str, ...);

YUBIMGR_EXPORT
void log_warning(const char* str, ...);

YUBIMGR_EXPORT
void log_error(const char* str, ...);
