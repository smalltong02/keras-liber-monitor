#pragma once

#ifndef OSSLSIGNCODE_H_
#define OSSLSIGNCODE_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#include "jansson/include/jansson.h"
#include <openssl/ssl.h>
#include <openssl/pkcs7.h>

namespace osslsigncode
{

typedef std::pair<uint32_t, std::string> ErrorType;
typedef std::vector<ErrorType> ErrorVector;

namespace dserror
{

const ErrorType                 // error code, error message
    unknown                     (0x0000, "Unknown"),
    debug_info                  (0x0001, "Debug"),
    runtime_error               (0x0002, "Runtime error"),
    extraction_error            (0x0003, "Extraction error"),
    vfy_leafhash_parse          (0x1001, "PE verify error: unable to parse leaf hash parameter"),
    vfy_leafhash_lookup         (0x1011, "PE verify error: unable to lookup digest by name"),
    vfy_leafhash_length_uneven  (0x1021, "PE verify error: leaf hash length is uneven"),
    vfy_leafhash_length_mismatch(0x1022, "PE verify error: leaf hash length mismatch"),
    vfy_leafhash_value_mismatch (0x1031, "PE verify error: leaf hash value mismatch"),
    vfy_checksum_mismatch       (0x1101, "PE verify error: PE checksum mismatch"),
    vfy_pkcs7_extract           (0x1102, "PE verify error: failed to extract PKCS7 data"),
    vfy_nested_sig_decode       (0x1103, "PE verify error: failed to decode nested signature"),
    vfy_cert_store              (0x1104, "PE verify error: failed to store certificate"),
    vfy_msg_digest_extract      (0x1201, "PE verify error: failed to extract existing message digest"),
    vfy_msg_digest_undefined    (0x1202, "PE verify error: undefined message digest"),
    vfy_msg_digest_mismatch     (0x1203, "PE verify error: PE message digest mismatch"),
    vfy_page_hash_mismatch      (0x1204, "PE verify error: page hash mismatch"),
    vfy_msg_digest_get          (0x1205, "PE verify error: failed to get message digest algorithm"),
    vfy_page_hash_calc          (0x1206, "PE verify error: failed to calculate page hash"),
    vfy_countersig_no_auth      (0x1301, "PE verify error: countersignature has no authenticatedAttributes"),
    vfy_countersig_unknown_auth (0x1302, "PE verify error: countersignature contains unknown authenticatedAttribute"),
    vfy_countersig_no_alg       (0x1303, "PE verify error: countersignature has no digestAlgorithm"),
    vfy_countersig_invalid_alg  (0x1304, "PE verify error: countersignature has invalid digestAlgorithm"),
    vfy_countersig_no_msg_digest(0x1305, "PE verify error: countersignature has no message digest"),
    vfy_countersig_attr_count   (0x1306, "PE verify error: countersignature's message digest has invalid count"),
    vfy_countersig_hash_alg     (0x1307, "PE verify error: cannot find hashing algorithm for countersignature"),
    vfy_countersig_mismatch     (0x1308, "PE verify error: countersignature message digest mismatch"),
    struct_unknown_filetype     (0x2001, "PE structure error unrecognized file type"),
    struct_dos_short            (0x2002, "PE structure error corrupted DOS file (too short)"),
    struct_dos_unknown_filetype (0x2003, "PE structure error unrecognized DOS file type"),
    struct_pe_short             (0x2004, "PE structure error corrupted PE file (too short)"),
    struct_pe_unknown_magic     (0x2005, "PE structure error unknown magic"),
    struct_pe_missing_cert_table(0x2006, "PE structure error missing certificate table resource"),
    struct_pe_sig_none          (0x2011, "PE structure error no signature found"),
    struct_pe_sig_end           (0x2012, "PE structure error signature not at end of file"),
    struct_pe_sig_length        (0x2013, "PE structure error false signature length"),
    io_empty_file               (0x3001, "IO error: file size is 0 or too short"),
    io_cannot_map               (0x3002, "IO error: failed to map file"),
    io_trusted_not_set          (0x3003, "IO error: trusted JSON file not set"),
    io_trusted_corrupt          (0x3004, "IO error: corrupted trusted JSON file"),
    io_crl_not_set              (0x3005, "IO error: CRL store file not set"),
    io_crl_corrupt              (0x3006, "IO error: corrupted CRL store file"),
    init_set_intermediate       (0x4001, "Initialization error: failed to set intermediate certificates"),
    init_set_trusted            (0x4002, "Initialization error: failed to set trusted certificates"),
    init_set_crl_store          (0x4003, "Initialization error: failed to set CRL store"),
    init_load_components        (0x4004, "Initialization error: failed to load verification components");

std::string GenerateErrorMessage(ErrorType error, std::string details);

}  // namespace dserror


typedef std::string CounterSignType;

const CounterSignType
    kUnknown("unknown"),
    kRfc2985("rfc_2985"),
    kRfc3161("rfc_3161");


class PeFileInfo
{
public:
    PeFileInfo()
    {
        filesize_ = 0;
		fileend_ = 0;
        data_ = NULL;
		peheader_ = 0;
		magic_ = 0;
		pe32plus_ = 0;
		nrvas_ = 0;
		sigpos_ = 0;
		siglen_ = 0;
#ifdef _WIN32
		fm_ = NULL;
		fh_ = INVALID_HANDLE_VALUE;
#else
		fd_ = -1;
#endif
	}

