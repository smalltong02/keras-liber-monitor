/*
   OpenSSL based Authenticode utilities for PE files.

   Copyright (C) 2005-2015 Per Allansson <pallansson@gmail.com>


   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   In addition, as a special exception, the copyright holders give
   permission to link the code of portions of this program with the
   OpenSSL library under certain conditions as described in each
   individual source file, and distribute linked combinations
   including the two.
   You must obey the GNU General Public License in all respects
   for all of the code used other than OpenSSL.  If you modify
   file(s) with this exception, you may extend this exception to your
   version of the file(s), but you are not obligated to do so.  If you
   do not wish to do so, delete this exception statement from your
   version.  If you delete this exception statement from all source
   files in the program, then also delete it here.
*/

//#include "stdafx.h"

static const char *rcsid = "$Id: osslsigncode.c,v 1.7.1 2014/07/11 14:14:14 mfive Exp $";

/*
   Implemented with good help from:

   * Peter Gutmann's analysis of Authenticode:

      http://www.cs.auckland.ac.nz/~pgut001/pubs/authenticode.txt

   * MS CAB SDK documentation

      http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dncabsdk/html/cabdl.asp

   * MS PE/COFF documentation

      http://www.microsoft.com/whdc/system/platform/firmware/PECOFF.mspx

   * MS Windows Authenticode PE Signature Format

      http://msdn.microsoft.com/en-US/windows/hardware/gg463183

      (Although the part of how the actual checksumming is done is not
      how it is done inside Windows. The end result is however the same
      on all "normal" PE files.)

   * tail -c, tcpdump, mimencode & openssl asn1parse :)

*/

#include "osslsigncode.h"

#ifdef HAVE_WINDOWS_H
#define NOCRYPT
#define WIN32_LEAN_AND_MEAN
typedef unsigned char u_char;
#endif

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#endif

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>

//#include <jansson.h>

#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/asn1t.h>
#include <openssl/conf.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/ossl_typ.h>
#include <openssl/stack.h>
#include <openssl/safestack.h>

using namespace osslsigncode;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef _WIN32
#define mkgmtime _mkgmtime
#else
#define mkgmtime timegm  // Might not be available in some Linux distributions
#endif

// MS Authenticode object ids
#define SPC_INDIRECT_DATA_OBJID	 "1.3.6.1.4.1.311.2.1.4"
#define SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.21"
#define SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.22"
#define SPC_PE_IMAGE_DATA_OBJID	 "1.3.6.1.4.1.311.2.1.15"
#define SPC_CAB_DATA_OBJID		 "1.3.6.1.4.1.311.2.1.25"
#define SPC_TIME_STAMP_REQUEST_OBJID "1.3.6.1.4.1.311.3.2.1"
#define SPC_SIPINFO_OBJID		 "1.3.6.1.4.1.311.2.1.30"

#define SPC_PE_IMAGE_PAGE_HASHES_V1 "1.3.6.1.4.1.311.2.3.1"  // Page hash using SHA1
#define SPC_PE_IMAGE_PAGE_HASHES_V2 "1.3.6.1.4.1.311.2.3.2"  // Page hash using SHA256

#define SPC_NESTED_SIGNATURE_OBJID  "1.3.6.1.4.1.311.2.4.1"

#define SPC_RFC2985_OBJID "1.2.840.113549.1.9.6"  // RFC 2985 Countersignature
#define SPC_RFC3161_OBJID "1.3.6.1.4.1.311.3.3.1"  // RFC 3161 Timestamp Countersignature

#define SPC_AUTH_ATTR_CONTENT_TYPE "1.2.840.113549.1.9.3"
#define SPC_AUTH_ATTR_MESSAGE_DIGEST "1.2.840.113549.1.9.4"
#define SPC_AUTH_ATTR_SIGNING_TIME "1.2.840.113549.1.9.5"

// 1.3.6.1.4.1.311.4... MS Crypto 2.0 stuff...

#define WIN_CERT_REVISION_2             0x0200
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA  0x0002

#define MAX_CN_LEN 256
#define MAX_NESTED_SIGS 8

// NOTE: Use this static instance to make sure OpenSSL only initialized once.
//       Otherwise we need to use static locks for multi-threading.
OpenSSLLoader OpenSSLLoader::m_smInstance;

static const std::vector<std::string> kAuthAttributes = {
    SPC_AUTH_ATTR_CONTENT_TYPE,
    SPC_AUTH_ATTR_MESSAGE_DIGEST,
    SPC_AUTH_ATTR_SIGNING_TIME
};

/******************************************************************************
 * ASN.1 Definitions (more or less from official MS Authenticode docs)
 */

typedef struct
{
    int type;
    union
    {
        ASN1_BMPSTRING *unicode;
        ASN1_IA5STRING *ascii;
    } value;
} SpcString;

DECLARE_ASN1_FUNCTIONS(SpcString)

ASN1_CHOICE(SpcString) = {
    ASN1_IMP_OPT(SpcString, value.unicode, ASN1_BMPSTRING , 0),
    ASN1_IMP_OPT(SpcString, value.ascii,   ASN1_IA5STRING,	1)
} ASN1_CHOICE_END(SpcString)

IMPLEMENT_ASN1_FUNCTIONS(SpcString)


typedef struct
{
    ASN1_OCTET_STRING *classId;
    ASN1_OCTET_STRING *serializedData;
} SpcSerializedObject;

DECLARE_ASN1_FUNCTIONS(SpcSerializedObject)

