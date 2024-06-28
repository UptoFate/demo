#include "myEncrypt.h"
// 错误处理函数
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

// 从文件读取公钥
EVP_PKEY* loadPublicKey(const std::string& publicKeyPath) {
    FILE* fp = fopen(publicKeyPath.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Unable to open public key file\n";
        return nullptr;
    }

    EVP_PKEY* pubKey = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (pubKey == nullptr) {
        std::cerr << "Unable to read public key\n";
        handleErrors();
    }

    return pubKey;
}

// 从文件读取私钥
EVP_PKEY* loadPrivateKey(const std::string& privateKeyPath) {
    FILE* fp = fopen(privateKeyPath.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Unable to open private key file\n";
        return nullptr;
    }

    EVP_PKEY* privKey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (privKey == nullptr) {
        std::cerr << "Unable to read private key\n";
        handleErrors();
    }

    return privKey;
}

// 使用公钥加密
std::vector<unsigned char> rsaEncrypt(EVP_PKEY* pubKey, const std::string& plaintext) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubKey, nullptr);
    if (!ctx) handleErrors();

    if (EVP_PKEY_encrypt_init(ctx) <= 0) handleErrors();

    // 设置填充方式为 RSA_PKCS1_PADDING
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) handleErrors();
    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (const unsigned char*)plaintext.c_str(), plaintext.size()) <= 0)
        handleErrors();

    std::vector<unsigned char> ciphertext(outlen);
    if (EVP_PKEY_encrypt(ctx, ciphertext.data(), &outlen, (const unsigned char*)plaintext.c_str(), plaintext.size()) <= 0)
        handleErrors();

    ciphertext.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return ciphertext;
}

// 使用私钥解密
std::string rsaDecrypt(EVP_PKEY* privKey, const std::vector<unsigned char>& ciphertext) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privKey, nullptr);
    if (!ctx) handleErrors();

    if (EVP_PKEY_decrypt_init(ctx) <= 0) handleErrors();

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        handleErrors();

    std::vector<unsigned char> plaintext(outlen);
    if (EVP_PKEY_decrypt(ctx, plaintext.data(), &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        handleErrors();

    plaintext.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return std::string(plaintext.begin(), plaintext.end());
}