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

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) handleErrors();

    size_t blockSize = EVP_PKEY_size(pubKey) - 11;  // RSA_PKCS1_PADDING会占用11个字节
    std::vector<unsigned char> ciphertext;
    size_t offset = 0;

    while (offset < plaintext.size()) {
        size_t chunkSize = std::min(blockSize, plaintext.size() - offset);
        size_t outlen;
        
        if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (const unsigned char*)&plaintext[offset], chunkSize) <= 0) handleErrors();
        
        size_t current_size = ciphertext.size();
        ciphertext.resize(current_size + outlen);
        
        if (EVP_PKEY_encrypt(ctx, &ciphertext[current_size], &outlen, (const unsigned char*)&plaintext[offset], chunkSize) <= 0) handleErrors();
        
        ciphertext.resize(current_size + outlen);
        offset += chunkSize;
    }

    EVP_PKEY_CTX_free(ctx);
    return ciphertext;
}

// 使用私钥解密
std::string rsaDecrypt(EVP_PKEY* privKey, const std::vector<unsigned char>& ciphertext) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privKey, nullptr);
    if (!ctx) handleErrors();

    if (EVP_PKEY_decrypt_init(ctx) <= 0) handleErrors();

    size_t blockSize = EVP_PKEY_size(privKey);
    std::vector<unsigned char> plaintext;
    size_t offset = 0;

    while (offset < ciphertext.size()) {
        size_t chunkSize = std::min(blockSize, ciphertext.size() - offset);
        size_t outlen;
        
        if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, &ciphertext[offset], chunkSize) <= 0) handleErrors();
        
        size_t current_size = plaintext.size();
        plaintext.resize(current_size + outlen);
        
        if (EVP_PKEY_decrypt(ctx, &plaintext[current_size], &outlen, &ciphertext[offset], chunkSize) <= 0) handleErrors();
        
        plaintext.resize(current_size + outlen);
        offset += chunkSize;
    }

    EVP_PKEY_CTX_free(ctx);
    return std::string(plaintext.begin(), plaintext.end());
}

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.length());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

void generate_random_bytes(unsigned char* buffer, int length) {
    if (!RAND_bytes(buffer, length)) {
        std::cerr << "Failed to generate random bytes" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// AES CBF加密
std::vector<unsigned char> aes_encrypt(const unsigned char* key, const unsigned char* iv, const std::string& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb(), nullptr, key, iv) != 1) handleErrors();

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
    int len, ciphertext_len;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (unsigned char*)plaintext.c_str(), plaintext.size()) != 1) handleErrors();
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) handleErrors();
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

// AES CBF解密
std::string aes_decrypt(const unsigned char* key, const unsigned char* iv, const std::vector<unsigned char>& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb(), nullptr, key, iv) != 1) handleErrors();

    std::vector<unsigned char> plaintext(ciphertext.size());
    int len, plaintext_len;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) handleErrors();
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) handleErrors();
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);

    return std::string(plaintext.begin(), plaintext.end());
}