ASN1_SEQUENCE(SpcSerializedObject) = {
    ASN1_SIMPLE(SpcSerializedObject, classId, ASN1_OCTET_STRING),
    ASN1_SIMPLE(SpcSerializedObject, serializedData, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(SpcSerializedObject)

IMPLEMENT_ASN1_FUNCTIONS(SpcSerializedObject)


typedef struct
{
    int type;
    union
    {
        ASN1_IA5STRING *url;
        SpcSerializedObject *moniker;
        SpcString *file;
    } value;
} SpcLink;

DECLARE_ASN1_FUNCTIONS(SpcLink)

ASN1_CHOICE(SpcLink) = {
    ASN1_IMP_OPT(SpcLink, value.url,	 ASN1_IA5STRING,	  0),
    ASN1_IMP_OPT(SpcLink, value.moniker, SpcSerializedObject, 1),
    ASN1_EXP_OPT(SpcLink, value.file,	 SpcString,			  2)
} ASN1_CHOICE_END(SpcLink)

IMPLEMENT_ASN1_FUNCTIONS(SpcLink)


typedef struct
{
    ASN1_OBJECT *type;
    ASN1_TYPE *value;
} SpcAttributeTypeAndOptionalValue;

DECLARE_ASN1_FUNCTIONS(SpcAttributeTypeAndOptionalValue)

ASN1_SEQUENCE(SpcAttributeTypeAndOptionalValue) = {
    ASN1_SIMPLE(SpcAttributeTypeAndOptionalValue, type, ASN1_OBJECT),
    ASN1_OPT(SpcAttributeTypeAndOptionalValue, value, ASN1_ANY)
} ASN1_SEQUENCE_END(SpcAttributeTypeAndOptionalValue)

IMPLEMENT_ASN1_FUNCTIONS(SpcAttributeTypeAndOptionalValue)


typedef struct
{
    ASN1_OBJECT *algorithm;
    ASN1_TYPE *parameters;
} AlgorithmIdentifier;

DECLARE_ASN1_FUNCTIONS(AlgorithmIdentifier)

ASN1_SEQUENCE(AlgorithmIdentifier) = {
    ASN1_SIMPLE(AlgorithmIdentifier, algorithm, ASN1_OBJECT),
    ASN1_OPT(AlgorithmIdentifier, parameters, ASN1_ANY)
} ASN1_SEQUENCE_END(AlgorithmIdentifier)

IMPLEMENT_ASN1_FUNCTIONS(AlgorithmIdentifier)


typedef struct
{
    AlgorithmIdentifier *digestAlgorithm;
    ASN1_OCTET_STRING *digest;
} DigestInfo;

DECLARE_ASN1_FUNCTIONS(DigestInfo)

ASN1_SEQUENCE(DigestInfo) = {
    ASN1_SIMPLE(DigestInfo, digestAlgorithm, AlgorithmIdentifier),
    ASN1_SIMPLE(DigestInfo, digest, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(DigestInfo)

IMPLEMENT_ASN1_FUNCTIONS(DigestInfo)


typedef struct
{
    SpcAttributeTypeAndOptionalValue *data;
    DigestInfo *messageDigest;
} SpcIndirectDataContent;

DECLARE_ASN1_FUNCTIONS(SpcIndirectDataContent)

ASN1_SEQUENCE(SpcIndirectDataContent) = {
    ASN1_SIMPLE(SpcIndirectDataContent, data, SpcAttributeTypeAndOptionalValue),
    ASN1_SIMPLE(SpcIndirectDataContent, messageDigest, DigestInfo)
} ASN1_SEQUENCE_END(SpcIndirectDataContent)

IMPLEMENT_ASN1_FUNCTIONS(SpcIndirectDataContent)


typedef struct
{
    ASN1_BIT_STRING* flags;
    SpcLink *file;
} SpcPeImageData;

DECLARE_ASN1_FUNCTIONS(SpcPeImageData)

ASN1_SEQUENCE(SpcPeImageData) = {
    ASN1_SIMPLE(SpcPeImageData, flags, ASN1_BIT_STRING),
    ASN1_EXP_OPT(SpcPeImageData, file, SpcLink, 0)
} ASN1_SEQUENCE_END(SpcPeImageData)

IMPLEMENT_ASN1_FUNCTIONS(SpcPeImageData)


#define GET_UINT16_LE(p) (((u_char*)(p))[0] | (((u_char*)(p))[1]<<8))

#define GET_UINT32_LE(p) (((u_char*)(p))[0] | (((u_char*)(p))[1]<<8) |    \
                          (((u_char*)(p))[2]<<16) | (((u_char*)(p))[3]<<24))

#define PUT_UINT16_LE(i,p)                        \
    ((u_char*)(p))[0] = (i) & 0xff;                \
    ((u_char*)(p))[1] = ((i)>>8) & 0xff

#define PUT_UINT32_LE(i,p)                        \
    ((u_char*)(p))[0] = (i) & 0xff;                \
    ((u_char*)(p))[1] = ((i)>>8) & 0xff;        \
    ((u_char*)(p))[2] = ((i)>>16) & 0xff;        \
    ((u_char*)(p))[3] = ((i)>>24) & 0xff

static const unsigned char classid_page_hash[] = {
    0xA6, 0xB5, 0x86, 0xD5, 0xB4, 0xA1, 0x24, 0x66,
    0xAE, 0x05, 0xA2, 0x17, 0xDA, 0x8E, 0x60, 0xD6
};

DEFINE_STACK_OF(ASN1_OCTET_STRING)
#ifndef sk_ASN1_OCTET_STRING_new_null
#define sk_ASN1_OCTET_STRING_new_null() SKM_sk_new_null(ASN1_OCTET_STRING)
#define sk_ASN1_OCTET_STRING_free(st) SKM_sk_free(ASN1_OCTET_STRING, (st))
#define sk_ASN1_OCTET_STRING_push(st, val) SKM_sk_push(ASN1_OCTET_STRING, (st), (val))
#define i2d_ASN1_SET_OF_ASN1_OCTET_STRING(st, pp, i2d_func, ex_tag, ex_class, is_set) \
    SKM_ASN1_SET_OF_i2d(ASN1_OCTET_STRING, (st), (pp), (i2d_func), (ex_tag), (ex_class), (is_set))
#endif

DEFINE_STACK_OF(SpcAttributeTypeAndOptionalValue)
#ifndef sk_SpcAttributeTypeAndOptionalValue_new_null
#define sk_SpcAttributeTypeAndOptionalValue_new_null() SKM_sk_new_null(SpcAttributeTypeAndOptionalValue)
#define sk_SpcAttributeTypeAndOptionalValue_free(st) SKM_sk_free(SpcAttributeTypeAndOptionalValue, (st))
#define sk_SpcAttributeTypeAndOptionalValue_push(st, val) SKM_sk_push(SpcAttributeTypeAndOptionalValue, (st), (val))
#define i2d_SpcAttributeTypeAndOptionalValue(st, pp, i2d_func, ex_tag, ex_class, is_set) \
    SKM_ASN1_SET_OF_i2d(SpcAttributeTypeAndOptionalValue, (st), (pp), (i2d_func), (ex_tag), (ex_class), (is_set))
#endif

/******************************************************************************
 * Helper Functions
 */

static std::string convert_asn1_time_to_string(ASN1_TIME* asn1_time)
{
    if (!asn1_time) return {};
    std::string timeString;
    BIO* bmem = BIO_new(BIO_s_mem());

    if (ASN1_TIME_print(bmem, asn1_time)) {
        BUF_MEM* bptr;

        BIO_get_mem_ptr(bmem, &bptr);
        timeString.assign(std::string(bptr->data, bptr->length));
    }
    else { // Log error
    }
    BIO_free_all(bmem);
    return timeString;
}

static std::string get_certificate_version(int version)
{
    switch (version) {
    case 0:
        return "V1";
    case 1:
        return "V2";
    case 2:
        return "V3";
    }
    return "Unknown";
}

static unsigned int asn1_simple_hdr_len(const unsigned char *p, unsigned int len)
{
    if (len <= 2 || p[0] > 0x31)
        return 0;
    return (p[1] & 0x80) ? (2 + (p[1] & 0x7f)) : 2;
}

static void tohex(const unsigned char *v, unsigned char *b, int len)
{
    int i;
    for (i = 0; i<len; i++)
        sprintf((char*)b + i * 2, "%02X", v[i]);
    b[i * 2] = 0x00;
}

inline std::string GetSha1(const std::string& hash_data)
{
    SHA_CTX ctx;
    unsigned char digest[SHA_DIGEST_LENGTH];
    unsigned char hexbuf[SHA_DIGEST_LENGTH * 2 + 1];
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, &hash_data[0], hash_data.size());
    SHA1_Final(digest, &ctx);
    tohex(digest, hexbuf, SHA_DIGEST_LENGTH);
    return std::string((char*)hexbuf);
}

static unsigned char nib2val(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

static off_t get_file_size(const char *infile)
{
    int ret;
#ifdef _WIN32
    struct _stat st;
    ret = _stat(infile, &st);
#else
    struct stat st;
    ret = stat(infile, &st);
#endif
    if (ret)
        return 0;
    if (st.st_size < 4)
        return 0;
    return st.st_size;
}

static bool map_data(const char* inbuf, size_t len, PeFileInfo& pe_info)
{
    pe_info.data_ = NULL;
    pe_info.filesize_ = len;
    if (pe_info.filesize_ == 0)
        return false;
#ifdef _WIN32
    pe_info.data_ = (char*)inbuf;
    if (pe_info.data_ == NULL)
        return false;
#else
    pe_info.data_ = (char*)inbuf;
    if (pe_info.data_ == MAP_FAILED)
        return false;
#endif
    return true;
}

static bool map_file(const char *infile, PeFileInfo& pe_info)
{
    pe_info.data_ = NULL;
    pe_info.filesize_ = get_file_size(infile);
    if (pe_info.filesize_ == 0)
        return false;
#ifdef _WIN32
    pe_info.fh_ = CreateFileA(infile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (pe_info.fh_ == INVALID_HANDLE_VALUE)
        return false;
    pe_info.fm_ = CreateFileMapping(pe_info.fh_, NULL, PAGE_READONLY, 0, 0, NULL);
    if (pe_info.fm_ == NULL)
        return false;
    pe_info.data_ = (char*)MapViewOfFile(pe_info.fm_, FILE_MAP_READ, 0, 0, 0);
    if (pe_info.data_ == NULL)
        return false;
#else
    pe_info.fd_ = open(infile, O_RDONLY);
    if (pe_info.fd_ < 0)
        return false;
	pe_info.data_ = (char *)mmap(0, pe_info.filesize_, PROT_READ, MAP_PRIVATE, pe_info.fd_, 0);
    if (pe_info.data_ == MAP_FAILED)
        return false;
#endif
    return true;
}

static unsigned int get_real_siglen(char* data, off_t filesize, unsigned int sigpos, unsigned int siglen)
{
    // Check and trim tailing 0xFFFFFFFF
    unsigned real_siglen = siglen;

    for (off_t i = filesize - 4; i >= (off_t)sigpos; i -= 4)
    {
        unsigned int word = GET_UINT32_LE(data + i);
        if (word == -1)
            real_siglen -= 4;
        else
            break;
    }
    return real_siglen;
}

static unsigned int calc_pe_checksum(BIO *bio, unsigned int peheader)
{
    unsigned int checkSum = 0;
    unsigned short val;
    unsigned int size = 0;
    unsigned short *buf;
    int nread;

    // Recalculate checksum
    buf = (unsigned short*)malloc(sizeof(unsigned short) * 32768);
    if (!buf)
        return checkSum;

    (void)BIO_seek(bio, 0);
    while ((nread = BIO_read(bio, buf, sizeof(unsigned short) * 32768)) > 0)
    {
        int i;
        for (i = 0; i < nread / 2; i++)
        {
            val = buf[i];
            if (size == peheader + 88 || size == peheader + 90)
                val = 0;
            checkSum += val;
            checkSum = 0xffff & (checkSum + (checkSum >> 0x10));
            size += 2;
        }
    }

    free(buf);

    checkSum = 0xffff & (checkSum + (checkSum >> 0x10));
    checkSum += size;

    return checkSum;
}

static void calc_pe_digest(BIO *bio, const EVP_MD *md, unsigned char *mdbuf,
    unsigned int peheader, int pe32plus, unsigned int fileend)
{
    static unsigned char bfb[16 * 1024 * 1024];
    EVP_MD_CTX* mdctx = nullptr;
    int result = 0;

    mdctx = EVP_MD_CTX_new();
    if (!mdctx) return;
    EVP_MD_CTX_init(mdctx);
    result = EVP_DigestInit(mdctx, md);
    if (!result)
    {
        EVP_MD_CTX_free(mdctx);
        return;
    }

    memset(mdbuf, 0, EVP_MAX_MD_SIZE);

    (void)BIO_seek(bio, 0);
    BIO_read(bio, bfb, peheader + 88);
    EVP_DigestUpdate(mdctx, bfb, peheader + 88);
    BIO_read(bio, bfb, 4);
    BIO_read(bio, bfb, 60 + pe32plus * 16);
    EVP_DigestUpdate(mdctx, bfb, 60 + pe32plus * 16);
    BIO_read(bio, bfb, 8);

    unsigned int n = peheader + 88 + 4 + 60 + pe32plus * 16 + 8;
    while (n < fileend)
    {
        int want = fileend - n;
        if (want > sizeof(bfb))
            want = sizeof(bfb);
        int l = BIO_read(bio, bfb, want);
        if (l <= 0)
            break;
        EVP_DigestUpdate(mdctx, bfb, l);
        n += l;
    }

    EVP_DigestFinal(mdctx, mdbuf, NULL);
    EVP_MD_CTX_free(mdctx);
}

static void extract_page_hash(SpcAttributeTypeAndOptionalValue *obj,
    unsigned char **ph, unsigned int *phlen, int *phtype)
{
    *phlen = 0;

    const unsigned char *blob = obj->value->value.sequence->data;
    SpcPeImageData *id = d2i_SpcPeImageData(NULL, &blob, obj->value->value.sequence->length);
    if (id == NULL)
        return;

    if (id->file->type != 1)
    {
        SpcPeImageData_free(id);
        return;
    }

    SpcSerializedObject *so = id->file->value.moniker;
    if (so->classId->length != sizeof(classid_page_hash) ||
        memcmp(so->classId->data, classid_page_hash, sizeof(classid_page_hash)))
    {
        SpcPeImageData_free(id);
        return;
    }

    // Skip ASN.1 SET header
    unsigned int l = asn1_simple_hdr_len(so->serializedData->data, so->serializedData->length);
    blob = so->serializedData->data + l;
    obj = d2i_SpcAttributeTypeAndOptionalValue(NULL, &blob, so->serializedData->length - l);
    SpcPeImageData_free(id);
    if (!obj)
        return;

    char buf[128];
    *phtype = 0;
    buf[0] = 0x00;
    OBJ_obj2txt(buf, sizeof(buf), obj->type, 1);
    if (!strcmp(buf, SPC_PE_IMAGE_PAGE_HASHES_V1))
    {
        *phtype = NID_sha1;
    }
    else if (!strcmp(buf, SPC_PE_IMAGE_PAGE_HASHES_V2))
    {
        *phtype = NID_sha256;
    }
    else
    {
        SpcAttributeTypeAndOptionalValue_free(obj);
        return;
    }

    // Skip ASN.1 SET header
    unsigned int l2 = asn1_simple_hdr_len(obj->value->value.sequence->data, obj->value->value.sequence->length);
    // Skip ASN.1 OCTET STRING header
    l = asn1_simple_hdr_len(obj->value->value.sequence->data + l2, obj->value->value.sequence->length - l2);
    l += l2;
    *phlen = obj->value->value.sequence->length - l;
    *ph = (unsigned char*)malloc(*phlen);
    memcpy(*ph, obj->value->value.sequence->data + l, *phlen);
    SpcAttributeTypeAndOptionalValue_free(obj);
}

static unsigned char *calc_page_hash(char *indata, unsigned int peheader, int pe32plus,
    unsigned int sigpos, int phtype, unsigned int *rphlen)
{
    // References:
    // - Authenticode_PE, chapter "Calculating the PE Image Hash":
    //   http://download.microsoft.com/download/9/c/5/9c5b2167-8017-4bae-9fde-d599bac8184a/authenticode_pe.docx
    // - PE Format, chapter "Appendix A: Calculating Authenticode PE Image Hash":
    //   https://docs.microsoft.com/en-us/windows/desktop/debug/pe-format#appendix-a-calculating-authenticode-pe-image-hash

    unsigned short nsections = GET_UINT16_LE(indata + peheader + 6);
    unsigned int pagesize = GET_UINT32_LE(indata + peheader + 60);  // use "File Alignment" instead of "Section Alignment"
    unsigned int hdrsize = GET_UINT32_LE(indata + peheader + 84);
    const EVP_MD *md = EVP_get_digestbynid(phtype);
    if (!md)
        return NULL;

    int pphlen = 4 + EVP_MD_size(md);
    int phlen = pphlen * (3 + nsections + sigpos / pagesize);
    unsigned char *res = (unsigned char*)malloc(phlen);
    unsigned char *zeroes = (unsigned char*)calloc(pagesize, 1);
    EVP_MD_CTX* mdctx = nullptr;

    mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return nullptr;
    EVP_MD_CTX_init(mdctx);
    EVP_DigestInit(mdctx, md);
    EVP_DigestUpdate(mdctx, indata, peheader + 88);
    EVP_DigestUpdate(mdctx, indata + peheader + 92, 60 + pe32plus * 16);
    EVP_DigestUpdate(mdctx, indata + peheader + 160 + pe32plus * 16, hdrsize - (peheader + 160 + pe32plus * 16));
    //EVP_DigestUpdate(&mdctx, zeroes, pagesize - hdrsize);  // No need to pad with zero (not in spec)
    memset(res, 0, 4);
    EVP_DigestFinal(mdctx, res + 4, NULL);

    unsigned short sizeofopthdr = GET_UINT16_LE(indata + peheader + 20);
    char *sections = indata + peheader + 24 + sizeofopthdr;
    int i, pi = 1;
    unsigned int lastpos = 0;
    for (i = 0; i<nsections; i++)
    {
        unsigned int rs = GET_UINT32_LE(sections + 16);
        unsigned int ro = GET_UINT32_LE(sections + 20);
        unsigned int l;
        for (l = 0; l < rs; l += pagesize, pi++)
        {
            PUT_UINT32_LE(ro + l, res + pi*pphlen);
            EVP_DigestInit(mdctx, md);
            if (rs - l < pagesize)
            {
                EVP_DigestUpdate(mdctx, indata + ro + l, rs - l);
                EVP_DigestUpdate(mdctx, zeroes, pagesize - (rs - l));
            }
            else
            {
                EVP_DigestUpdate(mdctx, indata + ro + l, pagesize);
            }
            EVP_DigestFinal(mdctx, res + pi*pphlen + 4, NULL);
        }
        lastpos = ro + rs;
        sections += 40;
    }
    PUT_UINT32_LE(lastpos, res + pi*pphlen);
    memset(res + pi*pphlen + 4, 0, EVP_MD_size(md));
    pi++;
    free(zeroes);
    *rphlen = pi*pphlen;
    if(mdctx)
        EVP_MD_CTX_free(mdctx);
    return res;
}

static PKCS7 *traverse_certificate_table(char *data, unsigned int sigpos, unsigned int siglen)
{
    PKCS7 *ret = NULL;
    uint32_t pos = 0;
    uint32_t lastpos = 0;
    size_t rep = 1;
    // Quick fix to avoid infinite loop; may need proper fix later
    // MD5: 02745642cb179a2e0300deb2dd1441be
    while (pos < siglen && rep < 3)
    {
        uint32_t l = GET_UINT32_LE(data + sigpos + pos);
        if (l == 0)
            break;
        uint16_t certrev = GET_UINT16_LE(data + sigpos + pos + 4);
        uint16_t certtype = GET_UINT16_LE(data + sigpos + pos + 6);
        if (certrev == WIN_CERT_REVISION_2 && certtype == WIN_CERT_TYPE_PKCS_SIGNED_DATA)
        {
            const unsigned char *blob = (unsigned char*)data + sigpos + pos + 8;
            ret = d2i_PKCS7(NULL, &blob, l - 8);
        }
        if (l % 8)
            l += (8 - l % 8);
        lastpos = pos;
        pos += l;
        if (pos <= lastpos)
            rep ++;
    }
    return ret;
}

// Function extract_existing_pe_pkcs7() retrieves a decoded PKCS7 struct
// corresponding to the existing signature of the PE file.
static PKCS7 *extract_existing_pe_pkcs7(PeFileInfo& pe)
{
    unsigned int sigpos = pe.sigpos_ ? pe.sigpos_ : pe.fileend_;
    PKCS7 *p7 = NULL;
    p7 = traverse_certificate_table(pe.data_, sigpos, pe.siglen_);
    return p7;
}

std::string dserror::GenerateErrorMessage(ErrorType error, std::string details)
{
    std::string error_msg = error.second;
    if (!details.empty())
        error_msg += ": " + details;
    return error_msg;
}

/******************************************************************************
 * OpenSSLLoader Definitions
 */

static void cleanup_lib_state(void)
{
    OBJ_cleanup();
#ifndef OPENSSL_NO_ENGINE
    ENGINE_cleanup();
#endif
    EVP_cleanup();
    CONF_modules_free();
    CRYPTO_cleanup_all_ex_data();
#if OPENSSL_VERSION_NUMBER > 0x10000000
    ERR_remove_thread_state(NULL);
#endif
    ERR_free_strings();
}

OpenSSLLoader::OpenSSLLoader()
{
    // Set up OpenSSL
    ERR_load_crypto_strings();
    OPENSSL_add_all_algorithms_conf();

    // Create some MS Authenticode OIDS we need later on
    if (!OBJ_create(SPC_NESTED_SIGNATURE_OBJID, NULL, NULL))
    {
        ERR_print_errors_fp(stderr);
        cleanup_lib_state();
        throw std::runtime_error("Failed to add objects");
    }
}

OpenSSLLoader::~OpenSSLLoader()
{
    ERR_free_strings();
    CONF_modules_free();
    OBJ_cleanup();
    cleanup_lib_state();
}

/******************************************************************************
 * SignInfo Definitions
 */

bool SignInfo::AddError(ErrorType error, std::string details /*= std::string()*/)
{
    bool is_added = false;

    uint16_t error_code = error.first;
    std::string error_msg = dserror::GenerateErrorMessage(error, details);

    bool error_exist = false;
    ErrorVector::iterator it;
    for (it = errors_.begin(); it != errors_.end(); it++)
    {
        if (it->first == error_code && it->second.compare(error_msg) == 0)
        {
            error_exist = true;
            break;
        }
    }

    if (!error_exist)
    {
        ErrorType new_error(error_code, error_msg);
        errors_.push_back(new_error);
        is_added = true;
    }
    return is_added;
}

bool SignInfo::AddOpenSSLError(unsigned long error_code, std::string error_message)
{
    bool is_added = false;

    bool error_exist = false;
    ErrorVector::iterator it;
    for (it = openssl_errors_.begin(); it != openssl_errors_.end(); it++)
    {
        if (it->first == error_code)
        {
            error_exist = true;
            break;
        }
    }

    if (!error_exist)
    {
        ErrorType new_error(error_code, error_message);
        openssl_errors_.push_back(new_error);
        is_added = true;
    }
    return is_added;
}

/******************************************************************************
 * CertManager Definitions
 */

void CertManager::ResetRuntimeVariables()
{
    crl_check_enabled_ = false;
    store_certificate_ = false;
    leaf_digest_name_ = NULL;
    leaf_hash_ = NULL;
    certificate_.clear();
}

bool CertManager::LoadCerts(X509_STORE *store, json_t *certs)
{
    bool success = true;

    size_t j_index;
    json_t *j_value, *j_cert;
    const char *cert_buffer = NULL;
    BIO *cbio = NULL;
    X509 *cert = NULL;

    json_array_foreach(certs, j_index, j_value)
    {
        // Extract trusted certificate from database
        if (!json_is_object(j_value))
        {
            success = false;
            break;
        }
        j_cert = json_object_get(j_value, "certificate");
        cert_buffer = json_string_value(j_cert);

        // Store trusted certificate
        cbio = BIO_new_mem_buf((void*)cert_buffer, -1);
        cert = PEM_read_bio_X509(cbio, NULL, 0, NULL);
        BIO_free(cbio);
        if (!cert)
        {
            success = false;
            break;
        }
        if (!X509_STORE_add_cert(store, cert))
        {
            if (cert)
                X509_free(cert);
            success = false;
            break;
        }
        if (cert)
            X509_free(cert);
    }

    return success;
}

bool CertManager::AnalyzePeStructure(PeFileInfo *pe, SignInfo &sinfo)
{
    if (memcmp(pe->data_, "MZ", 2))
    {
        RecordError(sinfo, dserror::struct_unknown_filetype);
        return false;
    }
    if (pe->filesize_ < 64)
    {
        RecordError(sinfo, dserror::struct_dos_short);
        return false;
    }

    pe->peheader_ = GET_UINT32_LE(pe->data_ + 60);
    if (pe->filesize_ < pe->peheader_ + 160)
    {
        RecordError(sinfo, dserror::struct_pe_short);
        return false;
    }
    if (memcmp(pe->data_ + pe->peheader_, "PE\0\0", 4))
    {
        RecordError(sinfo, dserror::struct_dos_unknown_filetype);
        return false;
    }

    pe->magic_ = GET_UINT16_LE(pe->data_ + pe->peheader_ + 24);
    if (pe->magic_ == 0x20b)
    {
        pe->pe32plus_ = 1;
    }
    else if (pe->magic_ == 0x10b)
    {
        pe->pe32plus_ = 0;
    }
    else
    {
        RecordError(sinfo, dserror::struct_pe_unknown_magic, std::to_string(pe->magic_));
        return false;
    }

    pe->nrvas_ = GET_UINT32_LE(pe->data_ + pe->peheader_ + 116 + pe->pe32plus_ * 16);
    if (pe->nrvas_ < 5)
    {
        RecordError(sinfo, dserror::struct_pe_missing_cert_table);
        return false;
    }

    pe->sigpos_ = GET_UINT32_LE(pe->data_ + pe->peheader_ + 152 + pe->pe32plus_ * 16);
    pe->siglen_ = GET_UINT32_LE(pe->data_ + pe->peheader_ + 152 + pe->pe32plus_ * 16 + 4);

    if (pe->sigpos_ == 0 || pe->siglen_ == 0)
    {
        RecordError(sinfo, dserror::struct_pe_sig_none);
        return false;
    }
    // Since fix for MS Bulletin MS12-024 we can really assume
    // that signature should be last part of file.
    else
    {
        // Prevent possible overflow
        if ((uint64_t)pe->sigpos_ + (uint64_t)pe->siglen_ != (uint64_t)pe->filesize_)
        {
            RecordError(sinfo, dserror::struct_pe_sig_end);
            return false;
        }
        unsigned real_siglen = get_real_siglen(pe->data_, pe->filesize_, pe->sigpos_, pe->siglen_);
        if (pe->siglen_ != real_siglen)
        {
            RecordError(sinfo, dserror::struct_pe_sig_length);
            pe->siglen_ = real_siglen;
        }
    }

    return true;
}

int CertManager::VerifyPeFile(VerifyInfo *vinfo, PeFileInfo& pe)
{
    int ret = -1;
    unsigned int sigpos = pe.sigpos_ ? pe.sigpos_ : pe.fileend_;
    unsigned int pe_checksum = GET_UINT32_LE(pe.data_ + pe.peheader_ + 88);
    BIO *bio = BIO_new_mem_buf(pe.data_, sigpos + pe.siglen_);
    unsigned int real_pe_checksum = calc_pe_checksum(bio, pe.peheader_);
    BIO_free(bio);
    if (pe_checksum && pe_checksum != real_pe_checksum)
    {
        RecordError(*vinfo, dserror::vfy_checksum_mismatch);
        ret = 1;
    }

    PKCS7 *p7 = NULL;
    p7 = traverse_certificate_table(pe.data_, sigpos, pe.siglen_);
    if (p7)
    {
        if (store_certificate_ && !StoreCertificate(p7))
            RecordError(*vinfo, dserror::vfy_cert_store);
    }
    else
    {
        RecordError(*vinfo, dserror::vfy_pkcs7_extract);
        return -1;
    }

    vinfo->has_signature_ = true;

    // Attempt to extract and verify countersignature, if there is any
    ProcessCountersig(vinfo, p7);

    // Attempt to verify the signature
    ret = VerifyPePkcs7(vinfo, p7, pe.data_, pe.peheader_, pe.pe32plus_,
        sigpos, pe.siglen_);

    // Attempt to verify nested signature, if there is any.
    // NOTE 1:
    // Skip nested signature retrieval if the certificate has incorrect format.
    // Otherwise, it may result in read violation error.
    // MD5: a8ee1785e7857ee0f20e35d128d9168d
    // NOTE 2:
    // Authenticode certificate may have multiple nested signatures.
    // MD5: 2c776a1edce317f51d2f53d27dbbb64c
    if (ret >= 0)
    {
        PKCS7_SIGNER_INFO *si = NULL;
        STACK_OF(X509_ATTRIBUTE) *attrs = NULL;
        X509_ATTRIBUTE *attrib = NULL;
        int loc = 0;
        int i = 0;
        ASN1_TYPE *nested_sig = NULL;
        ASN1_STRING *astr = NULL;
        const unsigned char *p = NULL;
        PKCS7 *p7_nest = NULL;

        do
        {
            si = sk_PKCS7_SIGNER_INFO_value(p7->d.sign->signer_info, 0);
            if (!si)
                break;
            attrs = PKCS7_get_attributes(si);
            loc = X509at_get_attr_by_NID(attrs, OBJ_txt2nid(SPC_NESTED_SIGNATURE_OBJID), -1);
            attrib = X509at_get_attr(attrs, loc);
            if (!attrib)
                break;

            for (i = 0; i < MAX_NESTED_SIGS; i++)
            {
                nested_sig = X509_ATTRIBUTE_get0_type(attrib, i);
                if (!nested_sig)
                    break;
                else
                    vinfo->has_nested_signatures_ = true;

                astr = nested_sig->value.sequence;
                p = astr->data;
                p7_nest = d2i_PKCS7(NULL, &p, astr->length);
                if (p7_nest)
                {
                    VerifyInfo nest_sinfo;
                    int nest_ret = VerifyPePkcs7(&nest_sinfo, p7_nest, pe.data_,
                        pe.peheader_, pe.pe32plus_, sigpos, pe.siglen_);
                    if (ret != 0 && nest_ret == 0)
                        vinfo->use_nested_signatures_ = true;
                    // Store nested signature information
                    vinfo->nested_signatures_.push_back(nest_sinfo);
                    PKCS7_free(p7_nest);
                }
                else
                {
                    RecordError(*vinfo, dserror::vfy_nested_sig_decode, std::to_string(i));
                }
            }
            // Overwrite return value if original verification fails but any of the
            // nested verification succeeds.
            if (vinfo->use_nested_signatures_)
                ret = 0;
        } while (false);
    }

    PKCS7_free(p7);
    return ret;
}

bool CertManager::StoreCertificate(PKCS7 *pkcs7_sig)
{
    bool success = true;
    const EVP_MD *md;
    BIO *hash = NULL, *outdata = NULL;

    md = EVP_sha1();
    hash = BIO_new(BIO_f_md());
    BIO_set_md(hash, md);
    outdata = BIO_new(BIO_s_mem());
    if (outdata)
    {
        BIO_push(hash, outdata);

        // A lil' bit of ugliness. Reset stream, write signature and skip forward
        (void)BIO_reset(outdata);
        PEM_write_bio_PKCS7(outdata, pkcs7_sig);

        char *data_ptr = NULL;
        long data_size = BIO_get_mem_data(outdata, &data_ptr);
        // Store certificate data
        if (data_ptr && data_size > 0)
            certificate_ = std::string(data_ptr, data_size);
    }
    else
    {
        success = false;
    }

    if (hash)
        BIO_free_all(hash);
    hash = outdata = NULL;
    return success;
}

// Verify countersignature according to RFC 2985.
bool CertManager::VerifyCountersig(
    PKCS7_SIGNER_INFO *counter_si,
    ASN1_OCTET_STRING *encrypted_digest,
    ErrorType& error)
{
    STACK_OF(X509_ATTRIBUTE) *cs_attrs = NULL;
    X509_ATTRIBUTE *cs_attr = NULL;
    int loc = 0;
    X509_ALGOR *cs_alg = NULL;
    //ASN1_OBJECT *mdobj = NULL;
    int mdtype = -1;
    const EVP_MD *md;
    EVP_MD_CTX *mdctx = NULL;

    cs_attrs = counter_si->auth_attr;
    if (!cs_attrs)
    {
        error = dserror::vfy_countersig_no_auth;
        return false;
    }

    cs_alg = counter_si->digest_alg;
    if (!cs_alg)
    {
        error = dserror::vfy_countersig_no_alg;
        return false;
    }

    mdtype = OBJ_obj2nid(cs_alg->algorithm);
    if (mdtype <= 0)
    {
        error = dserror::vfy_countersig_invalid_alg;
        return false;
    }

    loc = X509at_get_attr_by_NID(cs_attrs, OBJ_txt2nid(SPC_AUTH_ATTR_MESSAGE_DIGEST), -1);
    cs_attr = X509at_get_attr(cs_attrs, loc);
    if (!cs_attr)
    {
        error = dserror::vfy_countersig_no_msg_digest;
        return false;
    }

    unsigned char mdbuf[EVP_MAX_MD_SIZE];
    unsigned char cmdbuf[EVP_MAX_MD_SIZE];
    int cs_count = 0;
    ASN1_TYPE *cs_asn1_type = NULL;

    // Ensure only a single message digest exists
    cs_count = X509_ATTRIBUTE_count(cs_attr);
    if (cs_count != 1)
    {
        error = dserror::vfy_countersig_attr_count;
        return false;
    }
    // Store the saved message digest
    cs_asn1_type = X509_ATTRIBUTE_get0_type(cs_attr, 0);
    memcpy(mdbuf, cs_asn1_type->value.octet_string->data, cs_asn1_type->value.octet_string->length);

    md = EVP_get_digestbynid(mdtype);
    if (!md)
    {
        error = dserror::vfy_countersig_hash_alg;
        return false;
    }
    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, encrypted_digest->data, encrypted_digest->length);
    EVP_DigestFinal_ex(mdctx, cmdbuf, NULL);
    EVP_MD_CTX_destroy(mdctx);

    if (memcmp(mdbuf, cmdbuf, EVP_MD_size(md)) != 0)
    {
        error = dserror::vfy_countersig_mismatch;
        return false;
    }

    return true;
}

// Extract and verify countersignature, if there is any.
void CertManager::ProcessCountersig(VerifyInfo *vinfo, PKCS7 *pkcs7_sig)
{
    STACK_OF(PKCS7_SIGNER_INFO) *signer_infos = NULL;
    PKCS7_SIGNER_INFO *sign_info = NULL;
    ASN1_OCTET_STRING *enc_digest = NULL;
    STACK_OF(X509_ATTRIBUTE) *u_attrs = NULL;
    SignInfo::signer_info signer_info{};
    int loc = 0;
    X509_ATTRIBUTE *attr = NULL;
    ASN1_TYPE *asn1_type = NULL;

    // Sanity check
    if (!pkcs7_sig)
        return;

    // Check if there is only one signer
    signer_infos = PKCS7_get_signer_info(pkcs7_sig);
    if (!signer_infos || (sk_PKCS7_SIGNER_INFO_num(signer_infos) != 1))
        return;

    // Locate Countersignature
    sign_info = sk_PKCS7_SIGNER_INFO_value(signer_infos, 0);
    if (!sign_info)
        return;

    // Locate Message Digest which will be the data to be time-stamped
    enc_digest = sign_info->enc_digest;
    //if (!enc_digest)
    //    return;

    // CounterSignature is contained in unauthenticatedAttributes field of SignerInfo
    u_attrs = sign_info->unauth_attr;
    if (!u_attrs)
        return;

    do
    {
        // First, check if RFC 2985 Countersignature exists
        loc = X509at_get_attr_by_NID(u_attrs, OBJ_txt2nid(SPC_RFC2985_OBJID), -1);
        attr = X509at_get_attr(u_attrs, loc);
        if (attr)
        {
            ASN1_STRING *astr = NULL;
            const unsigned char *p = NULL;
            PKCS7_SIGNER_INFO *cs_sign_info = NULL;
            int i = 0;
            int count = X509_ATTRIBUTE_count(attr);

            // NOTE: For now we assume this loop handles multiple countersignatures,
            //       since can also be the case the iteration should be on 'u_attrs'
            //       instead, using sk_X509_ATTRIBUTE_num(), sk_X509_ATTRIBUTE_value(),
            //       X509_ATTRIBUTE_get0_object() and OBJ_cmp().
            //       Can only be verified once we find a file with actual multiple
            //       countersignatures.
            for (i = 0; i < count; i++)
            {
                X509 *cs_cert = NULL;
                bool verified = false;
                ErrorType err = dserror::unknown;

                asn1_type = X509_ATTRIBUTE_get0_type(attr, i);
                if (!asn1_type)
                    continue;
                // Check type; assume the default is octet string
                if (asn1_type->type == V_ASN1_SEQUENCE)
                    astr = asn1_type->value.sequence;
                else
                    astr = asn1_type->value.octet_string;
                if (!astr)
                    continue;

                p = astr->data;
                cs_sign_info = d2i_PKCS7_SIGNER_INFO(NULL, &p, astr->length);
                if (!cs_sign_info)
                    continue;

                cs_cert = PKCS7_cert_from_signer_info(pkcs7_sig, cs_sign_info);
                if (!cs_cert)
                    continue;

                // We're confident at this point that countersignature exists
                vinfo->has_counter_signatures_ = true;
                CounterSignInfo csi;
                csi.type_ = kRfc2985;

                // Store signer information
                char* cn_buff = (char*)malloc(MAX_CN_LEN);
                if (cn_buff)
                {
                    int cn_ret = X509_NAME_get_text_by_NID(X509_get_subject_name(cs_cert), NID_commonName, cn_buff, MAX_CN_LEN);
                    if (cn_ret != -1)
                    {
                        std::string cname(cn_buff);
                        signer_info.signer_name = cname;
                    }
                    free(cn_buff);
                }
                else
                {
                    RecordError(csi, dserror::runtime_error, "failed to allocate memory for Common Name (CN)");
                }

                signer_info.issuer_info.version = get_certificate_version(X509_get_version(cs_cert));
                char* subject = X509_NAME_oneline(X509_get_subject_name(cs_cert), NULL, 0);
                signer_info.issuer_info.subject = subject;
                char* issuer = X509_NAME_oneline(X509_get_issuer_name(cs_cert), NULL, 0);
                signer_info.issuer_info.issuer = issuer;
                BIGNUM* serialbn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cs_cert), NULL);
                char* serial = BN_bn2hex(serialbn);
                signer_info.issuer_info.serial_number = serial;
                ASN1_TIME* time1 = X509_getm_notBefore(cs_cert);
                signer_info.issuer_info.time_from = convert_asn1_time_to_string(time1);
                ASN1_TIME* time2 = X509_getm_notAfter(cs_cert);
                signer_info.issuer_info.time_to = convert_asn1_time_to_string(time2);
                signer_info.issuer_info.sign_algorithm = std::string(OBJ_nid2sn(X509_get_signature_nid(cs_cert)));
                int mdnid, pknid, secbits;
                uint32_t flags;
                X509_get_signature_info(cs_cert, &mdnid, &pknid, &secbits, &flags);
                signer_info.digest_algorithm = std::string(OBJ_nid2sn(mdnid));
                signer_info.issuer_info.sign_hash_algorithm = signer_info.digest_algorithm;
                signer_info.issuer_info.public_key_algorithm = std::string(OBJ_nid2sn(pknid));

                {
                    unsigned char* b = NULL;
                    int n = ASN1_item_i2d((ASN1_VALUE*)cs_cert, &b, ASN1_ITEM_rptr(X509));
                    if (b != nullptr && n > 0) {
                        signer_info.issuer_info.thumbprint = GetSha1(std::string((char*)b, n));
                    }
                    if (b) OPENSSL_free(b);
                }

                OPENSSL_free(subject);
                OPENSSL_free(issuer);
                OPENSSL_free(serial);
                BN_free(serialbn);

                if (signer_info.issuer_info.issuer.length()) {
                    csi.signers_.push_back(signer_info);
                }

                if (enc_digest)
                {
                    verified = VerifyCountersig(cs_sign_info, enc_digest, err);
                    csi.is_verified_ = verified;
                    if (err != dserror::unknown)
                        RecordError(csi, err);
                }
                vinfo->counter_signatures_.push_back(csi);

                PKCS7_SIGNER_INFO_free(cs_sign_info);
            }
        }
        // Then check if RFC 3161 Timestamp Countersignature exists
        loc = X509at_get_attr_by_NID(u_attrs, OBJ_txt2nid(SPC_RFC3161_OBJID), -1);
        attr = X509at_get_attr(u_attrs, loc);
        if (attr)
        {
            // TODO: Implement RFC 3161 extraction
            //asn1_type = X509_ATTRIBUTE_get0_type(attr, 0);
        }
    } while (false);

    return;
}

int CertManager::VerifyPePkcs7(VerifyInfo*sinfo, PKCS7 *p7, char *indata,
    unsigned int peheader, int pe32plus, unsigned int sigpos, unsigned int siglen)
{
    int ret = 0;
    SignInfo::signer_info signer_info{};
    SignInfo::certificate_info cert_info = {};
    int mdtype = -1, phtype = -1;
    unsigned char mdbuf[EVP_MAX_MD_SIZE];
    unsigned char cmdbuf[EVP_MAX_MD_SIZE];
    unsigned char hexbuf[EVP_MAX_MD_SIZE * 2 + 1];
    unsigned char *ph = NULL;
    unsigned int phlen = 0;
    int i;
    int leafok = 0;
    bool err = false;
    bool repeat;
    int cert_num = 0;

    BIO *bio = NULL;

    X509_STORE_CTX *store_ctx = NULL;
    X509_VERIFY_PARAM *verify_params = NULL;
    STACK_OF(X509) *signers = NULL;
    //STACK_OF(X509) *untrusted = NULL;
    STACK_OF(X509_CRL) *crls = NULL;

    STACK_OF(X509_CRL) *original_crls = NULL;
    bool crl_injected = false;

    ASN1_OBJECT *indir_objid = OBJ_txt2obj(SPC_INDIRECT_DATA_OBJID, 1);
    if (PKCS7_type_is_signed(p7) &&
        !OBJ_cmp(p7->d.sign->contents->type, indir_objid))
    {
        ASN1_STRING *astr = NULL;
        if (p7->d.sign->contents->d.other->type == V_ASN1_SEQUENCE)
        {
            astr = p7->d.sign->contents->d.other->value.sequence;
        }
        // ASN.1 indefinite length (terminated by two zero bytes)
        // MD5: 3277273412d8abaf3be767b6db227546
        else if (p7->d.sign->contents->d.other->type == V_ASN1_OCTET_STRING)
        {
            astr = p7->d.sign->contents->d.other->value.octet_string;
        }

        if (astr)
        {
            const unsigned char *p = astr->data;
            SpcIndirectDataContent *idc = d2i_SpcIndirectDataContent(NULL, &p, astr->length);
            if (idc)
            {
                extract_page_hash(idc->data, &ph, &phlen, &phtype);
                if (idc->messageDigest && idc->messageDigest->digest && idc->messageDigest->digestAlgorithm)
                {
                    mdtype = OBJ_obj2nid(idc->messageDigest->digestAlgorithm->algorithm);
                    memcpy(mdbuf, idc->messageDigest->digest->data, idc->messageDigest->digest->length);
                }
                SpcIndirectDataContent_free(idc);
            }
        }
    }
    ASN1_OBJECT_free(indir_objid);

    do
    {
        if (mdtype == -1)
        {
            RecordError(*sinfo, dserror::vfy_msg_digest_extract);
            ret = -1;
            break;
        }
        else if (mdtype == 0)
        {
            RecordError(*sinfo, dserror::vfy_msg_digest_undefined);
            ret = -1;
            break;
        }
        //printf("Message digest algorithm: %s\n", OBJ_nid2sn(mdtype));

        const EVP_MD *md = EVP_get_digestbynid(mdtype);
        if (!md)
        {
            RecordError(*sinfo, dserror::vfy_msg_digest_get);
            ret = -1;
            break;
        }
        tohex(mdbuf, hexbuf, EVP_MD_size(md));
        //printf("Current message digest: %s\n", hexbuf);

        bio = BIO_new_mem_buf(indata, sigpos + siglen);
        calc_pe_digest(bio, md, cmdbuf, peheader, pe32plus, sigpos);
        if (bio)
        {
            BIO_free(bio);
            bio = NULL;
        }
        tohex(cmdbuf, hexbuf, EVP_MD_size(md));
        int mdok = !memcmp(mdbuf, cmdbuf, EVP_MD_size(md));
        if (!mdok)
        {
            std::string detail = "Algorithm is " + std::string(OBJ_nid2sn(mdtype));
            RecordError(*sinfo, dserror::vfy_msg_digest_mismatch, detail);
            ret = 1;
            break;
        }
        
        //printf("Calculated message digest: %s [%s]\n", hexbuf, mdok ? "match" : "mismatch");

        if (phlen > 0)
        {
            //printf("Page hash algorithm: %s\n", OBJ_nid2sn(phtype));
            tohex(ph, hexbuf, (phlen < 32) ? phlen : 32);
            //printf("Page hash: %s\n", hexbuf);
            unsigned int cphlen = 0;
            unsigned char *cph = calc_page_hash(indata, peheader, pe32plus, sigpos, phtype, &cphlen);
            if (!cph)
            {
                RecordError(*sinfo, dserror::vfy_page_hash_calc);
                ret = -1;
                break;
            }
            tohex(cph, hexbuf, (cphlen < 32) ? cphlen : 32);
            //printf("Calculated page hash: %s [%s]\n", hexbuf,
            //    ((phlen != cphlen) || memcmp(ph, cph, phlen)) ? "mismatch" : "match");
            if ((phlen != cphlen) || memcmp(ph, cph, phlen))
            {
                std::string detail = "Algorithm is " + std::string(OBJ_nid2sn(phtype));
                RecordError(*sinfo, dserror::vfy_page_hash_mismatch, detail);
                //free(ph);
                //free(cph);
                //return 1;
            }
            else
            {
                //std::string detail("page hash (" + std::string(OBJ_nid2sn(phtype)) + ") match");
                //RecordError(*sinfo, dserror::debug_info, detail);
            }
            free(cph);
        }
    } while (false);

    if (ph)
        free(ph);
    if (ret != 0)
        return ret;

    // Get certificate revocation lists (CRLs) from CRL store database
    if (crl_check_enabled_ && crl_store_initialized_)
    {
        std::vector<std::string> issuers;

        // Get a list of issuers
        for (i = 0; i < sk_X509_num(p7->d.sign->cert); i++)
        {
            X509 *cert = sk_X509_value(p7->d.sign->cert, i);
            X509_NAME *issuer_name = X509_get_issuer_name(cert);
            if (!issuer_name)
                continue;
            char *issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
            issuers.push_back(std::string(issuer));
            OPENSSL_free(issuer);
        }

        original_crls = p7->d.sign->crl;
    }

    store_ctx = X509_STORE_CTX_new();
    if (!store_ctx)
    {
        RecordError(*sinfo, dserror::runtime_error, "failed to create a new X509 store ctx");
        ret = 1;
        goto verify_cleanup;
    }

    repeat = false;
    sinfo->crl_check_bypassed_ = false;
    do
    {
        if (repeat)
            sinfo->crl_check_bypassed_ = true;

        // NOTE: X509_STORE_CTX object can only be used once per verification;
        //       need to cleanup() and re-init().
        if (!X509_STORE_CTX_init(store_ctx, get_windows_root_store(), NULL, NULL /*untrusted*/))
        {
            RecordError(*sinfo, dserror::runtime_error, "failed to initialize X509 store ctx");
            ret = 1;
            goto verify_cleanup;
        }

        verify_params = X509_STORE_CTX_get0_param(store_ctx);
        if (!verify_params)
        {
            RecordError(*sinfo, dserror::runtime_error, "failed to retrieve X509 store ctx verification parameters");
            ret = 1;
            goto verify_cleanup;
        }
        if (!X509_VERIFY_PARAM_set_purpose(verify_params, X509_PURPOSE_ANY))
        {
            RecordError(*sinfo, dserror::runtime_error, "failed to set verification purpose");
            ret = 1;
            goto verify_cleanup;
        }
        // Flag below is set to avoid verification error 'unhandled critical extension'
        if (!X509_VERIFY_PARAM_set_flags(verify_params, X509_V_FLAG_IGNORE_CRITICAL))
        {
            RecordError(*sinfo, dserror::runtime_error, "failed to set verification flag for ignoring critical extension checking");
            ret = 1;
            goto verify_cleanup;
        }
        if (crl_check_enabled_)
        {
            if (!repeat)
            {
                if (!X509_VERIFY_PARAM_set_flags(verify_params, X509_V_FLAG_CRL_CHECK))
                {
                    RecordError(*sinfo, dserror::runtime_error, "failed to set verification flag for CRL checking");
                    ret = 1;
                    goto verify_cleanup;
                }

                // Set verification time to the last updated time of CRL store,
                // as it is the time where almost all of CRLs are valid.
                // NOTE: This implementation might result in a side effect of which
                //       we're stuck in the past to fulfil the CRL validity but not
                //       newly-issued certificate validity ("certificate is not yet valid").
                //time_t verification_time = GetLastUpdateTime(&crl_store_db_);
                //if (verification_time != 0)
                //    X509_VERIFY_PARAM_set_time(verify_params, verification_time);
                //else
                //    RecordError(*sinfo, dserror::runtime_error, "failed to retrieve CRL store's last updated time");
            }
            else
            {
                if (!X509_VERIFY_PARAM_clear_flags(verify_params, X509_V_FLAG_CRL_CHECK))
                {
                    RecordError(*sinfo, dserror::runtime_error, "failed to clear verification flag for CRL checking");
                    ret = 1;
                    goto verify_cleanup;
                }
            }
        }

        // Continue with certificate chain validation process
        if (p7->d.sign->contents->d.other->type == V_ASN1_SEQUENCE)
        {
            int seqhdrlen = asn1_simple_hdr_len(p7->d.sign->contents->d.other->value.sequence->data,
                p7->d.sign->contents->d.other->value.sequence->length);
            bio = BIO_new_mem_buf(p7->d.sign->contents->d.other->value.sequence->data + seqhdrlen,
                p7->d.sign->contents->d.other->value.sequence->length - seqhdrlen);
        }
        // ASN.1 indefinite length (terminated by two zero bytes)
        // MD5: 3277273412d8abaf3be767b6db227546
        else if (p7->d.sign->contents->d.other->type == V_ASN1_OCTET_STRING)
        {
            bio = BIO_new_mem_buf(p7->d.sign->contents->d.other->value.octet_string->data,
                p7->d.sign->contents->d.other->value.octet_string->length);
        }
        else
        {
            std::stringstream ss;
            ss << "unknown PKCS7 contents type: " << p7->d.sign->contents->d.other->type;
            RecordError(*sinfo, dserror::runtime_error, ss.str());
            ret = 1;
            goto verify_cleanup;
        }

        if (bio)
        {
            BIO_free(bio);
            bio = NULL;
        }

        // Cleanup X509 store ctx so that it can be reused in case of repeat
        X509_STORE_CTX_cleanup(store_ctx);
    } while (repeat && !sinfo->crl_check_bypassed_);

    signers = PKCS7_get0_signers(p7, NULL, 0);
    //printf("Number of signers: %d\n", sk_X509_num(signers));
    cert_num = sk_X509_num(signers);
    for (i = 0; i < cert_num; i++)
    {
        X509 *cert = sk_X509_value(signers, i);

        // Store signer information to VerifyInfo structure
        char *cn_buff = (char*)malloc(MAX_CN_LEN);
        if (!cn_buff)
        {
            RecordError(*sinfo, dserror::runtime_error, "failed to allocate memory for Common Name (CN)");
            err = true;
            break;
        }
        int cn_ret = X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName, cn_buff, MAX_CN_LEN);
        if (cn_ret != -1)
        {
            std::string cname(cn_buff);
            if (!signer_info.signer_name.length())
                signer_info.signer_name = cname;
        }
        free(cn_buff);

        signer_info.issuer_info.version = get_certificate_version(X509_get_version(cert));
        char *subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
        signer_info.issuer_info.subject = subject;
        char *issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
        signer_info.issuer_info.issuer = issuer;
        BIGNUM *serialbn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cert), NULL);
        char *serial = BN_bn2hex(serialbn);
        signer_info.issuer_info.serial_number = serial;
        ASN1_TIME *time1 = X509_getm_notBefore(cert);
        signer_info.issuer_info.time_from = convert_asn1_time_to_string(time1);
        ASN1_TIME* time2 = X509_getm_notAfter(cert);
        signer_info.issuer_info.time_to = convert_asn1_time_to_string(time2);
        signer_info.issuer_info.sign_algorithm = std::string(OBJ_nid2sn(X509_get_signature_nid(cert)));
        int mdnid, pknid, secbits;
        uint32_t flags;
        int ret = X509_get_signature_info(cert, &mdnid, &pknid, &secbits, &flags);
        signer_info.digest_algorithm = std::string(OBJ_nid2sn(mdtype));
        signer_info.issuer_info.sign_hash_algorithm = std::string(OBJ_nid2sn(mdnid));
        signer_info.issuer_info.public_key_algorithm = std::string(OBJ_nid2sn(pknid));

        {
            unsigned char* b = NULL;
            int n = ASN1_item_i2d((ASN1_VALUE*)cert, &b, ASN1_ITEM_rptr(X509));
            if (b != nullptr && n > 0) {
                signer_info.issuer_info.thumbprint = GetSha1(std::string((char*)b, n));
            }
            if(b) OPENSSL_free(b);
        }
        
        OPENSSL_free(subject);
        OPENSSL_free(issuer);
        OPENSSL_free(serial);
        BN_free(serialbn);

        if (leaf_digest_name_ != NULL && leaf_hash_ != NULL && leafok == 0)
        {
            leafok = VerifyLeafHash(cert, leaf_digest_name_, leaf_hash_, sinfo);
            if (!leafok)
                ret = 1;
        }
    }
    sk_X509_free(signers);

    if (err)
    {
        ret = 1;
        goto verify_cleanup;
    }

    //printf("\nNumber of certificates: %d\n", sk_X509_num(p7->d.sign->cert));
    cert_num = sk_X509_num(p7->d.sign->cert);
    for (i = 0; i < cert_num; i++)
    {
        X509 *cert = sk_X509_value(p7->d.sign->cert, i);
        cert_info.version = get_certificate_version(X509_get_version(cert));
        char* subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
        cert_info.subject = subject;
        char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
        cert_info.issuer = issuer;
        BIGNUM* serialbn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cert), NULL);
        char* serial = BN_bn2hex(serialbn);
        cert_info.serial_number = serial;
        ASN1_TIME* time1 = X509_getm_notBefore(cert);
        cert_info.time_from = convert_asn1_time_to_string(time1);
        ASN1_TIME* time2 = X509_getm_notAfter(cert);
        cert_info.time_to = convert_asn1_time_to_string(time2);
        cert_info.sign_algorithm = std::string(OBJ_nid2sn(X509_get_signature_nid(cert)));
        int mdnid, pknid, secbits;
        uint32_t flags;
        X509_get_signature_info(cert, &mdnid, &pknid, &secbits, &flags);
        cert_info.public_key_algorithm = std::string(OBJ_nid2sn(pknid));
        cert_info.brootcert = false;
        {
            unsigned char* b = NULL;
            int n = ASN1_item_i2d((ASN1_VALUE*)cert, &b, ASN1_ITEM_rptr(X509));
            if (b != nullptr && n > 0) {
                cert_info.thumbprint = GetSha1(std::string((char*)b, n));
            }
            if (b) OPENSSL_free(b);
        }
        if (cert_info.issuer.length()) {
            cert_info.issuer_thumbprint = GetSha1(cert_info.issuer);
        }
        else {
            cert_info.issuer_thumbprint = "";
        }
        if (cert_info.thumbprint.length()) {
            if (cert_info.thumbprint.compare(signer_info.issuer_info.thumbprint) != 0) {
                if (cert_info.issuer.length())
                    signer_info.certificates.push_back(cert_info);
            }
        }
        //if (i > 0)
        //    printf("\t------------------\n");
        //printf("\tCert #%d:\n\t\tSubject: %s\n\t\tIssuer: %s\n\t\tSerial: %s\n",
        //    i, subject, issuer, serial);
        OPENSSL_free(subject);
        OPENSSL_free(issuer);
        OPENSSL_free(serial);
        BN_free(serialbn);
    }

    if (sinfo->has_counter_signatures_ && sinfo->counter_signatures_.size()) {
        for (auto& counter : sinfo->counter_signatures_) {
            if (counter.signers_.size()) {
                if (counter.signers_[0].issuer_info.thumbprint.size()) {
                    auto iter = signer_info.certificates.begin();
                    for (; iter != signer_info.certificates.end(); iter++) {
                        if (counter.signers_[0].issuer_info.thumbprint.compare(iter->thumbprint) == 0) {
                            signer_info.certificates.erase(iter);
                            break;
                        }
                    }
                    BuildCertPath(counter.signers_[0].issuer_info, counter.signers_[0].certificates, signer_info.certificates);
                    if (get_winroot_cert(counter.signers_[0].certificates)) {
                        counter.signers_[0].bfindrootcert = true;
                        counter.is_verified_ = true;
                    }
                }
            }
        }
    }

    if (get_winroot_cert(signer_info.certificates)) {
        signer_info.bfindrootcert = true;
    }

