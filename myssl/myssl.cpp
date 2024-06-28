#include "myssl.h"

SSL *init_ssl(char* cert_path, char* key_path, SSL_MOOD mode, int fd){
    const SSL_METHOD* method;
    SSL_CTX *ctx;
    SSL* ssl = NULL;

    OpenSSL_add_all_algorithms();   
    SSL_load_error_strings();

    if(mode == SSL_MODE_SERVER)
    {
        method = SSLv23_server_method();
    }
    else if(mode == SSL_MODE_CLIENT)
    {
        method = SSLv23_client_method();
    }
    else
    {
        printf("Not found method.\n");
    }

    ctx = SSL_CTX_new(method);
    if(!ctx){
        printf("Unable to create SSL ctx.\n");
        return 0;
    }
    
    //生成公钥  openssl genpkey -algorithm RSA -out key.pem
    //用公钥生成私钥    
    //加载公钥和私钥
    if(SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM)<=0 ||
    SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM)<=0)
    {
        printf("Not found cert file or private key file.\n");
        return 0;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
    ssl = SSL_new(ctx);
    if(!ssl){
        printf("Failed to create SSL object.\n");
        return 0;
    }

    //将文件描述符与openssl关联
    if(SSL_set_fd(ssl, fd)==0){
        printf("Failed to set fd to SSL object.\n");
        return 0;
    }

    if(mode == SSL_MODE_SERVER && SSL_accept(ssl)<=0){
        printf("failed to handshake.\n");
        perror("accept");
        printf("%d",fd);
        return 0;
    }
    printf("11111111\n");
    return ssl;
}
