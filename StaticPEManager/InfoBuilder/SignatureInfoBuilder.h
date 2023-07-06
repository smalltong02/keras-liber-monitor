#pragma once
#include "InfoBuilder.h"
#include "osslsigncode.h"

namespace cchips {
    class CSignatureInfoBuilder : public CInfoBuilder
    {
    public:
        CSignatureInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_signature); }
        ~CSignatureInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) {
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            return true;
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) { 
            try {
                VerifyInfo digisig_info = m_cert_manager.Verify(to_byte_string(file_path.wstring()).c_str());
                for (auto& error : digisig_info.openssl_errors_) {
                    std::cout << error.second << std::endl;
                }
                std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
                json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
                if (!json_result)
                    return false;
                json_result->AddTopMember("bsigned", cchips::RapidValue(digisig_info.has_signature_));
                json_result->AddTopMember("bverified", cchips::RapidValue(digisig_info.is_verified_));
                json_result->AddTopMember("bcountersign", cchips::RapidValue(digisig_info.has_counter_signatures_));
                json_result->AddTopMember("bmutilcerts", cchips::RapidValue(digisig_info.has_nested_signatures_));
                cchips::RapidDocument::AllocatorType& allocator = json_result->GetAllocator();
                if (digisig_info.signers_.size() >= 1) {
                    auto& signer = digisig_info.signers_[0];
                    if (signer.signer_name.length()) {
                        std::unique_ptr<cchips::RapidValue> primary_cert = std::make_unique<cchips::RapidValue>();
                        if (!primary_cert) return false;
                        primary_cert->SetObject();
                        AddVerifyLog(primary_cert, signer, allocator);
                        json_result->AddTopMember("primarycert", std::move(primary_cert));
                    }
                }
                if (digisig_info.counter_signatures_.size()) {
                    for (auto& counter : digisig_info.counter_signatures_) {
                        std::unique_ptr<cchips::RapidValue> countersign_cert = std::make_unique<cchips::RapidValue>();
                        if (!countersign_cert) return false;
                        countersign_cert->SetObject();
                        countersign_cert->AddMember("bverified", counter.is_verified_, allocator);
                        if (counter.signers_.size() >= 1) {
                            auto& signer = counter.signers_[0];
                            if (signer.signer_name.length()) {
                                AddVerifyLog(countersign_cert, signer, allocator);
                            }
                        }
                        json_result->AddTopMember("countersign_path", std::move(countersign_cert));
                        break;
                    }
                }
                if (digisig_info.nested_signatures_.size()) {
                    for (auto& nested : digisig_info.nested_signatures_) {
                        std::stringstream nested_ss;
                        nested_ss << "second_cert_path";
                        std::unique_ptr<cchips::RapidValue> nested_cert = std::make_unique<cchips::RapidValue>();
                        if (!nested_cert) return false;
                        nested_cert->SetObject();
                        if (nested.signers_.size() >= 1) {
                            auto& signer = nested.signers_[0];
                            if (signer.signer_name.length()) {
                                AddVerifyLog(nested_cert, signer, allocator);
                            }
                        }
                        json_result->AddTopMember("nested_cert_path", std::move(nested_cert));
                        break;
                    }
                }
                file_info.SetJsonVerifyInfo(std::move(json_result));
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }
    private:
        void AddVerifyLog(std::unique_ptr<cchips::RapidValue>& certarray, SignInfo::signer_info& signer, cchips::RapidDocument::AllocatorType& allocator)
        {
            if (!certarray) return;
            certarray->SetArray();
            // add primary cert
            {
                cchips::RapidValue cert_docment;
                cert_docment.SetObject();
                {
                    cchips::RapidValue key("bprimary", allocator);
                    cchips::RapidValue value(true);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.digest_algorithm.length()) {
                    cchips::RapidValue key("digest", allocator);
                    cchips::RapidValue value(signer.digest_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.issuer.length()) {
                    cchips::RapidValue key("issuer", allocator);
                    cchips::RapidValue value(signer.issuer_info.issuer.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.subject.length()) {
                    cchips::RapidValue key("subject", allocator);
                    cchips::RapidValue value(signer.issuer_info.subject.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.serial_number.length()) {
                    cchips::RapidValue key("serialbn", allocator);
                    cchips::RapidValue value(signer.issuer_info.serial_number.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.time_from.length()) {
                    cchips::RapidValue key("time_from", allocator);
                    cchips::RapidValue value(signer.issuer_info.time_from.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.time_to.length()) {
                    cchips::RapidValue key("time_to", allocator);
                    cchips::RapidValue value(signer.issuer_info.time_to.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.sign_algorithm.length()) {
                    cchips::RapidValue key("sign_algorithm", allocator);
                    cchips::RapidValue value(signer.issuer_info.sign_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.sign_hash_algorithm.length()) {
                    cchips::RapidValue key("hash_algorithm", allocator);
                    cchips::RapidValue value(signer.issuer_info.sign_hash_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.public_key_algorithm.length()) {
                    cchips::RapidValue key("key_algorithm", allocator);
                    cchips::RapidValue value(signer.issuer_info.public_key_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (signer.issuer_info.thumbprint.length()) {
                    cchips::RapidValue key("thumbprint", allocator);
                    cchips::RapidValue value(signer.issuer_info.thumbprint.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                certarray->PushBack(cert_docment, allocator);
            }

            for (auto& cert : signer.certificates) {
                cchips::RapidValue cert_docment;

                cert_docment.SetObject();
                {
                    cchips::RapidValue key("brootcert", allocator);
                    cchips::RapidValue value(cert.brootcert);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.issuer.length()) {
                    cchips::RapidValue key("issuer", allocator);
                    cchips::RapidValue value(cert.issuer.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.subject.length()) {
                    cchips::RapidValue key("subject", allocator);
                    cchips::RapidValue value(cert.subject.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.serial_number.length()) {
                    cchips::RapidValue key("serialbn", allocator);
                    cchips::RapidValue value(cert.serial_number.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.time_from.length()) {
                    cchips::RapidValue key("time_from", allocator);
                    cchips::RapidValue value(cert.time_from.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.time_to.length()) {
                    cchips::RapidValue key("time_to", allocator);
                    cchips::RapidValue value(cert.time_to.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.sign_algorithm.length()) {
                    cchips::RapidValue key("sign_algorithm", allocator);
                    cchips::RapidValue value(cert.sign_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.sign_hash_algorithm.length()) {
                    cchips::RapidValue key("hash_algorithm", allocator);
                    cchips::RapidValue value(cert.sign_hash_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.public_key_algorithm.length()) {
                    cchips::RapidValue key("key_algorithm", allocator);
                    cchips::RapidValue value(cert.public_key_algorithm.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                if (cert.thumbprint.length()) {
                    cchips::RapidValue key("thumbprint", allocator);
                    cchips::RapidValue value(cert.thumbprint.c_str(), allocator);
                    cert_docment.AddMember(key, value, allocator);
                }
                certarray->PushBack(cert_docment, allocator);
            }
            return;
        }

        CJsonOptions::_signature_info m_options_info;
        CertManager m_cert_manager;
    };
} // namespace cchips