verify_cleanup:
    // Revert CRLs injection to PKCS7 object
    if (crl_injected)
        p7->d.sign->crl = original_crls;

    if (store_ctx)
        X509_STORE_CTX_free(store_ctx);

    // OpenSSL documentation:
    // "The certificates and CRLs in a store are used internally and should not
    // be freed up until after the associated X509_STORE_CTX is freed."
    if (crls)
        sk_X509_CRL_pop_free(crls, X509_CRL_free);
    if (sinfo && signer_info.issuer_info.issuer.length()) 
        sinfo->signers_.push_back(signer_info);
    return ret;
}

void CertManager::BuildCertPath(SignInfo::certificate_info& sign_info, std::vector<SignInfo::certificate_info>& new_cert_info, std::vector<SignInfo::certificate_info>& cert_info)
{
    std::string issuer_thumbprint = GetSha1(sign_info.issuer);
    if (!issuer_thumbprint.length())
        return;
    bool bfind = false;
    auto iter = cert_info.begin();
    for (; iter != cert_info.end(); iter++) {
        std::string subject_thumbprint = GetSha1(iter->subject);
        if (subject_thumbprint.length()) {
            if (issuer_thumbprint.compare(subject_thumbprint) == 0) {
                new_cert_info.push_back(*iter);
                cert_info.erase(iter);
                bfind = true;
                break;
            }
        }
    }
    if (bfind) {
        int pos = (int)new_cert_info.size();
        if (pos > 0) {
            BuildCertPath(new_cert_info[pos-1], new_cert_info, cert_info);
        }
    }

    return;
}

