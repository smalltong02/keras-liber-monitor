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
#include "ExceptionThrow.h"
#include "utils.h"

namespace cchips {

    class CStandardTypeObject;
    class CArrayObject;
    class CFlagsObject;
    class CBaseDef;

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

        CObObject() : m_ob_type(ob_invalid) {}
        virtual ~CObObject() = default;
        _ob_type GetObType() const { return m_ob_type; }
        size_t GetObSize() const { return m_ob_size; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual const bool IsValidValue(char* pdata) const = 0;
        virtual std::stringstream GetValue(char* pdata) const = 0;
        virtual bool SetValue(char* pdata, const std::stringstream& ss) = 0;
        virtual bool Success(char* pdata) const = 0;
        virtual bool Failed(char* pdata) const = 0;
        bool IsReference() const {
            if (m_ob_type == ob_reference || m_ob_type == ob_stringref)
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
    private:
        size_t m_ob_size;
        _ob_type m_ob_type;
    };

    using TYPE_SYMBOLTABLE = std::map<std::string, std::shared_ptr<CObObject>>;
    using IDENPAIR = std::pair<std::string, std::shared_ptr<CObObject>>;

    // the opration of base define
    template <typename T>
    class CTraits
    {
    public:
        enum _c_operator
        {
            op_invalid = 0,
            op_equal,
            op_n_equal,
            op_greater,
            op_less,
            op_max,
        };

        CTraits() : _value(0), _op(op_n_equal) {}
        CTraits(const T value, int op = op_n_equal) : _value(value), _op(op) {}
        CTraits(const CTraits& judge) : _value(judge._value), _op(judge._op) {}
        ~CTraits() = default;

        bool UpdateOp(int op) {
            ASSERT(op != op_invalid && op < op_max);
            if (op != op_invalid && op < op_max)
            {
                _op = op;
                return true;
            }
            return false;
        }
        void UpdateValue(const T& value) {
            _value = value;
        }
        bool UpdateValue(const std::stringstream& ss) {
            std::stringstream tmp_ss;
            tmp_ss << ss.str();
            tmp_ss >> _value;
            return true;
        }

        bool IsValidValue(char* pdata) const
        {
            ASSERT(pdata != nullptr);
            T* p = reinterpret_cast<T*>(pdata);
            if (!p) return false;
            //if (*p == _value)
            //	return false;
            return true;
        }
        std::stringstream GetValue() const
        {
            std::stringstream ss;
            ss << _value;
            return ss;
        }
        std::stringstream GetValue(char* pdata) const
        {
            std::stringstream ss;
            if (IsValidValue(pdata))
            {
                T* p = reinterpret_cast<T*>(pdata);
                ss << *p;
            }
            return ss;
        }

        bool SetValue(char* pdata, const std::stringstream& ss);

        bool Success(char* pdata) const
        {
            if (IsValidValue(pdata))
            {
                T* p = reinterpret_cast<T*>(pdata);
                if (_op == op_n_equal)
                {
                    if (*p == _value)
                        return false;
                    else
                        return true;
                }
                else if (_op == op_greater)
                {
                    if (*p >= _value)
                        return true;
                    else
                        return false;
                }
                else if (_op == op_less)
                {
                    if (*p <= _value)
                        return true;
                    else
                        return false;
                }
                if (*p == _value)
                    return true;
            }
            return false;
        }
        bool Failed(char* pdata) const { return !Success(pdata); }
    private:
        int _op;
        T _value;
    };

    // the base class of define, the based on C++ standrad type define.
    class CBaseDef
    {
    public:
        enum _standard_type
        {
            type_invalid = 0, type_void, type_byte, type_bool, type_char, type_uchar, type_int16, type_uint16, type_short,
            type_ushort, type_word, type_wchar, type_int, type_uint, type_long, type_long_ptr, type_ulong, type_ulong_ptr,
            type_dword, type_dword_ptr, type_float, type_int64, type_uint64, type_longlong, type_ulonglong, type_qword,
            type_double, type_large_integer, type_ularge_integer, type_lpvoid, type_hmodule, type_hresult, type_schandle,
            type_ntstatus, type_string, type_wstring, type_handle, type_guid, type_max,
        };

        CBaseDef() : _base_def(type_invalid), _base_size(0) {}
        virtual ~CBaseDef() = default;
        int GetBaseDef() const { return _base_def; }
        size_t GetBaseSize() const { return _base_size; }
        const static std::string& GetBaseStr(int def) {
            ASSERT(def != type_invalid && def < type_max);
            if (def != type_invalid && def < type_max)
            {
                auto& p = _base_to_str_def.find(def);
                if (p != _base_to_str_def.end()) { return (*p).second; }
            }
            return _base_to_str_def.begin()->second;
        }
        virtual std::shared_ptr<CBaseDef> Clone() const = 0;
        virtual bool FreshTraits(std::unique_ptr<void>& value, int op) = 0;
        virtual bool IsValidValue(char* pdata) const = 0;
        virtual std::stringstream GetValue() const = 0;
        virtual std::stringstream GetValue(char* pdata) const = 0;
        virtual bool SetValue(char* pdata, const std::stringstream& ss) = 0;
        virtual bool SetValue(const std::stringstream& ss) = 0;
        virtual bool Success(char* pdata) const = 0;
        virtual bool Failed(char* pdata) const = 0;
    protected:
        bool SetBaseDef(int def) {
            ASSERT(def != type_invalid && def < type_max);
            if (def != type_invalid && def < type_max)
            {
                auto& p = _base_to_size_def.find(def);
                if (p != _base_to_size_def.end()) { _base_size = (*p).second; }
                _base_def = def;
                return true;
            }
            return false;
        }
    private:
        // the list of a pair for base define and size.
        static const std::map<int, size_t> _base_to_size_def;
        // the list of a pair for base define and string name.
        static const std::map<int, std::string> _base_to_str_def;
        size_t _base_size;
        int _base_def;
    };

    // the type define of cchips, contains type information and opration.
    template <typename T>
    class CBaseType : public CBaseDef
    {
    public:
        CBaseType(int base_def, const CTraits<T>& judge) : _judge(judge) { SetBaseDef(base_def); }
        CBaseType(const CBaseType<T>& ctype) : _judge(ctype._judge) { SetBaseDef(ctype.GetBaseDef()); }
        ~CBaseType() = default;

        CTraits<T>& GetTraits() {
            return _judge;
        }
        virtual std::shared_ptr<CBaseDef> Clone() const override {
            std::shared_ptr<CBaseDef> p = std::make_shared<CBaseType<T>>(*this);
            if (p) return p;
            return nullptr;
        }
        virtual bool FreshTraits(std::unique_ptr<void>& value, int op) override {
            std::unique_ptr<T> v(reinterpret_cast<T*>(value.get()));
            if (!_judge.UpdateOp(op)) return false;
            _judge.UpdateValue(*v);
            return true;
        };
        virtual bool IsValidValue(char* pdata) const override {
            return _judge.IsValidValue(pdata);
        }
        std::stringstream GetValue() const override {
            return _judge.GetValue();
        }
        std::stringstream GetValue(char* pdata) const override {
            return _judge.GetValue(pdata);
        }
        virtual bool SetValue(char* pdata, const std::stringstream& ss) override {
            return _judge.SetValue(pdata, ss);
        }
        virtual bool SetValue(const std::stringstream& ss) override {
            return _judge.UpdateValue(ss);
            return true;
        }
        bool Success(char* pdata) const override {
            return _judge.Success(pdata);
        }
        bool Failed(char* pdata) const override {
            return _judge.Failed(pdata);
        }
    private:
        CTraits<T> _judge;
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
        CMetadataTypeObject(const std::string& name, const std::shared_ptr<CBaseDef>& pbasedef) : metadata_name(name), m_pmetadatadef(pbasedef) {
            SetObType(CObObject::ob_basetype); SetObSize(m_pmetadatadef->GetBaseSize());
        }
        CMetadataTypeObject(const std::string& name, const std::shared_ptr<CMetadataTypeObject>& pmetadata) : metadata_name(name) {
            SetObType(CObObject::ob_basetype); m_pmetadatadef = pmetadata->GetMetadataDef(); SetObSize(m_pmetadatadef->GetBaseSize());
        }
        ~CMetadataTypeObject() = default;

        virtual const std::string& GetName() const override { return metadata_name; }
        const std::shared_ptr<CObObject> GetData() { return shared_from_this(); }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return m_pmetadatadef; }
        virtual const bool IsValidValue(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->IsValidValue(pdata);
            return false;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (GetMetadataDef())
                return GetMetadataDef()->GetValue(pdata);
            return ss;
        }
        bool SetValue(const std::stringstream& ss) {
            if (GetMetadataDef())
                return GetMetadataDef()->SetValue(ss);
            return false;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            if (GetMetadataDef())
                return GetMetadataDef()->SetValue(pdata, ss);
            return false;
        }
        bool Success(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->Success(pdata);
            return false;
        }
        bool Failed(char* pdata) const override {
            if (GetMetadataDef())
                return GetMetadataDef()->Success(pdata);
            return false;
        }

        // the function for update type define
        bool FreshTraits(std::unique_ptr<void>& value, int op);
    private:
        std::string metadata_name;
        std::shared_ptr<CBaseDef> m_pmetadatadef;
    };
    // initialize and make the metadata type class object and return the object reference.
    template <typename _Ty>
    inline std::shared_ptr<CMetadataTypeObject> make_metadata_s_ptr(int def) noexcept
    {
        CBaseType<_Ty>* base_ptr = new CBaseType<_Ty>(def, CTraits<_Ty>());
        if (!base_ptr) return nullptr;
        std::shared_ptr<CBaseType<_Ty>> ptr(base_ptr, CDeleter<CBaseType<_Ty>>());
        return std::make_shared<CMetadataTypeObject>(CBaseDef::GetBaseStr(def), std::reinterpret_pointer_cast<CBaseDef>(ptr));
    }
    // initialize and make the metadata type class object and return the object reference.
    template <typename _Ty>
    inline std::shared_ptr<CMetadataTypeObject> make_metadata_j_ptr(int def, CTraits<_Ty>& judge) noexcept
    {
        CBaseType<_Ty>* base_ptr = new CBaseType<_Ty>(def, judge);
        if (!base_ptr) return nullptr;
        std::shared_ptr<CBaseType<_Ty>> ptr(base_ptr, CDeleter<CBaseType<_Ty>>());
        return std::make_shared<CMetadataTypeObject>(CBaseDef::GetBaseStr(def), std::reinterpret_pointer_cast<CBaseDef>(ptr));
    }
    // the reference class object, It is a reference of other data object.
    class CReferenceObject : public CObObject
    {
    public:
        CReferenceObject() { SetObType(CObObject::ob_reference); SetObSize(sizeof(PVOID)); }
        CReferenceObject(const std::string name) : m_refername(name) { SetObType(CObObject::ob_reference); SetObSize(sizeof(PVOID)); }
        CReferenceObject(const std::string name, std::shared_ptr<CObObject>& pelement) : m_refername(name), m_pelement(pelement) {
            ResetOb();
        }
        ~CReferenceObject() = default;
        virtual bool AddReference(std::shared_ptr<CObObject>& pelement) {
            if (!m_pelement)
            {
                m_pelement = pelement;
                ResetOb();
                return true;
            }
            return false;
        }
        virtual const std::string& GetName() const override { return m_refername; }
        const std::shared_ptr<CObObject> GetData() const { return m_pelement; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return (*m_pelement).GetMetadataDef(); }
        virtual const bool IsValidValue(char* pdata) const override {
            ASSERT(pdata != nullptr);
            if (reinterpret_cast<const PVOID*>(pdata) == nullptr)
                return false;
            // the lpModuleName parameter could be NULL in API GetModuleHandle().
    //		ASSERT(*reinterpret_cast<const PVOID*>(pdata) != nullptr);
            //if (*reinterpret_cast<const PVOID*>(pdata) == nullptr)
            //	return false;
            return true;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (!IsValidValue(pdata)) return ss;
            if (*reinterpret_cast<wchar_t**>(pdata) == nullptr)
            {
                ss << "nullptr";
                return ss;
            }
            if (GetObType() == ob_stringref)
            {
                if (m_pelement->GetMetadataDef()->GetBaseDef() == CBaseDef::type_wchar)
                    ss << W2AString(std::wstring(*reinterpret_cast<wchar_t**>(pdata))).c_str();
                else
                    ss << *reinterpret_cast<const char**>(pdata);
            }
            else
                ss << std::hex << "0x" << *reinterpret_cast<const PVOID*>(pdata);
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override;
        bool Success(char* pdata) const override {
            std::stringstream ss;
            ss = GetValue(pdata);
            if (ss.str().length() == 0 || _stricmp(ss.str().c_str(), "nullptr") == 0)
                return false;
            return true;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        void ResetOb() {
            if (m_pelement->IsBaseTy() && IsStringType(m_pelement->GetMetadataDef()->GetBaseDef()))
            {
                // the string is a special reference
                SetObType(CObObject::ob_stringref); SetObSize(sizeof(PVOID));
            }
            else
            {
                SetObType(CObObject::ob_reference); SetObSize(sizeof(PVOID));
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
        CArrayObject(const std::string& name) : m_arrayname(name), m_dim(0) { SetObType(CObObject::ob_array); SetObSize(0); }
        CArrayObject(const std::string& name, const std::shared_ptr<CObObject>& pelement, int dim) : m_arrayname(name), m_pelement(pelement), m_dim(dim) { SetObType(CObObject::ob_array); SetObSize(m_pelement->GetObSize()*dim); }
        ~CArrayObject() = default;
        bool AddArray(const std::shared_ptr<CObObject>& pelement, int dim) {
            if (!m_pelement)
            {
                m_dim = dim;
                m_pelement = pelement;
                SetObSize(m_pelement->GetObSize()*dim);
                return true;
            }
            return false;
        }
        int GetDim() const { return m_dim; }
        virtual const std::string& GetName() const override { return m_arrayname; }
        const std::shared_ptr<CObObject> GetData() const { return m_pelement; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return (*m_pelement).GetMetadataDef(); }
        virtual const bool IsValidValue(char* pdata) const override {
            ASSERT(pdata != nullptr);
            if (pdata == nullptr) return false;
            return true;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (!IsValidValue(pdata)) return ss;
            if (!m_pelement) return ss;
            char * p = (char *)pdata;
            for (int i = 0; i < m_dim; i++) {
                ss << m_pelement->GetValue(p).str();
                p += m_pelement->GetObSize();
            }
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            if (!IsValidValue(pdata)) return false;
            if (!m_pelement) return false;
            char * p = (char *)pdata;
            for (int i = 0; i < m_dim; i++) {
                if (m_pelement->SetValue(p, ss))
                    return false;
                p += m_pelement->GetObSize();
            }
            return true;
        }
        bool Success(char* pdata) const override {
            return true;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        int m_dim;
        std::string m_arrayname;
        std::shared_ptr<CObObject> m_pelement;
    };
    // the tuple class object, it could be contains variable length and variable type of data.
    class CTupleObject : public CObObject
    {
    public:
        CTupleObject() = delete;
        CTupleObject(const std::string& name) : m_tuplename(name) { SetObType(CObObject::ob_tuple); SetObSize(0); }
        CTupleObject(const std::string& name, const std::vector<std::shared_ptr<CObObject>>& parray) : m_tuplename(name) {
            SetObType(CObObject::ob_tuple);
            AddArray(parray);
        }
        ~CTupleObject() = default;
        bool AddArray(const std::vector<std::shared_ptr<CObObject>>& parray)
        {
            size_t size = 0;
            for (auto& it : m_parray)
            {
                if (!it) return false;
                size = it->GetObSize();
            }
            m_parray = parray;
            SetObSize(size);
            return true;
        }
        void AddElement(const std::shared_ptr<CObObject>& pelement)
        {
            m_parray.push_back(pelement);
            SetObSize(GetObSize() + pelement->GetObSize());
        }
        virtual const std::string& GetName() const override { return m_tuplename; }
        const std::vector<std::shared_ptr<CObObject>>& GetData() const { return m_parray; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return nullptr; }
        virtual const bool IsValidValue(char* pdata) const override {
            ASSERT(pdata != nullptr);
            if (pdata == nullptr) return false;
            return true;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (!IsValidValue(pdata)) return ss;
            if (m_parray.size() == 0) return ss;
            char * p = pdata;

            for (const auto& elem : m_parray) {
                if (!ss.str().length())
                    ss << "{ ";
                else
                    ss << "; ";
                ss << elem->GetName() << ": " << elem->GetValue(p).str();
                p += elem->GetObSize();
            }
            if (ss.str().length())
                ss << " }";
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            return false;
        }
        bool Success(char* pdata) const override {
            return true;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        std::string m_tuplename;
        std::vector<std::shared_ptr<CObObject>> m_parray;
    };
    // the struct class object
    class CStructObject : public CObObject
    {
    public:
        using ElementTypeDefine = std::pair<std::string, std::pair<int, std::shared_ptr<CObObject>>>;
        using ElementListDefine = std::map<std::string, ElementTypeDefine>;
        using iterator = ElementListDefine::iterator;
        using const_iterator = ElementListDefine::const_iterator;

        CStructObject() { SetObType(CObObject::ob_struct); SetObSize(0); }
        CStructObject(const std::string& name) : m_structname(name) { SetObType(CObObject::ob_struct); SetObSize(0); }
        CStructObject(const std::string& name, const ElementListDefine& pstruct) : m_structname(name) {
            SetObType(CObObject::ob_struct);
            AddStruct(pstruct);
        }
        ~CStructObject() = default;
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
        bool AddElement(const std::string name, const ElementTypeDefine& pelement) {
            ASSERT(name.length());
            if (!name.length()) return false;
            auto& it = m_pstruct.find(name);
            if (it != m_pstruct.end() && it->second.second.second != nullptr) return false;
            m_pstruct[name] = pelement;
            m_pstruct[name].second.first = (int)GetObSize();
            if (pelement.second.second != nullptr)
            {
                SetObSize(GetObSize() + pelement.second.second->GetObSize());
                return true;
            }
            return false;
        }
        virtual const std::string& GetName() const override { return m_structname; }
        const ElementListDefine& GetData() const { return m_pstruct; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return nullptr; }
        virtual const bool IsValidValue(char* pdata) const override {
            ASSERT(pdata != nullptr);
            if (pdata == nullptr) return false;
            return true;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (!IsValidValue(pdata)) return ss;
            if (m_pstruct.size() == 0) return ss;
            char * p = (char *)pdata;
            for (const auto& elem : *this) {
                if (!elem.second.second.second)
                {
                    ss.str(""); return ss;
                }
                if (!ss.str().length())
                    ss << "{ ";
                else
                    ss << "; ";
                ss << elem.first << ": " << elem.second.second.second->GetValue(p).str();
                p += elem.second.second.second->GetObSize();
            }
            if (ss.str().length())
                ss << " }";
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            return false;
        }
        bool Success(char* pdata) const override {
            return true;
        }
        bool Failed(char* pdata) const override {
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
        int GetElementOffset(const std::string& name) const {
            if (!name.length()) return InvalidOffset;
            const auto p = m_pstruct.find(name);
            if (p == m_pstruct.end()) return InvalidOffset;
            return p->second.second.first;
        }
        static const int InvalidOffset = -1;
        iterator		begin() { return m_pstruct.begin(); }
        const_iterator	begin() const { return m_pstruct.begin(); }
        iterator		end() { return m_pstruct.end(); }
        const_iterator	end() const { return m_pstruct.end(); }
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
        CFlagObject(const std::string& name) : m_flagname(name), m_operation(flag_invalid) { SetObType(CObObject::ob_flag); SetObSize(0); }
        ~CFlagObject() = default;

        int GetFlagOp(const std::string& op_name) const
        {
            if (_stricmp(op_name.c_str(), "OR") == 0)
                return flag_or;
            else if (_stricmp(op_name.c_str(), "AND") == 0)
                return flag_and;
            else if (_stricmp(op_name.c_str(), "NOT") == 0)
                return flag_not;
            return flag_invalid;
        }
        bool AddFlags(const std::shared_ptr<CObObject>& pdata, int op, std::unique_ptr<std::map<std::string, int>>& values) {
            if (!pdata || (*values).size() == 0) return false;
            if (op == flag_invalid || op > flag_not) return false;
            ASSERT(pdata->IsBaseTy()); // m_pdata must be ob_basetype;
            m_operation = (_flag_operation)op;
            m_pflagvalues = std::move(values);
            m_pdata = pdata;
            SetObSize(m_pdata->GetObSize());
            return true;
        }
        virtual const std::string& GetName() const override { return m_flagname; }
        const std::shared_ptr<CObObject> GetData() const { return m_pdata; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return (*m_pdata).GetMetadataDef(); }
        int GetFlagVa(const std::string& value_str) const {
            if (!value_str.length()) return InvalidFlagVa;
            if (!m_pflagvalues) return InvalidFlagVa;
            auto& it = (*m_pflagvalues).find(value_str);
            if (it == (*m_pflagvalues).end()) return InvalidFlagVa;
            return it->second;
        }
        const std::string GetFlagStr(int flag_value) const {
            if (!m_pflagvalues) return std::string({});
            if (flag_value == InvalidFlagVa) return std::string({});
            for (const auto& flag : *m_pflagvalues)
            {
                if (flag.second == flag_value)
                    return flag.first;
            }
            std::string ret_string = std::string("unknown-flags:") + std::to_string(flag_value);
            return ret_string;
        }
        virtual const bool IsValidValue(char* pdata) const override {
            if (!m_pdata) return false;
            return m_pdata->IsValidValue(pdata);
            return false;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            if (m_pdata)
            {
                ss = m_pdata->GetValue(pdata);
                if (!ss.str().length()) return ss;
                char* nodig = nullptr;
                int flag_value = std::strtol(ss.str().c_str(), &nodig, 10);
                ss.str("");
                ss << GetFlagStr(flag_value).c_str();
            }
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            if (m_pdata)
            {
                if (!ss.str().length()) return false;
                int flag_value = GetFlagVa(ss.str());
                if (flag_value == InvalidFlagVa) return false;
                std::stringstream tmp_ss;
                tmp_ss << flag_value;
                return m_pdata->SetValue(pdata, ss);
            }
            return false;
        }
        bool Success(char* pdata) const override {
            return true;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
        static const int InvalidFlagVa = -1;
    private:
        std::string m_flagname;
        std::shared_ptr<CObObject> m_pdata;
        std::unique_ptr<std::map<std::string, int>> m_pflagvalues;
        _flag_operation m_operation;
    };
    // the symbol table class object
    class CTypeSymbolTableObject : public CObObject
    {
    public:
        const std::shared_ptr<CObObject> GetTypeSymbolRefrence(const std::string& symbol_name) const {
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
        static std::unique_ptr<CTypeSymbolTableObject> GetInstance()
        {
            if (m_reference_count == 0)
            {
                CTypeSymbolTableObject* p = new CTypeSymbolTableObject();
                if (p)
                {
                    m_reference_count++;
                    return std::unique_ptr<CTypeSymbolTableObject>(p);
                }
            }
            return nullptr;
        }
        void InitializeTypeSymbolTable();
        virtual const std::string& GetName() const override { return m_symbolname; }
        virtual const std::shared_ptr<CBaseDef> GetMetadataDef() const override { return nullptr; }
        virtual const bool IsValidValue(char* pdata) const override {
            return false;
        }
        std::stringstream GetValue(char* pdata) const override {
            std::stringstream ss;
            return ss;
        }
        bool SetValue(char* pdata, const std::stringstream& ss) override {
            return true;
        }
        bool Success(char* pdata) const override {
            return true;
        }
        bool Failed(char* pdata) const override {
            return !Success(pdata);
        }
    private:
        CTypeSymbolTableObject() :m_symbolname("type_symbol") { SetObType(CObObject::ob_symboltable); SetObSize(0); InitializeTypeSymbolTable(); }

        std::string m_symbolname;
        TYPE_SYMBOLTABLE m_typesymboltable;
        static int m_reference_count;
    };

} // namespace cchips