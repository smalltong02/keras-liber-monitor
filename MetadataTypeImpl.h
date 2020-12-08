//===- MetadataTypeImpl.h - metadata type implement define -------------------*- C++ -*-===//
// 
// This file contains the standrad type, additional type and metadata data class define.
//
//===-----------------------------------------------------------------------------------===//
#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <variant>
#include "ExceptionThrow.h"
#include "utils.h"

namespace cchips {

    //the base class of all data object.
    class CObObject
    {
    public:
        enum _ob_type {
            ob_invalid = 0,
            ob_basetype,
            ob_reference,
            ob_stringref,
            ob_array,
            ob_tuple,
            ob_struct,
            ob_flag,
            ob_symboltable,
            ob_max,
        };
        enum _c_operator
        {
            op_invalid = 0,
            op_equal,
            op_n_equal,
            op_greater,
            op_greater_e,
            op_less,
            op_less_e,
            op_max,
        };

        CObObject() : m_ob_type(ob_invalid), m_ob_size(0), m_op(op_n_equal) {}
        virtual ~CObObject() = default;
        _ob_type GetObType() const { return m_ob_type; }
        size_t GetObSize() const { return m_ob_size; }
        _c_operator GetOp() const { return m_op; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual bool IsValidValue(char* pdata) const = 0;
        virtual std::any GetCurValue() const = 0;
        virtual std::any GetValue(char* pdata) const = 0;
        virtual bool SetCurValue(const std::any& anyvalue) = 0;
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const = 0;
        virtual bool Success(char* pdata) const = 0;
        virtual bool Failed(char* pdata) const = 0;
        bool IsReference() const {
            if (m_ob_type == ob_reference)
                return true;
            return false;
        }
        bool IsStringref() const {
            if (m_ob_type == ob_stringref)
                return true;
            return false;
        }
        bool IsCommReference() const {
            if (IsReference() || IsStringref())
                return true;
            return false;
        }
        bool IsBaseTy() const {
            if (m_ob_type == ob_basetype)
                return true;
            return false;
        }
        bool IsStruct() const {
            if (m_ob_type == ob_struct)
                return true;
            return false;
        }
        bool IsFlag() const {
            if (m_ob_type == ob_flag)
                return true;
            return false;
        }
        bool IsArray() const {
            if (m_ob_type == ob_array)
                return true;
            return false;
        }
        bool IsTuple() const {
            if (m_ob_type == ob_tuple)
                return true;
            return false;
        }
    protected:
        bool SetObType(_ob_type type) {
            ASSERT(type != ob_invalid && type < ob_max);
            if (type != ob_invalid && type < ob_max)
                m_ob_type = type;
            return false;
        }
        void SetObSize(size_t size) {
            m_ob_size = size;
        }
        void SetOp(_c_operator op) {
            ASSERT(op != op_invalid && op < op_max);
            if (op != op_invalid && op < op_max)
                m_op = op;
            return;
        }
    private:
        _c_operator m_op;
        size_t m_ob_size;
        _ob_type m_ob_type;
    };

    using TYPE_SYMBOLTABLE = std::map<std::string, std::shared_ptr<CObObject>>;
    using IDENPAIR = std::pair<std::string, std::shared_ptr<CObObject>>;

    // the base type of define, the based on C++ standrad type define.
    class CBaseDef
    {
    public:
        enum _standard_type
        {
            type_invalid = 0, type_void, type_byte, type_bool, type_char, type_uchar, type_int16, type_uint16, type_short,
            type_ushort, type_word, type_wchar, type_int, type_uint, type_long, type_long_ptr, type_ulong, type_ulong_ptr,
            type_dword, type_dword_ptr, type_float, type_int64, type_uint64, type_longlong, type_ulonglong, type_qword,
            type_double, type_large_integer, type_ularge_integer, type_lpvoid, type_hmodule, type_hinstance, type_hresult, 
            type_schandle, type_ntstatus, type_string, type_wstring, type_handle, type_guid, type_max,
        };

        CBaseDef() : _base_def(type_invalid) {}
        virtual ~CBaseDef() = default;
        _standard_type GetBaseDef() const { return _base_def; }
        const static std::string& GetBaseStr(_standard_type def) {
            ASSERT(def != type_invalid && def < type_max);
            if (def != type_invalid && def < type_max)
            {
                const auto& p = _base_to_str_def.find(def);
                if (p != _base_to_str_def.end()) { return (*p).second; }
            }
            return _base_to_str_def.begin()->second;
        }
        size_t GetBaseSize() {
            const auto& p = _base_to_size_def.find(_base_def);
            if (p == _base_to_size_def.end()) { return _base_to_size_def.begin()->second; }
            return p->second;
        }

    protected:
        bool SetBaseDef(CBaseDef::_standard_type def) {
            ASSERT(def != type_invalid && def < type_max);
            if (def != type_invalid && def < type_max)
            {
                const auto& p = _base_to_size_def.find(def);
                if (p == _base_to_size_def.end()) { return false; }
                _base_def = def;
                return true;
            }
            return false;
        }
    private:
        // the list of a pair for base define and size.
        static const std::map<_standard_type, size_t> _base_to_size_def;
        // the list of a pair for base define and string name.
        static const std::map<_standard_type, std::string> _base_to_str_def;
        _standard_type _base_def;
    };

