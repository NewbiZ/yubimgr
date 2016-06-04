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
#include <stdlib.h>
#include <stdio.h>
#include <argp.h>

#include <yubimgr/yubimgr.h>

const char* program_version     = PACKAGE_STRING;
const char* program_bug_address = PACKAGE_BUGREPORT;

static char doc[] =
    "yubimgr -- Bootstrap and manage YubiKeys OpenPGP/PIV applets.";

enum {
    // Actions
    ACTION_BOOTSTRAP = 'b',
    ACTION_RESET     = 'r',
    ACTION_STATUS    = 's',
    // Information
    INFO_USERNAME  = 'u',
    INFO_FIRSTNAME = 'f',
    INFO_LASTNAME  = 'l',
    INFO_EMAIL     = 'e',
};

struct arguments {
    char action;
    const char* username;
    const char* firstname;
    const char* lastname;
    const char* email;
};

static struct argp_option options[] = {
    // Actions
    {"status", ACTION_STATUS, 0, 0, "Print smartcard status.", 0},
    {"reset", ACTION_RESET, 0, 0, "Reset smartcard to factory.", 0},
    {"bootstrap", ACTION_BOOTSTRAP, 0, 0, "Bootstrap new masterkey.", 0},
    // Info
    {"username", INFO_USERNAME, "USERNAME", 0, "Provide username.", 0},
    {"firstname", INFO_FIRSTNAME, "FIRSTNAME", 0, "Provide first name.", 0},
    {"lastname", INFO_LASTNAME, "LASTNAME", 0, "Provide last name.", 0},
    {"email", INFO_EMAIL, "EMAIL", 0, "Provide email.", 0},

    {0},
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    state->name = PACKAGE_NAME;

    struct arguments* arguments = state->input;

    switch (key) {
        case ACTION_STATUS:
        case ACTION_RESET:
        case ACTION_BOOTSTRAP:
            if (arguments->action != 0)
                argp_error(state, "only one action is possible.");
            arguments->action = key;
            break;
        case INFO_USERNAME:
            arguments->username = arg;
            break;
        case INFO_FIRSTNAME:
            arguments->firstname = arg;
            break;
        case INFO_LASTNAME:
            arguments->lastname = arg;
            break;
        case INFO_EMAIL:
            arguments->email = arg;
            break;
        case ARGP_KEY_END:
            if (arguments->action == 0) {
                argp_error(state, "an action is required.");
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, 0, doc, 0, 0, 0};

int main(int argc, char** argv)
{
    argp_program_version     = program_version;
    argp_program_bug_address = program_bug_address;

    struct arguments arguments = {0};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    switch (arguments.action) {
        case ACTION_STATUS:
            status();
            break;
        case ACTION_RESET:
            reset();
            break;
        case ACTION_BOOTSTRAP:
            bootstrap();
            break;
    }

    return EXIT_SUCCESS;
}
