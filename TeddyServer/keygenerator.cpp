#include "keygenerator.h"

EVP_PKEY *genKey() {
    EVP_PKEY * pkey = EVP_PKEY_new();
    if(!pkey)
    {
        std::cerr << "Unable to create EVP_PKEY structure." << std::endl;
        return NULL;
    }

    auto primeNumber = BN_new();
    if (primeNumber == NULL){
        std::cerr << "Prime number not generated";
        EVP_PKEY_free(pkey);
        return NULL;
    }
    RSA* rsa = RSA_new();
    if (rsa == NULL){
        EVP_PKEY_free(pkey);
        BN_free(primeNumber);
        std::cerr << "No memory allocated for RSA";
        return NULL;
    }
    if (!BN_set_word(primeNumber, RSA_F4) || !RSA_generate_key_ex(rsa, 2048, primeNumber, NULL)){
        std::cerr << "Unable to generate 2048-bit RSA key." << std::endl;
        EVP_PKEY_free(pkey);
        BN_free(primeNumber);
        return NULL;
    }
    if(!EVP_PKEY_assign_RSA(pkey, rsa))
    {
        std::cerr << "Unable to assign 2048-bit RSA key." << std::endl;
        EVP_PKEY_free(pkey);
        BN_free(primeNumber);
        return NULL;
    }
    BN_free(primeNumber);
    return pkey;
}

X509 * genX509(EVP_PKEY * pkey) {
    X509 * x509 = X509_new();
    if(!x509)
    {
        std::cerr << "Unable to create X509 structure." << std::endl;
        return NULL;
    }
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);
    X509_set_pubkey(x509, pkey);
    X509_NAME * name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)"MyCompany", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"localhost", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    if(!X509_sign(x509, pkey, EVP_sha1()))
    {
        std::cerr << "Error signing certificate." << std::endl;
        X509_free(x509);
        return NULL;
    }
    return x509;
}

bool writeToDisk(EVP_PKEY * pkey, X509 * x509) {
    FILE * pkey_file = fopen("key.pem", "wb");
    if(!pkey_file)
    {
        std::cerr << "Unable to open \"key.pem\" for writing." << std::endl;
        return false;
    }
    bool ret = PEM_write_PrivateKey(pkey_file, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(pkey_file);
    if(!ret)
    {
        std::cerr << "Unable to write private key to disk." << std::endl;
        return false;
    }
    FILE * x509_file = fopen("cert.pem", "wb");
    if(!x509_file)
    {
        std::cerr << "Unable to open \"cert.pem\" for writing." << std::endl;
        return false;
    }
    ret = PEM_write_X509(x509_file, x509);
    fclose(x509_file);
    if(!ret)
    {
        std::cerr << "Unable to write certificate to disk." << std::endl;
        return false;
    }
    return true;
}
