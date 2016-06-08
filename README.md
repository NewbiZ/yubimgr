YubiMgr
---------
Bootstrap and manage YubiKeys OpenPGP/PIV applets.

YubiMgr automates most of the steps required to use YubiKeys in corporate
environments.

Namely, YubiMgr will allow you to:

- Generate a single OpenPGP master key (RSA/2048) for each user
- Export the master key to an offline storage (secured physical vault)
- Generate an encryption subkey (on the card)
- Generate an authentication subkey (on the card)
- Generate a signing subkey (on the card)
- Generate an SSH public key (from the authentication subkey)
- Generate an x509 CSR for use in your corporate PKI (from the authentication
  subkey)

Dependencies
------------
In order to build yubimgr, you will need:
- PGPME (https://www.gnupg.org/documentation/manuals/gpgme/)

Authors
-------
- Aurelien Vallee <vallee.aurelien@gmail.com>

Disclaimer
----------
YubiMgr is *NOT* meant to be run in an insecured environment. Clear text
passphrases are stored in the process memory space during key generation, files
are shredded whenever possible but non securely. You should only use this tool
from a secured, offline, computer.
Use at your own risks.

Licensed under MIT/X
--------------------
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
