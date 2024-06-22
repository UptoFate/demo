#ifndef __MYSSL_H__
#define __MYSSL_H__

#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 1024

typedef enum{
    SSL_MODE_SERVER,
    SSL_MODE_CLIENT
}SSL_MOOD;

SSL *init_ssl(char* cert_path, char* key_path, SSL_MOOD mode, int fd);

#endif