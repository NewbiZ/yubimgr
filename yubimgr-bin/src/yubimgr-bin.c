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
#include <string.h>
#include <termios.h>
#include <argp.h>

#include <yubimgr/yubimgr.h>
#include <yubimgr/logging.h>

const char* program_version     = PACKAGE_STRING;
const char* program_bug_address = PACKAGE_BUGREPORT;

static char doc[] =
    "yubimgr -- Bootstrap and manage YubiKeys OpenPGP/PIV applets.";

enum {
    // Options
    OPTION_LOG_LEVEL = 'v',
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
    const char* log_level;
    char action;
    char username[256];
    char firstname[256];
    char lastname[256];
    char email[256];
    char passphrase[256];
};

static struct argp_option options[] = {
    // Options
    {"log-level", OPTION_LOG_LEVEL, "LOG_LEVEL", 0,
     "Logging level (trace|debug|info|warning|error)", 0},
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

void read_info(const char* prompt, size_t min, size_t max, char* out, int echo)
{
    while (strlen(out) < min) {
        printf("%s: ", prompt);
        fflush(stdout);

        struct termios old_termattrs;
        if (!echo) {
            struct termios new_termattrs;
            if (tcgetattr(fileno(stdin), &old_termattrs) != 0) {
                log_error("Failed to turn off echo for passphrase input.\n");
                exit(EXIT_FAILURE);
            }
            new_termattrs = old_termattrs;
            new_termattrs.c_lflag &= ~ECHO;
            if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_termattrs) != 0) {
                log_error("Failed to turn off echo for passphrase input.\n");
                exit(EXIT_FAILURE);
            }
        }

        if (!fgets(out, max, stdin))
            continue;

        if (!echo) {
            tcsetattr(fileno(stdin), TCSAFLUSH, &old_termattrs);
            if (strlen(out) < min) {
                puts("\r");
                fflush(stdout);
            }
        }

        fflush(stdin);
        out[strcspn(out, "\r\n")] = 0;
    }
    if (!echo)
        puts("\r");
}

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    state->name = PACKAGE_NAME;

    struct arguments* arguments = state->input;

    switch (key) {
        case OPTION_LOG_LEVEL:
            arguments->log_level = arg;
            break;
        case ACTION_STATUS:
        case ACTION_RESET:
        case ACTION_BOOTSTRAP:
            if (arguments->action != 0)
                argp_error(state, "only one action is possible.");
            arguments->action = key;
            break;
        case INFO_USERNAME:
            strncpy(arguments->username, arg, 256);
            break;
        case INFO_FIRSTNAME:
            strncpy(arguments->firstname, arg, 256);
            break;
        case INFO_LASTNAME:
            strncpy(arguments->lastname, arg, 256);
            break;
        case INFO_EMAIL:
            strncpy(arguments->email, arg, 256);
            break;
        case ARGP_KEY_END:
            // Check actions
            if (arguments->action == 0) {
                argp_error(state, "an action is required.");
                argp_usage(state);
            }

            // Check log level
            if (arguments->log_level == NULL) {
                set_log_level(LOG_LEVEL_DEBUG);
            } else if (strncmp("trace", arguments->log_level, 6) == 0) {
                set_log_level(LOG_LEVEL_TRACE);
            } else if (strncmp("debug", arguments->log_level, 6) == 0) {
                set_log_level(LOG_LEVEL_DEBUG);
            } else if (strncmp("info", arguments->log_level, 5) == 0) {
                set_log_level(LOG_LEVEL_INFO);
            } else if (strncmp("warning", arguments->log_level, 7) == 0) {
                set_log_level(LOG_LEVEL_WARNING);
            } else if (strncmp("error", arguments->log_level, 6) == 0) {
                set_log_level(LOG_LEVEL_ERROR);
            } else {
                argp_error(state, "invalid log level \"%s\".",
                           arguments->log_level);
            }

            // Check information
            if (arguments->action == ACTION_BOOTSTRAP) {
                read_info("Username", 3, sizeof(arguments->username),
                          arguments->username, 1);
                read_info("Firstname", 3, sizeof(arguments->firstname),
                          arguments->firstname, 1);
                read_info("Lastname", 3, sizeof(arguments->lastname),
                          arguments->lastname, 1);
                read_info("Email", 10, sizeof(arguments->email),
                          arguments->email, 1);
                read_info("Passphrase", 10, sizeof(arguments->passphrase),
                          arguments->passphrase, 0);
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

    set_log_file(stdout);

    switch (arguments.action) {
        case ACTION_STATUS:
            if (status() != 0) {
                log_error("Failed to perform \"status\" action.\n");
                return EXIT_FAILURE;
            }
            break;
        case ACTION_RESET:
            if (reset() != 0) {
                log_error("Failed to perform \"reset\" action.\n");
                return EXIT_FAILURE;
            }
            break;
        case ACTION_BOOTSTRAP:
            if (bootstrap(arguments.username, arguments.firstname,
                          arguments.lastname, arguments.email,
                          arguments.passphrase) != 0) {
                log_error("Failed to perform \"bootstrap\" action.\n");
                return EXIT_FAILURE;
            }
            break;
    }

    return EXIT_SUCCESS;
}
