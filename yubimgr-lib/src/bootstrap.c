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
#include <errno.h>
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
            "    GETPIN) echo \"D %s\"; echo \"omfg\" >/tmp/lol ; echo OK;;\n"
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

int setup_gpgme(struct gpgme_context** context, const char* temporary_keyring)
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
    if ((err = gpgme_new(context)) != GPG_ERR_NO_ERROR) {
        log_error("Failed to create GPGME context.\n");
        return 1;
    }

    if ((err = gpgme_set_protocol(*context, GPGME_PROTOCOL_OpenPGP)) !=
        GPG_ERR_NO_ERROR) {
        log_error("Failed to use OpenPGP protocol.\n");
        return 1;
    }

    gpgme_set_armor(*context, 1);

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
    log_trace("Removing temporary keyring file \"%s\".\n", fpath);
    return remove(fpath);
}

void rm_tmpdir(const char* path)
{
    log_debug("Removing temporary keyring %s.\n", path);
    nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int generate_masterkey(struct gpgme_context* context,
                       const char* temporary_keyring,
                       const char* username,
                       const char* firstname,
                       const char* lastname,
                       const char* email,
                       const char* passphrase,
                       char* masterkey_fpr)
{
    log_info("Generating masterkey...\n");

    int err = 0;

    if ((err = set_passphrase(temporary_keyring, passphrase)))
        return err;

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

    char* realname = (char*)malloc(strlen(username) + strlen(lastname) + 2);
    strcpy(realname, firstname);
    strcat(realname, " ");
    strcat(realname, lastname);

    size_t genkey_params_size = sizeof(genkey_params_template) - 2 +
                                strlen(passphrase) + strlen(realname) +
                                strlen(email) + strlen(username);
    char* genkey_params = (char*)malloc(genkey_params_size);
    snprintf(genkey_params, genkey_params_size, genkey_params_template,
             realname, username, email, passphrase);

    log_trace("Key generation params:\n%s", genkey_params);

    if ((err = gpgme_op_genkey(context, genkey_params, NULL, NULL))) {
        log_error("Failed to call genkey (%d). %s: %s\n", err,
                  gpgme_strsource(err), gpgme_strerror(err));
        return err;
    }

    gpgme_genkey_result_t result;
    if (!(result = gpgme_op_genkey_result(context))) {
        log_error("Failed to retrieve genkey results.\n");
        return 1;
    }

    strncpy(masterkey_fpr, result->fpr, 41);

    log_info("Generated masterkey fingerprint: %s\n", masterkey_fpr);

    return 0;
}

struct step {
    const char* request;
    const char* response;
};

struct edit_state {
    size_t cur_step;
    size_t max_step;
    size_t runs;
    size_t max_runs;
    struct step* steps;
};

gpgme_error_t edit_key_cb(void* handle,
                          gpgme_status_code_t status,
                          const char* args,
                          int fd)
{
    log_trace("edit_key_cb: status=%i args=%s\n", status, args);
    struct edit_state* state = (struct edit_state*)handle;

    state->runs++;
    if (state->runs > state->max_runs) {
        log_error("Reached max run threshold.\n");
        return 1;
    }

    // Nothing is expected from us here
    if (fd < 0)
        return 0;

    // If we reached the final step, then just leave
    if (state->cur_step >= state->max_step) {
        gpgme_io_write(fd, "quit\n", 5);
        return 0;
    }

    // This is not an interesting step
    if (strcmp(args, state->steps[state->cur_step].request))
        return 0;

    // Write the step response
    const char* response = state->steps[state->cur_step].response;
    gpgme_io_write(fd, response, strlen(response));
    gpgme_io_write(fd, "\n", 1);

    // Ready to move to next step
    state->cur_step++;

    return 0;
}

int generate_subkey_encrypt(struct gpgme_context* context, char* masterkey_fpr)
{
    log_info("Generating encryption subkey...\n");

    int err;
    gpgme_key_t key  = NULL;
    gpgme_data_t out = NULL;

    // Retrieve the masterkey
    if ((err = gpgme_op_keylist_start(context, masterkey_fpr, 0))) {
        log_error("Failed to list keys.\n");
        return err;
    }

    if ((err = gpgme_op_keylist_next(context, &key))) {
        log_error("Failed to list keys.\n");
        return err;
    }

    if ((err = gpgme_op_keylist_end(context))) {
        log_error("Failed to list keys.\n");
        return err;
    }

    if ((err = gpgme_data_new(&out))) {
        log_error("Failed to create new data.\n");
        return err;
    }

    struct step steps[] = {
        {"keyedit.prompt", "addkey"}, {"keygen.algo", "8"},
        {"keygen.flags", "s"},        {"keygen.flags", "e"},
        {"keygen.flags", "q"},        {"keygen.size", "2048"},
        {"keygen.valid", "0"},        {"keyedit.prompt", "save"},
    };
    struct edit_state state = {0, 8, 0, 250, steps};

    // Add encryption subkey
    if ((err = gpgme_op_edit(context, key, edit_key_cb, &state, out))) {
        log_error("Failed to edit masterkey.\n");
        return err;
    }

    return 0;
}

int export_masterkey(struct gpgme_context* context,
                     const char* temporary_keyring,
                     const char* passphrase,
                     char* masterkey_fpr)
{
    (void)(context);
    (void)(temporary_keyring);
    (void)(passphrase);
    (void)(masterkey_fpr);

    return 1;
}

int bootstrap(const char* username,
              const char* firstname,
              const char* lastname,
              const char* email,
              const char* passphrase)
{
    int err;
    struct gpgme_context* context = NULL;
    char masterkey_fpr[41];

    if ((err = check_gpgme()) != 0)
        return err;

    static char temporary_keyring[256];
    if (!mk_tmpdir(temporary_keyring, sizeof(temporary_keyring))) {
        log_error("Failed to create temporary keyring.\n");
        return 1;
    }
    log_debug("Using temporary keyring directory %s.\n", temporary_keyring);

    if ((err = setup_gpgme(&context, temporary_keyring)) != 0) {
        log_error("Step setup_gpgme failed.\n");
        goto cleanup;
    }

    if ((err = generate_masterkey(context, temporary_keyring, username,
                                  firstname, lastname, email, passphrase,
                                  masterkey_fpr)) != 0) {
        log_error("Step generate_masterkey failed.\n");
        goto cleanup;
    }

    if ((err = generate_subkey_encrypt(context, masterkey_fpr))) {
        log_error("Step generate_subkey_encrypt failed.\n");
        goto cleanup;
    }

    if ((err = export_masterkey(context, temporary_keyring, passphrase,
                                masterkey_fpr))) {
        log_error("Step export_masterkey failed.\n");
        goto cleanup;
    }

cleanup:
    rm_tmpdir(temporary_keyring);

    return err;
}