int CertManager::VerifyLeafHash(X509 *leaf, const char *leaf_digest_name, const char *leaf_hash, SignInfo* sinfo)
{
    std::string reason;

    const char *mdid = leaf_digest_name;
    const char *hash = leaf_hash;

    if (hash == NULL)
    {
        RecordError(*sinfo, dserror::vfy_leafhash_parse);
        return 0;
    }

    const EVP_MD *md = EVP_get_digestbyname(mdid);
    if (md == NULL)
    {
        RecordError(*sinfo, dserror::vfy_leafhash_lookup);
        return 0;
    }

    unsigned long sz = EVP_MD_size(md);
    unsigned long actual = strlen(hash);
    if (actual % 2 != 0)
    {
        RecordError(*sinfo, dserror::vfy_leafhash_length_uneven);
        return 0;
    }
    actual /= 2;
    if (actual != sz)
    {
        std::stringstream ss;
        ss << "\'" << mdid << "\' digest must be " << sz << " bytes long (got " << actual << " bytes)";
        RecordError(*sinfo, dserror::vfy_leafhash_length_mismatch, ss.str());
        return 0;
    }

    unsigned char mdbuf[EVP_MAX_MD_SIZE];
    unsigned char cmdbuf[EVP_MAX_MD_SIZE];
    unsigned long i = 0, j = 0;
    while (i < sz * 2)
    {
        unsigned char x;
        x = nib2val(hash[i + 1]);
        x |= nib2val(hash[i]) << 4;
        mdbuf[j] = x;
        i += 2;
        j += 1;
    }

    unsigned long certlen = i2d_X509(leaf, NULL);
    unsigned char *certbuf = (unsigned char*)malloc(certlen);
    unsigned char *tmp = certbuf;
    i2d_X509(leaf, &tmp);

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, certbuf, certlen);
    EVP_DigestFinal_ex(ctx, cmdbuf, NULL);
    EVP_MD_CTX_destroy(ctx);

    free(certbuf);

    if (memcmp(mdbuf, cmdbuf, EVP_MD_size(md)))
    {
        RecordError(*sinfo, dserror::vfy_leafhash_value_mismatch);
        return 0;
    }
    return 0;
}