    ~PeFileInfo()
    {
#ifdef _WIN32
        if (data_)
        {
            UnmapViewOfFile((void*)data_);
            data_ = NULL;
        }
        if (fm_)
        {
            CloseHandle(fm_);
            fm_ = NULL;
        }
        if (fh_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(fh_);
            fh_ = NULL;
        }
#else
        if (data_)
        {
            munmap((void*)data_, filesize_);
            data_ = NULL;
        }
        if (fd_ != -1)
        {
            close(fd_);
            fd_ = -1;
        }
#endif
    }

    off_t filesize_;
    off_t fileend_;
    char *data_;
    uint32_t peheader_;
    uint16_t magic_;
    int32_t pe32plus_;
    uint32_t nrvas_;
    uint32_t sigpos_;
    uint32_t siglen_;
#ifdef _WIN32
    HANDLE fm_;
    HANDLE fh_;
#else
    int fd_;
#endif
};

class SignInfo
{
public:
    struct certificate_info {
        bool brootcert = false;
        std::string version;
        std::string serial_number;
        std::string sign_algorithm;
        std::string sign_hash_algorithm;
        std::string issuer;
        std::string time_from;
        std::string time_to;
        std::string subject;
        std::string public_key;
        std::string public_key_algorithm;
        std::string public_key_param;
        std::string authority_key_iden;
        std::string subject_key_iden;
        std::string subject_alter_name;
        std::string enhanced_key_usage;
        std::string crl_distrubtion_points;
        std::string certificate_policies;
        std::string authority_information;
        std::string key_usage;
        std::string basic_constraints;
        std::string thumbprint;
        std::string issuer_thumbprint;
        std::string extended_error_info;
    };

    struct signer_info {
        bool bfindrootcert = false;
        std::string signer_name;
        std::string digest_algorithm;
        std::string timestamp;
        certificate_info issuer_info;
        std::vector<certificate_info> certificates;
    };

    bool crl_check_bypassed_;
    std::vector<signer_info> signers_;
    ErrorVector errors_;
    ErrorVector openssl_errors_;

    SignInfo()
    {
        crl_check_bypassed_ = false;
    }

    bool AddError(ErrorType error, std::string details = std::string());
    bool AddOpenSSLError(unsigned long error_code, std::string error_message);
};

class CounterSignInfo : public SignInfo
{
public:
    CounterSignType type_;
    bool is_verified_;

    CounterSignInfo() : SignInfo()
    {
        type_ = kUnknown;
        is_verified_ = false;
    }
};

class VerifyInfo : public SignInfo
{
public:
    bool has_signature_;
    bool is_verified_;

    bool has_nested_signatures_;
    bool use_nested_signatures_;
    std::vector<SignInfo> nested_signatures_;

