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
#include <yubimgr/logging.h>

#include <gpgme.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#define _XOPEN_SOURCE 500
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <sys/stat.h>

int check_gpgme()
{
    gpgme_error_t err;

    // Check for proper GPGME version
    const char* version;
    version = gpgme_check_version(NULL);
    log_info("Using gpgme %s\n", version);

    // Set default locale
    setlocale(LC_ALL, "");
    gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));

    // Check for OpenPGP support
    if ((err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP)) !=
        GPG_ERR_NO_ERROR) {
        log_error("Failed to check for OpenPGP support.\n");
        return 1;
    }

    // Get engine information
    gpgme_engine_info_t engine_info;
    if ((err = gpgme_get_engine_info(&engine_info)) != GPG_ERR_NO_ERROR) {
        log_error("Failed to retrieve GPGME engine information.\n");
        return 1;
    }

    return 0;
}

int set_passphrase(const char* temporary_keyring, const char* passphrase)
{
    static char pinentry_path[256];
    static const char* pinentry_filename = "pinentry";
    strncpy(pinentry_path, temporary_keyring,
            sizeof(pinentry_path) - sizeof(pinentry_filename) - 1);
    strcat(pinentry_path, "/");
    strcat(pinentry_path, pinentry_filename);
    log_debug("Generating pinentry at \"%s\".\n", pinentry_path);
    FILE* pinentry_file = fopen(pinentry_path, "w");
    fprintf(pinentry_file,
            "#!/bin/bash\n"
            "\n"
            "echo OK Your orders please\n"
            "while read cmd; do\n"
            "  case $cmd in\n"
            "    GETPIN) echo \"D %s\"; echo OK;;\n"
            "    *) echo OK;;\n"
            "  esac\n"
            "done\n",
            passphrase);
    fclose(pinentry_file);

    if (chmod(pinentry_path, S_IRUSR | S_IWUSR | S_IXUSR)) {
        log_error("Failed to chmod pinentry program.\n");
        return 1;
    }

    return 0;
}

int configure_gpg(const char* temporary_keyring)
{
    // Setup GPG to automatically use "expert" mode
    static char gpg_conf_path[256];
    static const char* gpg_conf_filename = "gpg.conf";
    strncpy(gpg_conf_path, temporary_keyring,
            sizeof(gpg_conf_path) - sizeof(gpg_conf_filename) - 1);
    strcat(gpg_conf_path, "/");
    strcat(gpg_conf_path, gpg_conf_filename);
    log_debug("Generating gpg.conf at \"%s\".\n", gpg_conf_path);
    FILE* gpg_conf_file = fopen(gpg_conf_path, "w");
    fputs("expert\n", gpg_conf_file);
    fclose(gpg_conf_file);

    return 0;
}

int configure_gpg_agent(const char* temporary_keyring)
{
    // Setup gpg-agent to use dummy pinentry program
    static char gpg_agent_conf_path[256];
    static const char* gpg_agent_conf_filename = "gpg-agent.conf";
    strncpy(gpg_agent_conf_path, temporary_keyring,
            sizeof(gpg_agent_conf_path) - sizeof(gpg_agent_conf_filename));
    strcat(gpg_agent_conf_path, "/");
    strcat(gpg_agent_conf_path, gpg_agent_conf_filename);
    log_debug("Generatig gpg-agent.conf at \"%s\".\n", gpg_agent_conf_path);
    FILE* gpg_agent_conf_file = fopen(gpg_agent_conf_path, "w");
    fprintf(gpg_agent_conf_file, "pinentry-program %s/pinentry\n",
            temporary_keyring);
    fputs("allow-loopback-pinentry\n", gpg_agent_conf_file);
    fclose(gpg_agent_conf_file);

    // Make sure gpg-agent is running properly and configured to use our
    // gpg-agent.conf
    static char gpg_connect_agent_command[256];
    snprintf(gpg_connect_agent_command, sizeof(gpg_connect_agent_command),
             "gpg-connect-agent --homedir \"%s\" KILLAGENT /bye 2>&1",
             temporary_keyring);
    FILE* outp = popen(gpg_connect_agent_command, "r");
    log_debug("Running command \"%s\".\n", gpg_connect_agent_command);
    if (pclose(outp))
        log_error("Failed to stop currently running gpg-agent.\n");
    snprintf(gpg_connect_agent_command, sizeof(gpg_connect_agent_command),
             "gpg-connect-agent --homedir \"%s\" /bye 2>&1", temporary_keyring);
    outp = popen(gpg_connect_agent_command, "r");
    log_debug("Running command \"%s\".\n", gpg_connect_agent_command);
    if (pclose(outp))
        log_error("Failed to stop currently running gpg-agent.\n");

    return 0;
}

