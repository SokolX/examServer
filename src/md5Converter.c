/**
 * \file md5Converter.c
 * \brief Plik zawierający metodę *str2md5, która po stronie Servera odpowiada
 * za generowanie hash'y md5. 
 */

#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define SHA1 CC_SHA1
#else
#include <openssl/md5.h>
#endif

/**
 * Metoda generuje skróty MD5 podanych ciągów znaków.
 * 
 * @param str   Ciąg znaków, dla którego MD5 ma zostać wygenerowany.
 * @return      Skrót w postaci MD5.
 */
char *str2md5(char *str) {
    int n;
    int length = strlen(str);
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}