/******************************************************************************
 * External Functions
 */

CertManager::CertManager(void)
{
    intermediate_ = NULL;
    trusted_ = NULL;
    winroot_ctx_ = nullptr;
    crl_store_initialized_ = false;
    last_err_msg_ = "";
    ResetRuntimeVariables();
    add_windows_root_certs();
}

CertManager::~CertManager(void)
{
    if (intermediate_)
        json_decref(intermediate_);
    if (trusted_)
        json_decref(trusted_);
    if (crl_store_initialized_)
    {
        //crl_store_db_.close();
        crl_store_initialized_ = false;
    }
    if (winroot_ctx_) {
        SSL_CTX_free(winroot_ctx_);
    }
}

std::string CertManager::GetLastErrorMessage()
{
    return last_err_msg_;
}

void CertManager::SetLastErrorMessage(std::string error_message)
{
    last_err_msg_ = error_message;
    return;
}

void CertManager::RecordError(SignInfo &sinfo, ErrorType error, std::string details /*= std::string()*/)
{
    if (sinfo.AddError(error, details))
    {
        SetLastErrorMessage(sinfo.errors_.back().second);
    }
    else
    {
        std::string err_msg = dserror::GenerateErrorMessage(error, details);
        SetLastErrorMessage(err_msg);
    }
    return;
}

