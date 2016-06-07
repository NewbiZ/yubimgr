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

int setup_gpgme(gpgme_ctx_t context)
{
    gpgme_error_t err;

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

// Warning: this function is not reentrant.
const char* mk_tmpdir()
{
    const char* tmprootdir        = getenv("TMPDIR");
    static const char* tmpnametpl = "tmp.yubimgr.XXXXXX";

    if (!tmprootdir)
        tmprootdir = "/tmp";

    static char tmptpldir[256] = {0};
    if (strlen(tmprootdir) + strlen(tmpnametpl) + 1 > sizeof(tmptpldir) - 1) {
        log_error("Temporary directory name too long. Check $TMPDIR.");
        return NULL;
    }

    strncat(tmptpldir, tmprootdir, sizeof(tmptpldir) - 1);
    strncat(tmptpldir, "/", 1);
    strncat(tmptpldir, tmpnametpl, sizeof(tmptpldir) - strlen(tmptpldir) - 1);

    const char* tmpdir = mkdtemp(tmptpldir);

    return tmpdir;
}

int unlink_cb(const char* fpath,
              const struct stat __attribute__((unused)) * sb,
              int __attribute__((unused)) typeflag,
              struct FTW __attribute__((unused)) * ftwbuf)
{
    return remove(fpath);
}

void rm_tmpdir(const char* path)
{
    log_debug("Removing temporary keyring %s.\n", path);
    nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int bootstrap()
{
    int err;
    gpgme_ctx_t context = NULL;

    if ((err = check_gpgme()) != 0)
        return err;

    const char* temporary_keyring;
    if ((temporary_keyring = mk_tmpdir()) == NULL) {
        log_error("Failed to create temporary keyring.\n");
        return 1;
    }
    log_debug("Using temporary keyring directory %s.\n", temporary_keyring);

    if ((err = setup_gpgme(context)) != 0)
        goto cleanup;

cleanup:
    rm_tmpdir(temporary_keyring);

    return err;
}