    // the type define of cchips, contains type information and opration.
    template <typename T>
    class CBaseType : public CBaseDef, public CObObject
    {
    public:
        CBaseType() = delete;
        CBaseType(CBaseDef::_standard_type base_def, T&& init_value = {}, CObObject::_c_operator op = CObObject::op_n_equal) { SetObType(CObObject::ob_basetype); SetBaseDef(base_def); SetObSize(GetBaseSize()); SetOp(op); _value = std::move(init_value); }
        CBaseType(const CBaseType& ctype) { SetObType(ctype.GetObType()); SetBaseDef(ctype.GetBaseDef()); SetObSize(static_cast<CBaseDef>(ctype).GetBaseSize()); SetOp(ctype.GetOp()); _value = std::any_cast<T>(ctype.GetCurValue()); }
        ~CBaseType() = default;

        CBaseType<T>& operator=(CBaseType<T> const& ctype)
        {
            SetObType(ctype.GetObType()); SetBaseDef(ctype.GetBaseDef()); SetObSize(ctype.GetBaseSize());
            return *this;
        }
        bool operator==(CBaseType<T> const& rhs)
        {
            if (GetBaseDef() != rhs.GetBaseDef()) return false;
            if (GetObType() != rhs.GetObType()) return false;
            if (GetObSize() != rhs.GetObType()) return false;
            return true;
        }
        const std::string& GetName() const override { return CBaseDef::GetBaseStr(GetBaseDef()); }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { return nullptr; }
        bool IsValidValue(char* pdata) const override {
            if (GetBaseDef() == CBaseDef::type_void)
                return true;
            //ASSERT(pdata != nullptr);
            T* p = reinterpret_cast<T*>(pdata);
            if (!p) return false;
            // please check valid memory at here.
            return true;
        }
        std::any GetCurValue() const override {
            if (GetBaseDef() == CBaseDef::type_void)
                return {};
            return _value;
        }
        std::any GetValue(char* pdata) const override {
            if (GetBaseDef() != CBaseDef::type_void && IsValidValue(pdata))
            {
                T* p = reinterpret_cast<T*>(pdata);
                tls_check_struct *tls = check_get_tls();
                if (tls) {
                    T checked_p;
                    tls->active = 1;
                    if (setjmp(tls->jb) == 0) {
                        checked_p = *p;
                        tls->active = 0;
                        return checked_p;
                    }
                    //execption occur
                    tls->active = 0;
                }
                else {
                    return *p;
                }
            }
            return {};
        }
        bool SetValue(char* pdata, const std::any& anyvalue) const override;
        bool SetCurValue(const std::any& anyvalue) override {
            if (GetBaseDef() == CBaseDef::type_void)
                return false;
            if (anyvalue.has_value() && anyvalue.type() == typeid(T))
            {
                _value = std::any_cast<T>(anyvalue);
                return true;
            }
            return false;
        }
        bool Success(char* pdata) const override {
            if (GetBaseDef() == CBaseDef::type_void)
                return true;
            std::any anyvalue = GetValue(pdata);
            if (anyvalue.has_value() && anyvalue.type() == typeid(T))
            {
                T checked_p = std::any_cast<T>(anyvalue);
                if (GetOp() == op_n_equal)
                {
                    if (checked_p == _value)
                        return false;
                    else
                        return true;
                }
                else if (GetOp() == op_greater)
                {
                    if (checked_p > _value)
                        return true;
                    else
                        return false;
                }
                else if (GetOp() == op_greater_e)
                {
                    if (checked_p >= _value)
                        return true;
                    else
                        return false;
                }
                else if (GetOp() == op_less)
                {
                    if (checked_p < _value)
                        return true;
                    else
                        return false;
                }
                else if (GetOp() == op_less_e)
                {
                    if (checked_p <= _value)
                        return true;
                    else
                        return false;
                }
                if (checked_p == _value)
                    return true;
            }
            return false;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        T _value;
    };

    // the deleter for type define
    template <typename T>
    struct CDeleter
    {
        inline void operator()(void* data)
        {
            delete static_cast<T*>(data);
        }
    };

    template<class T>
    struct CDeleter<T[]>
    {
        inline void operator()(void* data)
        {
            delete[] static_cast<T*>(data);
        }
    };