void CertManager::RecordOpenSSLError(SignInfo &sinfo, unsigned long error_code, std::string error_message)
{
    if (sinfo.AddOpenSSLError(error_code, error_message))
    {
        SetLastErrorMessage(sinfo.openssl_errors_.back().second);
    }
    else
    {
        std::string err_msg = "OpenSSL error: " + error_message;
        SetLastErrorMessage(err_msg);
    }
    return;
}

bool CertManager::SetIntermediate(const char* intermediate, bool is_file /*= false*/)
{
    json_t *i_root = NULL;
    json_error_t j_error;

    if (intermediate_)
        json_decref(intermediate_);
    if (is_file)
        i_root = json_load_file(intermediate, 0, &j_error);
    else
        i_root = json_loads(intermediate, 0, &j_error);
    if (!i_root)
    {
        std::string err_prefix = "Initialization error: failed to set intermediate certificates: ";
        std::stringstream ss;
        ss << "line " << j_error.line << ": " << j_error.text;
        err_msg_intermediate_ = ss.str();
        SetLastErrorMessage(err_prefix + err_msg_intermediate_);
        return false;
    }
    else
    {
        err_msg_intermediate_.clear();
    }
    intermediate_ = i_root;
    return true;
}

bool CertManager::SetCrlStore(const char* crl_store)
{
    return true;
}

