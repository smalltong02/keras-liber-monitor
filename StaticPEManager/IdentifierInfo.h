#pragma once
#include <string>
#include <filesystem>
#include "md5.h"
#include "openssl/sha.h"
#include "infobuilder\peinside\stringutils.h"

namespace fs = std::filesystem;

class CIdentifierInfo
{
public:
    using _identifier_type = enum {
        identifier_unknown,
        identifier_md5,
        identifier_sha1,
        identifier_sha256
    };

    CIdentifierInfo() = default;
    CIdentifierInfo(const fs::path& path, _identifier_type type) { 
        Initialize(path, type);
    }
    CIdentifierInfo(const std::string& buffer, _identifier_type type) {
        Initialize(buffer, type);
    }
    ~CIdentifierInfo() = default;

    bool Initialize(const std::string& buffer, _identifier_type type) {
        std::string identifier = CalculateIdentifier(buffer, type);
        if (!identifier.length())
            return false;
        m_identifier = identifier;
        m_type = type;
        return true;
    }

    bool Initialize(const fs::path& path, _identifier_type type) {
        std::ifstream infile;
        infile.open(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!infile.is_open()) return false;
        int file_length = (int)infile.tellg();
        if (!file_length) return false;
        std::string buffer;
        buffer.resize(file_length);
        if (buffer.size() != file_length) {
            return false;
        }
        infile.seekg(0, std::ios::beg);
        infile.read(&buffer[0], file_length);
        auto readed = (int)infile.tellg();
        if (readed != file_length) {
            return false;
        }
        return Initialize(buffer, type);
    }

    const std::string& GetIdentifier() const { return m_identifier; }

private:
    std::string CalculateIdentifier(const std::string& buffer, _identifier_type type)
    {
        std::vector<BYTE> iden_ctx;
        auto method_init = [](std::vector<BYTE>& iden_ctx, _identifier_type type) ->bool {
            if (type == identifier_md5) {
                iden_ctx.resize(sizeof(MD5_CTX));
                MD5_CTX* ctx_ptr = reinterpret_cast<MD5_CTX*>(&iden_ctx[0]);
                MD5Init(ctx_ptr);
                return true;
            }
            else if (type == identifier_sha1) {
                iden_ctx.resize(sizeof(SHA_CTX));
                SHA_CTX* ctx_ptr = reinterpret_cast<SHA_CTX*>(&iden_ctx[0]);
                SHA1_Init(ctx_ptr);
                return true;
            }
            else if (type == identifier_sha256) {
                iden_ctx.resize(sizeof(SHA256_CTX));
                SHA256_CTX* ctx_ptr = reinterpret_cast<SHA256_CTX*>(&iden_ctx[0]);
                SHA256_Init(ctx_ptr);
                return true;
            }
            return false;
        };
        auto method_update = [](std::vector<BYTE>& iden_ctx, _identifier_type type, unsigned char* buffer, unsigned int buffer_size) ->bool {
            if (!buffer || buffer_size == 0)
                return false;
            if (type == identifier_md5 && iden_ctx.size() == sizeof(MD5_CTX)) {
                MD5_CTX* ctx_ptr = reinterpret_cast<MD5_CTX*>(&iden_ctx[0]);
                MD5Update(ctx_ptr, buffer, buffer_size);
                return true;
            }
            else if (type == identifier_sha1 && iden_ctx.size() == sizeof(SHA_CTX)) {
                SHA_CTX* ctx_ptr = reinterpret_cast<SHA_CTX*>(&iden_ctx[0]);
                SHA1_Update(ctx_ptr, buffer, buffer_size);
                return true;
            }
            else if (type == identifier_sha256 && iden_ctx.size() == sizeof(SHA256_CTX)) {
                SHA256_CTX* ctx_ptr = reinterpret_cast<SHA256_CTX*>(&iden_ctx[0]);
                SHA256_Update(ctx_ptr, buffer, buffer_size);
                return true;
            }
            return false;
        };
        auto method_final = [](std::vector<BYTE>& iden_ctx, _identifier_type type) ->std::string {
            if (type == identifier_md5 && iden_ctx.size() == sizeof(MD5_CTX)) {
                MD5_CTX* ctx_ptr = reinterpret_cast<MD5_CTX*>(&iden_ctx[0]);
                MD5Final(ctx_ptr);
                return HexEncode(ctx_ptr->digest, 16);
            }
            else if (type == identifier_sha1 && iden_ctx.size() == sizeof(SHA_CTX)) {
                unsigned char digest[SHA_DIGEST_LENGTH];
                SHA_CTX* ctx_ptr = reinterpret_cast<SHA_CTX*>(&iden_ctx[0]);
                SHA1_Final(digest, ctx_ptr);
                return HexEncode(digest, SHA_DIGEST_LENGTH);
            }
            else if (type == identifier_sha256 && iden_ctx.size() == sizeof(SHA256_CTX)) {
                unsigned char digest[SHA256_DIGEST_LENGTH];
                SHA256_CTX* ctx_ptr = reinterpret_cast<SHA256_CTX*>(&iden_ctx[0]);
                SHA256_Final(digest, ctx_ptr);
                return HexEncode(digest, SHA256_DIGEST_LENGTH);
            }
            return {};
        };
        if (buffer.empty()) {
            return {};
        }
        if (!method_init(iden_ctx, type)) return {};
        method_update(iden_ctx, type, (unsigned char*)&buffer[0], static_cast<unsigned int>(buffer.size()));
        return method_final(iden_ctx, type);
    }

    _identifier_type m_type{ identifier_unknown };
    std::string m_identifier;
};