    bool has_counter_signatures_;
    std::vector<CounterSignInfo> counter_signatures_;

    VerifyInfo() : SignInfo()
    {
        has_signature_ = false;
        is_verified_ = false;
        has_nested_signatures_ = false;
        use_nested_signatures_ = false;
        has_counter_signatures_ = false;
    }
};

class OpenSSLLoader
{
public:
    OpenSSLLoader();
    ~OpenSSLLoader();

private:
    static OpenSSLLoader m_smInstance;
};

class CertManager
{
private:
    json_t *intermediate_;
    json_t *trusted_;
    //CppSQLite3DB crl_store_db_;
    bool crl_store_initialized_;

    std::string err_msg_intermediate_;
    std::string err_msg_trusted_;
    std::string err_msg_crl_store_;

    std::string last_err_msg_;

    // Runtime variables
    bool crl_check_enabled_;
    bool store_certificate_;
    char *leaf_digest_name_;
    char *leaf_hash_;
    std::string certificate_;
    SSL_CTX* winroot_ctx_;
    std::map<std::string, SignInfo::certificate_info> winroot_certs_;

    void add_windows_root_certs();
    void add_winroot_cert_to_list(X509* cert);
    X509_STORE* get_windows_root_store() {
        if (!winroot_ctx_)
            return {};
        return SSL_CTX_get_cert_store(winroot_ctx_);
    }
    bool get_winroot_cert(std::vector<SignInfo::certificate_info>& cert_info);
    void BuildCertPath(SignInfo::certificate_info& sign_info, std::vector<SignInfo::certificate_info>& new_cert_info, std::vector<SignInfo::certificate_info>& cert_info);
    void ResetRuntimeVariables();

    bool LoadCerts(X509_STORE *store, json_t *certs);

    bool AnalyzePeStructure(PeFileInfo *pe, SignInfo &sinfo);
    int VerifyPeFile(VerifyInfo *vinfo, PeFileInfo& pe);
    bool StoreCertificate(PKCS7 *pkcs7_sig);
    bool VerifyCountersig(
        PKCS7_SIGNER_INFO *counter_si,
        ASN1_OCTET_STRING *encrypted_digest,
        ErrorType& error);
    void ProcessCountersig(VerifyInfo *vinfo, PKCS7 *pkcs7_sig);
    int VerifyPePkcs7(VerifyInfo*sinfo, PKCS7 *p7, char *indata,
        unsigned int peheader, int pe32plus, unsigned int sigpos, unsigned int siglen);
    int VerifyLeafHash(
        X509 *leaf,
        const char *leaf_digest_name,
        const char *leaf_hash,
        SignInfo* sinfo);

public:
    CertManager();
    ~CertManager();

    // Getter and setter for the latest error message
    std::string GetLastErrorMessage();
    void SetLastErrorMessage(std::string error_message);

    // Wrapper function which records an error on SignInfo object and
    // updates the latest error message simultaneously.
    void RecordError(SignInfo &sinfo, ErrorType error, std::string details = std::string());
    // Wrapper function which records an OpenSSL error on SignInfo object and
    // updates the latest error message simultaneously.
    void RecordOpenSSLError(SignInfo &sinfo, unsigned long error_code, std::string error_message);

    // Set the components needed by X509_STORE in verifying certificate
    bool SetIntermediate(const char* intermediate, bool is_file = false);
    bool SetCrlStore(const char* crl_store);

    // Verify certificate / digital signature of a PE file
    VerifyInfo Verify(
        const char *infile,
        bool crl_check = false,
        bool store_certificate = false,
        char *leaf_digest_name = NULL,
        char *leaf_hash = NULL);
    // Extract certificate (if it exists)
    int Extract(const char *infile, bool output_pem, const char *outfile = NULL);
    // Retrieve the certificate (if it exists) extracted in either of this call:
    // 1. Verify(), when 'store_certificate' is set to 'true'
    // 2. Extract(), when 'outfile' is set to 'NULL'
    std::string GetCertificate();
};

}  // namespace osslsigncode

#endif  // OSSLSIGNCODE_H_