VerifyInfo CertManager::VerifyBuffer(
    const char* inbuf,
    size_t bufsize,
    bool crl_check /*= false*/,
    bool store_certificate /*= false*/,
    char* leaf_digest_name /*= NULL*/,
    char* leaf_hash /*= NULL*/)
{
    ResetRuntimeVariables();

    crl_check_enabled_ = crl_check;
    store_certificate_ = store_certificate;
    leaf_digest_name_ = leaf_digest_name;
    leaf_hash_ = leaf_hash;

    VerifyInfo vinfo;

    int verify_ret = -1;

    PeFileInfo pe_finfo = {};
    bool pe_res;

    bool success = map_data(inbuf, bufsize, pe_finfo);
    if (!success)
    {
        if (pe_finfo.filesize_ == 0)
        {
            RecordError(vinfo, dserror::io_empty_file);
        }
        if (pe_finfo.data_ == NULL)
        {
            RecordError(vinfo, dserror::io_cannot_map);
        }
        goto cleanup;
    }

    pe_finfo.fileend_ = pe_finfo.filesize_;
    pe_res = AnalyzePeStructure(&pe_finfo, vinfo);
    if (!pe_res)
    {
        goto cleanup;
    }

    verify_ret = VerifyPeFile(&vinfo, pe_finfo);
    if (vinfo.signers_.size()) {
        vinfo.is_verified_ = vinfo.signers_[0].bfindrootcert;
    }

cleanup:
    ERR_clear_error();
    return vinfo;
}

