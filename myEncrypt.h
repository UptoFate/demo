#pragma once 
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// 错误处理函数
void handleErrors() ;
// 从文件读取公钥
EVP_PKEY* loadPublicKey(const std::string& publicKeyPath);

// 从文件读取私钥
EVP_PKEY* loadPrivateKey(const std::string& privateKeyPath) ;

// 使用公钥加密
std::vector<unsigned char> rsaEncrypt(EVP_PKEY* pubKey, const std::string& plaintext);

// 使用私钥解密
std::string rsaDecrypt(EVP_PKEY* privKey, const std::vector<unsigned char>& ciphertext);