    // the metadata type class object, the based on type define of cchips
    class CMetadataTypeObject : public CObObject, public std::enable_shared_from_this<CMetadataTypeObject>
    {
    public:
        CMetadataTypeObject() = delete;
        CMetadataTypeObject(const CMetadataTypeObject& p) = delete;
        CMetadataTypeObject(const std::string& name, std::shared_ptr<CObObject>& pbasedef) : metadata_name(name), m_pmetadatadef(std::move(pbasedef)) {
            SetObType(m_pmetadatadef->GetObType()); SetObSize(m_pmetadatadef->GetObSize()); SetOp(m_pmetadatadef->GetOp());
        }
        CMetadataTypeObject(const std::string& name, const std::shared_ptr<CMetadataTypeObject>& pmetadata) : metadata_name(name) {
            m_pmetadatadef = pmetadata->GetMetadataDef(); SetObType(m_pmetadatadef->GetObType()); SetObSize(m_pmetadatadef->GetObSize()); SetOp(m_pmetadatadef->GetOp());
        }
        ~CMetadataTypeObject() = default;
        CMetadataTypeObject& operator=(const CMetadataTypeObject& p) = delete;
        const std::string& GetName() const { return metadata_name; }
        const std::shared_ptr<CObObject> GetInstance() { return shared_from_this(); }
        const std::shared_ptr<CObObject> GetMetadataDef() const override { return m_pmetadatadef; }
        virtual bool IsValidValue(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->IsValidValue(pdata);
            return false;
        }
        virtual std::any GetCurValue() const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetCurValue();
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetValue(pdata);
            return {};
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            if (GetMetadataDef())
                return GetMetadataDef()->SetCurValue(anyvalue);
            return false;
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->SetValue(pdata, anyvalue);
            return false;
        }
        virtual bool Success(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->Success(pdata);
            return false;
        }
        virtual bool Failed(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->Failed(pdata);
            return false;
        }
    private:
        std::string metadata_name;
        std::shared_ptr<CObObject> m_pmetadatadef;
    };
    // initialize and make the metadata type class object and return the object reference.
    template <typename Ty>
    inline std::shared_ptr<CMetadataTypeObject> make_metadata_s_ptr(CBaseDef::_standard_type def) noexcept
    {
        CBaseType<Ty>* base_ptr = new CBaseType<Ty>(def);
        if (!base_ptr) return nullptr;
        std::shared_ptr<CBaseType<Ty>> ptr(base_ptr, CDeleter<CBaseType<Ty>>());
        return std::make_shared<CMetadataTypeObject>(CBaseDef::GetBaseStr(def), std::static_pointer_cast<CObObject>(ptr));
    }
    // initialize and make the metadata type class object and return the object reference.
    template <typename Ty>
    inline std::shared_ptr<CMetadataTypeObject> make_metadata_j_ptr(CBaseDef::_standard_type def, Ty&& init_value, CObObject::_c_operator op) noexcept
    {
        CBaseType<Ty>* base_ptr = new CBaseType<Ty>(def, std::forward<Ty>(init_value), op);
        if (!base_ptr) return nullptr;
        std::shared_ptr<CBaseType<Ty>> ptr(base_ptr, CDeleter<CBaseType<Ty>>());
        return std::make_shared<CMetadataTypeObject>(CBaseDef::GetBaseStr(def), std::static_pointer_cast<CObObject>(ptr));
    }
    // the reference class object, It is a reference of other data object.
    class CReferenceObject : public CObObject
    {
    public:
        CReferenceObject() = delete;
        CReferenceObject(const CReferenceObject& p) = delete;
        CReferenceObject(const std::string_view& name) : m_refername(name) { SetObType(CObObject::ob_reference); SetObSize(sizeof(PVOID)); }
        CReferenceObject(const std::string& name, std::shared_ptr<CObObject> pelement) : m_refername(name), m_pelement(std::move(pelement)) {
            ResetOb();
        }
        ~CReferenceObject() = default;
        CReferenceObject& operator=(const CReferenceObject& p) = delete;
        virtual bool AddReference(std::shared_ptr<CObObject> pelement) {
            m_pelement = std::move(pelement);
            ResetOb();
            return true;
        }
        const std::shared_ptr<CObObject> GetData() const { return m_pelement; }
        virtual const std::string& GetName() const override { return m_refername; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { 
            if (!m_pelement) return nullptr;
            return (*m_pelement).GetMetadataDef(); 
        }
        virtual bool IsValidValue(char* pdata) const override {
            if (reinterpret_cast<const PVOID*>(pdata) == nullptr)
                return false;
            // the lpModuleName parameter could be NULL in API GetModuleHandle().
            const PVOID* pp = reinterpret_cast<const PVOID*>(pdata);
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                PVOID checked_p;
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    checked_p = *pp;
                    tls->active = 0;
                    if (checked_p == nullptr)
                        return false;
                }
                //execption occur
                tls->active = 0;
            }
            else {
                if (*pp == nullptr)
                    return false;
            }
            return true;
        }
        virtual std::any GetCurValue() const override {
            if (m_pelement && GetObType() == ob_stringref) {
                if (std::static_pointer_cast<CBaseType<CHAR>>(m_pelement->GetMetadataDef())->GetBaseDef() == CBaseDef::type_wchar)
                {
                    return std::wstring();
                }
                return std::string();
            }
            return (PVOID)nullptr;
        }
        virtual std::any GetValue(char* pdata) const override {
            if (!IsValidValue(pdata))
            {
                return {};
            }
            if (m_pelement && GetObType() == ob_stringref)
            {
                if (std::static_pointer_cast<CBaseType<CHAR>>(m_pelement->GetMetadataDef())->GetBaseDef() == CBaseDef::type_wchar)
                {
                    std::wstring str_value;
                    tls_check_struct *tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            str_value = std::wstring(*reinterpret_cast<const wchar_t**>(pdata));
                            tls->active = 0;
                            return str_value;
                        }
                        //execption occur
                        tls->active = 0;
                    }
                }
                else
                {
                    std::string str_value;
                    tls_check_struct *tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            str_value = std::string(*reinterpret_cast<const char**>(pdata));
                            tls->active = 0;
                            return str_value;
                        }
                        //execption occur
                        tls->active = 0;
                    }
                }
            }
            else
                return *reinterpret_cast<const PVOID*>(pdata);
            return {};
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            if (GetMetadataDef())
            {
                return GetMetadataDef()->SetCurValue(anyvalue);
            }
            return false;
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override;
        virtual bool Success(char* pdata) const override {
            std::any anyvalue = GetValue(pdata);
            if (anyvalue.has_value())
                return true;
            return false;
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        void ResetOb() {
            if (m_pelement)
            {
                if (m_pelement->IsBaseTy() && IsStringType(std::static_pointer_cast<CBaseType<CHAR>>(m_pelement->GetMetadataDef())->GetBaseDef()))
                {
                    // the string is a special reference
                    SetObType(CObObject::ob_stringref); SetObSize(sizeof(PVOID));
                }
                else
                {
                    SetObType(CObObject::ob_reference); SetObSize(sizeof(PVOID));
                }
            }
        }
        static bool IsStringType(int type) {
            for (auto& i : _str_ref_types)
            {
                if (type == i) return true;
            }
            return false;
        }
        static const std::vector<CBaseDef::_standard_type> _str_ref_types;
        std::string m_refername;
        std::shared_ptr<CObObject> m_pelement;
    };
    // the array class object, it could be contains variable length and the same type of data.
    class CArrayObject : public CObObject
    {
    public:
        CArrayObject() = delete;
        CArrayObject(const CArrayObject& p) = delete;
        CArrayObject(const std::string_view& name) : m_arrayname(name), m_dim(0) { SetObType(CObObject::ob_array); SetObSize(0); }
        CArrayObject(const std::string& name, const std::shared_ptr<CObObject> pelement, unsigned int dim) : m_arrayname(name), m_pelement(std::move(pelement)), m_dim(dim) { SetObType(CObObject::ob_array); SetObSize(m_pelement->GetObSize()*dim); }
        ~CArrayObject() = default;
        CArrayObject& operator=(const CArrayObject& p) = delete;
        bool AddArray(const std::shared_ptr<CObObject> pelement, unsigned int dim) {
            if (!m_pelement)
            {
                m_dim = dim;
                m_pelement = std::move(pelement);
                SetObSize(m_pelement->GetObSize()*dim);
                return true;
            }
            return false;
        }
        unsigned int GetDim() const { return m_dim; }
        void SetDim(unsigned int dim) { m_dim = dim; }
        virtual const std::string& GetName() const override { return m_arrayname; }
        const std::shared_ptr<CObObject> GetData() const { return m_pelement; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { 
            if (!m_pelement) return nullptr;
            return (*m_pelement).GetMetadataDef(); 
        }
        virtual bool IsValidValue(char* pdata) const override {
            if (pdata == nullptr) return false;
            return true;
        }
        virtual std::any GetCurValue() const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetCurValue();
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
            if (!IsValidValue(pdata)) return {};
            if (GetMetadataDef())
                return GetMetadataDef()->GetValue(pdata);
            return {};
        }
        std::any GetValue(char* pdata, unsigned int sublin) const {
            if (!IsValidValue(pdata)) return {};
            if (sublin >= m_dim) return {};
            if (GetMetadataDef()) {
                unsigned int offset = (unsigned int)GetMetadataDef()->GetObSize() * sublin;
                return GetMetadataDef()->GetValue(pdata + offset);
            }
            return {};
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            if (GetMetadataDef())
            {
                return GetMetadataDef()->SetCurValue(anyvalue);
            }
            return false;
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override {
            if (!IsValidValue(pdata)) return false;
            if (!GetMetadataDef()) return false;
            return GetMetadataDef()->SetValue(pdata, anyvalue);
        }
        bool SetValue(char* pdata, const std::any& anyvalue, unsigned int sublin) const {
            if (!IsValidValue(pdata)) return false;
            if (sublin >= m_dim) return false;
            if (GetMetadataDef()) {
                unsigned int offset = (unsigned int)GetMetadataDef()->GetObSize() * sublin;
                return GetMetadataDef()->SetValue(pdata + offset, anyvalue);
            }
            return {};
        }
        bool SetArray(char* pdata, const std::any& anyvalue, unsigned int dim) const {
            if (!IsValidValue(pdata)) return false;
            if (dim > m_dim) return false;
            if (GetMetadataDef()) {
                bool bret = false;
                for (unsigned int sublin = 0; sublin < dim; sublin++)
                {
                    unsigned int offset = (unsigned int)GetMetadataDef()->GetObSize() * sublin;
                    bret = GetMetadataDef()->SetValue(pdata + offset, anyvalue);
                    if (!bret) break;
                }
                return bret;
            }
            return {};
        }
        bool SetArray(char* pdata, char* psrc_data, unsigned int dim) const {
            if (!IsValidValue(pdata)) return false;
            if (!IsValidValue(psrc_data)) return false;
            if (dim > m_dim) return false;
            if (GetMetadataDef()) {
                bool bret = false;
                for (unsigned int sublin = 0; sublin < dim; sublin++)
                {
                    unsigned int offset = (unsigned int)GetMetadataDef()->GetObSize() * sublin;
                    std::any anyvalue = GetMetadataDef()->GetValue(psrc_data + offset);
                    bret = GetMetadataDef()->SetValue(pdata + offset, anyvalue);
                    if (!bret) break;
                }
                return bret;
            }
            return {};
        }
        unsigned int GetElementOffset(unsigned int dim) {
            if (!GetData()) return InvalidOffset;
            return GetData()->GetObSize() * dim;
        }
        virtual bool Success(char* pdata) const override {
            return true;
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
        static const int InvalidOffset = -1;
    private:
        unsigned int m_dim;
        std::string m_arrayname;
        std::shared_ptr<CObObject> m_pelement;
    };
    // the struct class object
    class CStructObject : public CObObject
    {
    public:
        using ElementTypeDefine = std::pair<std::string, std::pair<unsigned int, std::shared_ptr<CObObject>>>;
        using ElementListDefine = std::map<std::string, ElementTypeDefine>;
        using iterator = ElementListDefine::iterator;
        using const_iterator = ElementListDefine::const_iterator;

        CStructObject() = delete;
        CStructObject(const CStructObject& p) = delete;
        CStructObject(const std::string_view& name) : m_structname(name) { SetObType(CObObject::ob_struct); SetObSize(0); }
        CStructObject(const std::string& name, const ElementListDefine& pstruct) : m_structname(name) {
            SetObType(CObObject::ob_struct);
            AddStruct(pstruct);
        }
        ~CStructObject() = default;
        CStructObject& operator=(const CStructObject& p) = delete;
        bool AddStruct(const ElementListDefine& pstruct)
        {
            size_t size = 0;
            for (auto& it : pstruct)
            {
                if (!(it.second.second.second)) return false;
                size += (it.second.second.second)->GetObSize();
            }
            m_pstruct = pstruct;
            SetObSize(size);
            return true;
        }
        bool AddElement(const std::string_view& name, ElementTypeDefine& pelement) {
            ASSERT(name.length());
            if (!name.length()) return false;
            std::string name_str(name);
            auto& it = m_pstruct.find(name_str);
            if (it != m_pstruct.end() && it->second.second.second != nullptr) return false;
            m_pstruct[name_str] = std::move(pelement);
            m_pstruct[name_str].second.first = (int)GetObSize();
            if (m_pstruct[name_str].second.second != nullptr)
            {
                SetObSize(GetObSize() + m_pstruct[name_str].second.second->GetObSize());
                return true;
            }
            else
                SetObSize(GetObSize() + 4);
            return false;
        }
        virtual const std::string& GetName() const override { return m_structname; }
        const ElementListDefine& GetData() const { return m_pstruct; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { return nullptr; }
        virtual bool IsValidValue(char* pdata) const override {
            if (pdata == nullptr) return false;
            return true;
        }
        virtual std::any GetCurValue() const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetCurValue();
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
            if (!m_pstruct.size()) return {};
            std::shared_ptr<CObObject> ob_ptr = GetElement(0);
            if (!ob_ptr) return {};
            return ob_ptr->GetValue(pdata);
        }
        std::any GetValue(char* pdata, const std::string& name) const {
            if (!name.length()) return {};
            std::shared_ptr<CObObject> ob_ptr = GetElement(name);
            if (!ob_ptr) return {};
            unsigned int offset = GetElementOffset(name);
            return ob_ptr->GetValue(pdata + offset);
            return {};
        }
        std::any GetValue(char* pdata, unsigned int index) const {
            std::shared_ptr<CObObject> ob_ptr = GetElement(index);
            if (!ob_ptr) return {};
            unsigned int offset = GetElementOffset(index);
            return ob_ptr->GetValue(pdata + offset);
            return {};
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            return false;
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override {
            if (!m_pstruct.size()) return {};
            std::shared_ptr<CObObject> ob_ptr = GetElement(0);
            if (!ob_ptr) return {};
            return ob_ptr->SetValue(pdata, anyvalue);
            return {};
        }
        bool SetValue(char* pdata, const std::string& name, const std::any& anyvalue) const {
            if (!name.length()) return {};
            std::shared_ptr<CObObject> ob_ptr = GetElement(name);
            if (!ob_ptr) return {};
            unsigned int offset = GetElementOffset(name);
            return ob_ptr->SetValue(pdata + offset, anyvalue);
            return {};
        }
        virtual bool Success(char* pdata) const override {
            return true;
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
        std::shared_ptr<CObObject> GetElement(const std::string& name) const {
            if (!name.length()) return nullptr;
            for (const auto& i : m_pstruct)
            {
                if (_stricmp(i.first.c_str(), name.c_str()) == 0)
                {
                    return i.second.second.second;
                }
            }
            return nullptr;
        }
        std::shared_ptr<CObObject> GetElement(unsigned int index) const {
            if (index < m_pstruct.size())
            {
                std::map<unsigned int, std::shared_ptr<CObObject>> list;
                for (const auto& i : m_pstruct)
                {
                    list[i.second.second.first] = i.second.second.second;
                }
                for (const auto& l : list)
                {
                    if(index == 0) return l.second;
                    index--;
                }
            }
            return nullptr;
        }
        unsigned int GetElementIdx(const std::string& name) const {
            unsigned int index = 0;
            std::map<unsigned int, std::string> list;
            for (const auto& i : m_pstruct)
            {
                list[i.second.second.first] = i.first;
            }
            for (const auto& l : list)
            {
                if (_stricmp(l.second.c_str(), name.c_str()) == 0)
                {
                    return index;
                }
                index++;
            }
            return static_cast<unsigned int>(-1);
        }
        unsigned int GetElementOffset(const std::string& name) const {
            if (!name.length()) return InvalidOffset;
            const auto p = m_pstruct.find(name);
            if (p == m_pstruct.end()) return InvalidOffset;
            return p->second.second.first;
        }
        unsigned int GetElementOffset(unsigned int index) const {
            if (index < m_pstruct.size())
            {
                std::map<unsigned int, std::shared_ptr<CObObject>> list;
                for (const auto& i : m_pstruct)
                {
                    list[i.second.second.first] = i.second.second.second;
                }
                for (const auto& j : list)
                {
                    if (index == 0) return j.first;
                    index--;
                }
            }
            return InvalidOffset;
        }
        static const int InvalidOffset = -1;
        iterator        begin() { return m_pstruct.begin(); }
        const_iterator  begin() const { return m_pstruct.begin(); }
        iterator        end() { return m_pstruct.end(); }
        const_iterator  end() const { return m_pstruct.end(); }
    private:
        std::string m_structname;
        ElementListDefine m_pstruct;
    };
    // the flag class object
    class CFlagObject : public CObObject
    {
    public:
        enum _flag_operation {
            flag_invalid = 0,
            flag_and,
            flag_or,
            flag_not,
        };
        CFlagObject() = delete;
        CFlagObject(const CFlagObject& p) = delete;
        CFlagObject(const std::string_view& name) : m_flagname(name), m_operation(flag_invalid) { SetObType(CObObject::ob_flag); SetObSize(0); }
        ~CFlagObject() = default;
        CFlagObject& operator=(const CFlagObject& p) = delete;
        static _flag_operation GetFlagOp(const std::string& op_name)
        {
            if (_stricmp(op_name.c_str(), "OR") == 0)
                return flag_or;
            else if (_stricmp(op_name.c_str(), "AND") == 0)
                return flag_and;
            else if (_stricmp(op_name.c_str(), "NOT") == 0)
                return flag_not;
            return flag_invalid;
        }
        _flag_operation GetFlagOp() const
        {
            return m_operation;
        }
        bool AddFlags(const std::shared_ptr<CObObject>& pdata, _flag_operation op, std::unique_ptr<std::map<std::string, unsigned int>> values) {
            if (!pdata || (*values).size() == 0) return false;
            if (op == flag_invalid || op > flag_not) return false;
            ASSERT(pdata->IsBaseTy()); // m_pdata must be ob_basetype;
            m_operation = op;
            m_pflagvalues = std::move(values);
            m_pdata = pdata;
            SetObSize(m_pdata->GetObSize());
            return true;
        }
        virtual const std::string& GetName() const override { return m_flagname; }
        const std::shared_ptr<CObObject> GetData() const { return m_pdata; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { return (*m_pdata).GetMetadataDef(); }
        int GetFlagVa(const std::string& value_str) const {
            if (!value_str.length()) return InvalidFlagVa;
            if (!m_pflagvalues) return InvalidFlagVa;
            auto& it = (*m_pflagvalues).find(value_str);
            if (it == (*m_pflagvalues).end()) return InvalidFlagVa;
            return it->second;
        }
        std::any GetFlagStr(unsigned int flag_value) const {
            if (!m_pflagvalues) return {};
            if (flag_value == InvalidFlagVa) return {};
            if (m_operation == flag_or) {
                bool is_first = true;
                std::ostringstream flag_list;
                std::transform(m_pflagvalues->begin(), m_pflagvalues->end(), std::ostream_iterator<std::string>{flag_list}, [&](const auto& flag) {
                    if ((flag.second & flag_value) == flag.second) {
                        flag_value = flag_value ^ flag.second;
                        if (!is_first)
                            return std::string(" | ") + flag.first;
                        is_first = false;
                        return flag.first;
                    }
                    return std::string{};
                    });
                if (flag_list.str().length())
                {
                    if(flag_value == 0)
                        return flag_list.str();
                    std::ostringstream ostr;
                    ostr << std::hex << "0x" << flag_value;
                    return flag_list.str() + std::string(" | unknown-flags:") + ostr.str();
                }
            }
            else {
                for (const auto& flag : *m_pflagvalues)
                {
                    if (flag.second == flag_value)
                        return flag.first;
                }
            }
            {
                std::ostringstream ostr;
                ostr << std::hex << "0x" << flag_value;
                std::string ret_string = std::string("unknown-flags:") + ostr.str();
                return ret_string;
            }
        }
        virtual bool IsValidValue(char* pdata) const override {
            if (!m_pdata) return false;
            return m_pdata->IsValidValue(pdata);
            return false;
        }
        virtual std::any GetCurValue() const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetCurValue();
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
                if (!m_pdata) return {};
                return m_pdata->GetValue(pdata);
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            if (!m_pdata) return false;
            return m_pdata->SetCurValue(anyvalue);
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override {
            if (!m_pdata) return {};
            return m_pdata->SetValue(pdata, anyvalue);
        }
        virtual bool Success(char* pdata) const override {
            return true;
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
        static const int InvalidFlagVa = -1;
    private:
        std::string m_flagname;
        std::shared_ptr<CObObject> m_pdata;
        std::unique_ptr<std::map<std::string, unsigned int>> m_pflagvalues;
        _flag_operation m_operation;
    };
    // the tuple class object, it could be contains variable length and variable type of data.
    class CTupleObject : public CObObject
    {
    public:
        using function_type = enum {
            function_isvalidvalue = 0,
            function_getvalue,
            function_setvalue,
            function_success,
        };
        enum class _tuple_ref {
            tuple_ref_addr = 0,
            tuple_ref_val,
        };
        enum class _tuple_flg {
            tuple_flg_int = 0,
            tuple_flg_str,
        };
        struct _tuple_elem {
            union {
                struct _ref {
                    _tuple_ref tuple_ref;
                } ref;
                struct _arr {
                    unsigned int dim;
                } arr;
                struct _struc {
                    unsigned int idx;
                } struc;
                struct _flag {
                    _tuple_flg tuple_flg;
                } flag;
            };
            std::string elem_name;
            std::shared_ptr<CObObject> ptr;
        };
        using iterator = std::vector<_tuple_elem>::iterator;
        using const_iterator = std::vector<_tuple_elem>::const_iterator;
        CTupleObject() = delete;
        CTupleObject(const CTupleObject& p) = delete;
        CTupleObject(const std::string_view name) : m_tuplename(name) { SetObType(CObObject::ob_tuple); SetObSize(0); }
        CTupleObject(const std::string_view name, const std::vector<_tuple_elem>& ptuple) : m_tuplename(name) {
            SetObType(CObObject::ob_tuple);
            AddTuple(ptuple);
        }
        ~CTupleObject() = default;
        CTupleObject& operator=(const CTupleObject& p) = delete;
        unsigned int GetElemCount() const { return (unsigned int)m_ptuple.size(); }
        bool AddTuple(const std::vector<_tuple_elem>& ptuple)
        {
            size_t size = 0;
            for (auto& it : ptuple)
            {
                if (!it.ptr) return false;
                size = it.ptr->GetObSize();
            }
            m_ptuple = ptuple;
            SetObSize(size);
            return true;
        }
        bool AddElement(const _tuple_elem& pelement)
        {
            if (!pelement.ptr) return false;
            m_ptuple.push_back(pelement);
            SetObSize(GetObSize() + pelement.ptr->GetObSize());
            return true;
        }
        std::any GetElement(unsigned int index) {
            if (!GetElemCount()) return {};
            if (index != static_cast<unsigned int>(-1) && (index > GetElemCount()))
                return {};
            if (index == static_cast<unsigned int>(-1))
                index = GetElemCount() - 1;
            return &(m_ptuple.at(index));
        }
        virtual const std::string& GetName() const override { return m_tuplename; }
        const std::vector<_tuple_elem>& GetData() const { return m_ptuple; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { 
            if (m_ptuple.size() == 1)
                return m_ptuple[0].ptr;
            return nullptr;
        }

        std::variant<bool, std::any> PolyImplFunction(function_type f_type, char* pdata, const std::any anyval) const
        {
            auto default_rr = [=]() ->std::variant<bool, std::any> { if (f_type == function_getvalue) return std::any{}; else return false; };
            auto call_func = [](std::shared_ptr<CObObject> object, function_type f_type, char* pdata, const std::any anyval) ->std::variant<bool, std::any> {
                switch (f_type)
                {
                case function_success:
                {
                    const auto func = std::bind(&CObObject::Success, object, std::placeholders::_1);
                    return func(pdata);
                }
                break;
                case function_isvalidvalue:
                {
                    const auto func = std::bind(&CObObject::IsValidValue, object, std::placeholders::_1);
                    return func(pdata);
                }
                break;
                case function_getvalue:
                {
                    const auto func = std::bind(&CObObject::GetValue, object, std::placeholders::_1);
                    return func(pdata);
                }
                break;
                case function_setvalue:
                {
                    const auto func = std::bind(&CObObject::SetValue, object, std::placeholders::_1, std::placeholders::_2);
                    return func(pdata, anyval);
                }
                break;
                default:
                {; }
                }
                return false;
            };
            char* data_ptr = pdata;
            if (data_ptr == nullptr) return default_rr();
            std::any anyvalue;
            for (const auto& it : m_ptuple)
            {
                if (it.ptr == nullptr)
                    return default_rr();
                if (it.ptr->IsReference()) {
                    if (it.ref.tuple_ref == CTupleObject::_tuple_ref::tuple_ref_addr)
                        return call_func(it.ptr, f_type, data_ptr, anyval);
                    anyvalue = it.ptr->GetValue(data_ptr);
                    if (!anyvalue.has_value() || anyvalue.type() != typeid(PVOID))
                        return default_rr();
                    data_ptr = static_cast<char*>(std::any_cast<PVOID>(anyvalue));
                }
                else if (it.ptr->IsStruct()) {
                    if (!data_ptr) return default_rr();
                    std::shared_ptr<CStructObject> struct_ptr = std::static_pointer_cast<CStructObject>(it.ptr);
                    if (it.struc.idx == static_cast<unsigned int>(-1))
                        return default_rr();
                    unsigned int offset = struct_ptr->GetElementOffset(it.struc.idx);
                    if (offset == CStructObject::InvalidOffset)
                        return default_rr();
                    data_ptr = data_ptr + offset;
                }
                else if (it.ptr->IsBaseTy()) {
                    return call_func(it.ptr, f_type, data_ptr, anyval);
                }
                else if (it.ptr->IsStringref()) {
                    return call_func(it.ptr, f_type, data_ptr, anyval);
                }
                else if (it.ptr->IsFlag())
                {
                    std::shared_ptr<CFlagObject> flag_ptr = std::static_pointer_cast<CFlagObject>(it.ptr);
                    if(f_type != function_getvalue || it.flag.tuple_flg == CTupleObject::_tuple_flg::tuple_flg_int)
                        return call_func(it.ptr, f_type, data_ptr, anyval);
                    std::any val = std::get<std::any>(call_func(it.ptr, f_type, data_ptr, anyval));
                    unsigned int value = ConvertAnyType<unsigned int>(val);
                    if (value == static_cast<unsigned int>(-1))
                        return default_rr();
                    return flag_ptr->GetFlagStr(value);
                }
                else if (it.ptr->IsArray()) {
                    if (!data_ptr) return default_rr();
                    std::shared_ptr<CArrayObject> array_ptr = std::static_pointer_cast<CArrayObject>(it.ptr);
                    if (it.arr.dim == static_cast<unsigned int>(-1))
                        return default_rr();
                    unsigned int offset = array_ptr->GetElementOffset(it.arr.dim);
                    if (offset == CStructObject::InvalidOffset)
                        return default_rr();
                    data_ptr = data_ptr + offset;
                }
            }
            return default_rr();
        }

        virtual bool IsValidValue(char* pdata) const override {
            return std::get<bool>(PolyImplFunction(function_isvalidvalue, pdata, std::any()));
        }
        virtual std::any GetCurValue() const override {
            if (GetMetadataDef())
                return GetMetadataDef()->GetCurValue();
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
            return std::get<std::any>(PolyImplFunction(function_getvalue, pdata, std::any()));
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            return false;
        }
        virtual bool SetValue(char* pdata, const std::any& any_value) const override {
            return std::get<bool>(PolyImplFunction(function_setvalue, pdata, any_value));
        }
        virtual bool Success(char* pdata) const override {
            return std::get<bool>(PolyImplFunction(function_success, pdata, std::any()));
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
        iterator        begin() { return m_ptuple.begin(); }
        const_iterator  begin() const { return m_ptuple.begin(); }
        iterator        end() { return m_ptuple.end(); }
        const_iterator  end() const { return m_ptuple.end(); }
    private:
        std::string m_tuplename;
        std::vector<_tuple_elem> m_ptuple;
    };
    // the symbol table class object
    class CTypeSymbolTableObject : public CObObject
    {
    public:
        std::shared_ptr<CObObject> GetTypeSymbolReference(const std::string& symbol_name) const {
            if (!symbol_name.length()) return nullptr;
            auto& it = m_typesymboltable.find(symbol_name);
            if (it == m_typesymboltable.end())
                return nullptr;
            return it->second;
        }
        bool AddTypeSymbol(const std::pair<std::string, std::shared_ptr<CObObject>>& symbol_pair) {
            if (m_typesymboltable.find(symbol_pair.first) == m_typesymboltable.end())
            {
                m_typesymboltable[symbol_pair.first] = symbol_pair.second;
                return true;
            }
            return false;
        }
        size_t GetSymbolTableSize() { return m_typesymboltable.size(); }
        static CTypeSymbolTableObject& GetInstance()
        {
            static CTypeSymbolTableObject m_instance;
            return m_instance;
        }
        virtual const std::string& GetName() const override { return m_symbolname; }
        virtual const std::shared_ptr<CObObject> GetMetadataDef() const override { return nullptr; }
        virtual bool IsValidValue(char* pdata) const override {
            return true;
        }
        virtual std::any GetCurValue() const override {
            return {};
        }
        virtual std::any GetValue(char* pdata) const override {
            return {};
        }
        virtual bool SetCurValue(const std::any& anyvalue) override {
            return true;
        }
        virtual bool SetValue(char* pdata, const std::any& anyvalue) const override {
            return true;
        }
        virtual bool Success(char* pdata) const override {
            return true;
        }
        virtual bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        CTypeSymbolTableObject() :m_symbolname("type_symbol") { SetObType(CObObject::ob_symboltable); SetObSize(0); InitializeTypeSymbolTable(); }
        ~CTypeSymbolTableObject() = default;
        CTypeSymbolTableObject(const CTypeSymbolTableObject&) = delete;
        CTypeSymbolTableObject& operator=(const CTypeSymbolTableObject&) = delete;
        void InitializeTypeSymbolTable();
        std::string m_symbolname;
        TYPE_SYMBOLTABLE m_typesymboltable;
    };

} // namespace cchips