VerifyInfo CertManager::VerifyFile(
    const char *infile,
    bool crl_check /*= false*/,
    bool store_certificate /*= false*/,
    char *leaf_digest_name /*= NULL*/,
    char *leaf_hash /*= NULL*/)
{
    ResetRuntimeVariables();

    crl_check_enabled_ = crl_check;
    store_certificate_ = store_certificate;
    leaf_digest_name_ = leaf_digest_name;
    leaf_hash_ = leaf_hash;

    VerifyInfo vinfo;

    int verify_ret = -1;

    PeFileInfo pe_finfo = {};
    bool pe_res;

    bool success = map_file(infile, pe_finfo);
    if (!success)
    {
        if (pe_finfo.filesize_ == 0)
        {
            RecordError(vinfo, dserror::io_empty_file);
        }
        if (pe_finfo.data_ == NULL)
        {
            RecordError(vinfo, dserror::io_cannot_map);
        }
        goto cleanup;
    }
    
    pe_finfo.fileend_ = pe_finfo.filesize_;
    pe_res = AnalyzePeStructure(&pe_finfo, vinfo);
    if (!pe_res)
    {
        goto cleanup;
    }

    verify_ret = VerifyPeFile(&vinfo, pe_finfo);
    if (vinfo.signers_.size()) {
        vinfo.is_verified_ = vinfo.signers_[0].bfindrootcert;
    }

cleanup:
    ERR_clear_error();
    return vinfo;
}

int CertManager::Extract(const char *infile, bool output_pem, const char *outfile /*= NULL*/)
{
    int ret = 0;
    bool is_error = false;

    PeFileInfo pe_finfo;
    bool pe_res;
    BIO *hash = NULL, *outdata = NULL;
    PKCS7 *sig = NULL;
    const EVP_MD *md;
    SignInfo sinfo;

    ResetRuntimeVariables();

    do
    {
        bool success = map_file(infile, pe_finfo);
        if (!success)
        {
            if (pe_finfo.filesize_ == 0)
                SetLastErrorMessage(dserror::io_empty_file.second);
            if (pe_finfo.data_ == NULL)
            {
                is_error = true;
                SetLastErrorMessage(dserror::io_cannot_map.second);
            }
            break;
        }

        pe_finfo.fileend_ = pe_finfo.filesize_;
        pe_res = AnalyzePeStructure(&pe_finfo, sinfo);
        if (!pe_res)
        {
            is_error = true;
            break;
        }

        md = EVP_sha1();
        hash = BIO_new(BIO_f_md());
        BIO_set_md(hash, md);

        if (outfile)
        {
            // Create outdata file
            outdata = BIO_new_file(outfile, "w+b");
            if (outdata == NULL)
            {
                is_error = true;
                SetLastErrorMessage(dserror::extraction_error.second + ": failed to create file");
                break;
            }
        }
        else
        {
            outdata = BIO_new(BIO_s_mem());
            if (outdata == NULL)
            {
                is_error = true;
                SetLastErrorMessage(dserror::extraction_error.second + ": failed to create memory BIO");
                break;
            }
        }
        BIO_push(hash, outdata);

        // A lil' bit of ugliness. Reset stream, write signature and skip forward
        (void)BIO_reset(outdata);
        if (output_pem)
        {
            sig = extract_existing_pe_pkcs7(pe_finfo);
            if (!sig)
            {
                is_error = true;
                SetLastErrorMessage(dserror::extraction_error.second + ": unable to extract existing signature");
                break;
            }
            PEM_write_bio_PKCS7(outdata, sig);
            PKCS7_free(sig);
        }
        else
            BIO_write(outdata, pe_finfo.data_ + pe_finfo.sigpos_, pe_finfo.siglen_);

        if (!outfile)
        {
            char *data_ptr = NULL;
            long data_size = BIO_get_mem_data(outdata, &data_ptr);
            // Store certificate data
            if (data_ptr && data_size > 0)
                certificate_ = std::string(data_ptr, data_size);
        }
    } while (false);

    if (hash)
        BIO_free_all(hash);
    hash = outdata = NULL;

    if (is_error)
    {
        ERR_print_errors_fp(stderr);
        if (outfile)
        {
#ifdef _WIN32
            DeleteFileA(outfile);
#else
            unlink(outfile);
#endif
        }
        ret = -1;
    }

    ERR_clear_error();
    return ret;
}

std::string CertManager::GetCertificate()
{
    return certificate_;
}

void CertManager::add_winroot_cert_to_list(X509* cert)
{
    if (!cert)
        return;
    SignInfo::certificate_info info{};

    info.brootcert = true;
    info.version = get_certificate_version(X509_get_version(cert));
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    info.subject = subject;
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
    info.issuer = issuer;
    BIGNUM* serialbn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cert), NULL);
    char* serial = BN_bn2hex(serialbn);
    info.serial_number = serial;
    ASN1_TIME* time1 = X509_getm_notBefore(cert);
    info.time_from = convert_asn1_time_to_string(time1);
    ASN1_TIME* time2 = X509_getm_notAfter(cert);
    info.time_to = convert_asn1_time_to_string(time2);
    info.sign_algorithm = std::string(OBJ_nid2sn(X509_get_signature_nid(cert)));
    int mdnid, pknid, secbits;
    uint32_t flags;
    X509_get_signature_info(cert, &mdnid, &pknid, &secbits, &flags);

    {
        unsigned char* b = NULL;
        int n = ASN1_item_i2d((ASN1_VALUE*)cert, &b, ASN1_ITEM_rptr(X509));
        if (b != nullptr && n > 0) {
            info.thumbprint = GetSha1(std::string((char*)b, n));
        }
        if (b) OPENSSL_free(b);
    }

    OPENSSL_free(subject);
    OPENSSL_free(issuer);
    OPENSSL_free(serial);
    BN_free(serialbn);

    std::string sha1_string = GetSha1(info.subject);
    if (sha1_string.length()) {
        auto find = winroot_certs_.find(sha1_string);
        if (find == winroot_certs_.end()) {
            winroot_certs_[sha1_string] = info;
        }
    }
    return;
}

bool CertManager::get_winroot_cert(std::vector<SignInfo::certificate_info>& cert_info)
{
    bool bfind = false;
    if (!cert_info.size()) {
        return bfind;
    }
    if (!winroot_certs_.size()) {
        return bfind;
    }

    auto iter = cert_info.begin();
    for (; iter != cert_info.end(); iter++) {
        if (iter->issuer_thumbprint.length()) {
            auto find = winroot_certs_.find(iter->issuer_thumbprint);
            if (find != winroot_certs_.end()) {
                // add root certificates
                bool badd = true;
                for (auto& it : cert_info) {
                    if (find->second.thumbprint.compare(it.thumbprint) == 0) {
                        it.brootcert = true;
                        badd = false;
                        break;
                    }
                }
                if(badd)
                    cert_info.push_back(find->second);
                bfind = true;
                break;
            }
        }
    }
    return bfind;
}

void CertManager::add_windows_root_certs()
{
    winroot_ctx_ = SSL_CTX_new(TLS_client_method());
    if (!winroot_ctx_) {
        return;
    }
    HCERTSTORE hStore = CertOpenSystemStoreA(0, "ROOT");
    if (hStore == NULL) {
        return;
    }

    X509_STORE* store = X509_STORE_new();
    PCCERT_CONTEXT pContext = NULL;
    while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
        X509* x509 = d2i_X509(NULL,
            (const unsigned char**)&pContext->pbCertEncoded,
            pContext->cbCertEncoded);
        if (x509 != NULL) {
            X509_STORE_add_cert(store, x509);
            add_winroot_cert_to_list(x509);
            X509_free(x509);
        }
    }

    CertFreeCertificateContext(pContext);
    CertCloseStore(hStore, 0);

    SSL_CTX_set_cert_store(winroot_ctx_, store);
}

extern "C"
{
#include <openssl/applink.c>
}
