#pragma once
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <iostream>
#include <memory>

EVP_PKEY *genKey();
X509 * genX509(EVP_PKEY * pkey);
bool writeToDisk(EVP_PKEY * pkey, X509 * x509);