int setup_gpgme(gpgme_ctx_t context, const char* temporary_keyring)
{
    gpgme_error_t err;

    // Prepare the environment
    unsetenv("GPG_AGENT_INFO");
    setenv("GNUPGHOME", temporary_keyring, 1);

    if (configure_gpg(temporary_keyring))
        return 1;

    if (configure_gpg_agent(temporary_keyring))
        return 1;

    // Setup GPGME context
    if ((err = gpgme_new(&context)) != GPG_ERR_NO_ERROR) {
        log_error("Failed to create GPGME context.\n");
        return 1;
    }

    if ((err = gpgme_set_protocol(context, GPGME_PROTOCOL_OpenPGP)) !=
        GPG_ERR_NO_ERROR) {
        log_error("Failed to use OpenPGP protocol.\n");
        return 1;
    }

    return 0;
}

int mk_tmpdir(char* tmpdir, size_t size)
{
    const char* tmprootdir = getenv("TMPDIR");
    if (!tmprootdir)
        tmprootdir          = "/tmp";
    size_t tmprootdir_count = strlen(tmprootdir);

    static const char tmpnametpl[]       = "tmp.yubimgr.XXXXXX";
    static const size_t tmpnametpl_count = sizeof(tmpnametpl) - 1;

    // Account space count for '/' and \0
    if (tmprootdir_count + tmpnametpl_count + 2 > size) {
        log_error("Temporary directory name too long. Check your $TMPDIR.\n");
        return 1;
    }

    strncpy(tmpdir, tmprootdir, size);
    strncat(tmpdir, "/", 1);
    strncat(tmpdir, tmpnametpl, size);

    return mkdtemp(tmpdir) != 0;
}

int unlink_cb(const char* fpath,
              const struct stat __attribute__((unused)) * sb,
              int __attribute__((unused)) typeflag,
              struct FTW __attribute__((unused)) * ftwbuf)
{
    log_debug("Removing temporary keyring file \"%s\".\n", fpath);
    return remove(fpath);
}

void rm_tmpdir(const char* path)
{
    log_debug("Removing temporary keyring %s.\n", path);
    nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void generate_masterkey(gpgme_ctx_t context, const char* passphrase)
{
    log_info("Generating masterkey");

    static const char genkey_params_template[] =
        "<GnupgKeyParms format=\"internal\">\n"
        "    Key-Type: RSA\n"
        "    Key-Length: 2048\n"
        "    Key-Usage: sign\n"
        "    Name-Real: %s\n"
        "    Name-Comment: %s\n"
        "    Name-Email: %s\n"
        "    Expire-Date: 0\n"
        "    Passphrase: %s\n"
        "</GnupgKeyParms>\n";
    (void)(genkey_params_template);
    (void)(context);
    (void)(passphrase);
}

int bootstrap(const char* username,
              const char* firstname,
              const char* lastname,
              const char* email,
              const char* passphrase)
{
    (void)(username);
    (void)(firstname);
    (void)(lastname);
    (void)(email);
    (void)(passphrase);
    int err;
    gpgme_ctx_t context = NULL;

    if ((err = check_gpgme()) != 0)
        return err;

    static char temporary_keyring[256];
    if (!mk_tmpdir(temporary_keyring, sizeof(temporary_keyring))) {
        log_error("Failed to create temporary keyring.\n");
        return 1;
    }
    log_debug("Using temporary keyring directory %s.\n", temporary_keyring);

    if ((err = setup_gpgme(context, temporary_keyring)) != 0)
        goto cleanup;

    if (set_passphrase(temporary_keyring, "hello world :)"))
        goto cleanup;

cleanup:
    rm_tmpdir(temporary_keyring);

    return err;
}
