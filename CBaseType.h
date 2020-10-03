#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <cassert>
#include <regex>
#include <any>
#include "CBaseType.h"
#include "MetadataTypeImpl.h"
#include "ExceptionThrow.h"
#include "utils.h"

namespace cchips {

    // overload operator for support special type.
    template<>
    bool CBaseType<LARGE_INTEGER>::Success(char* pdata) const
    {
        if (IsValidValue(pdata))
        {
            LARGE_INTEGER* p = reinterpret_cast<LARGE_INTEGER*>(pdata);
            if (GetOp() == op_n_equal)
            {
                if (p->QuadPart == _value.QuadPart)
                    return false;
                else
                    return true;
            }
            else if (GetOp() == op_greater)
            {
                if (p->QuadPart > _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_greater_e)
            {
                if (p->QuadPart >= _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_less)
            {
                if (p->QuadPart < _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_less_e)
            {
                if (p->QuadPart <= _value.QuadPart)
                    return true;
                else
                    return false;
            }
            if (p->QuadPart == _value.QuadPart)
                return true;
        }
        return false;
    }

    template<>
    bool CBaseType<ULARGE_INTEGER>::Success(char* pdata) const
    {
        if (IsValidValue(pdata))
        {
            ULARGE_INTEGER* p = reinterpret_cast<ULARGE_INTEGER*>(pdata);
            if (GetOp() == op_n_equal)
            {
                if (p->QuadPart == _value.QuadPart)
                    return false;
                else
                    return true;
            }
            else if (GetOp() == op_greater)
            {
                if (p->QuadPart > _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_greater_e)
            {
                if (p->QuadPart >= _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_less)
            {
                if (p->QuadPart < _value.QuadPart)
                    return true;
                else
                    return false;
            }
            else if (GetOp() == op_less_e)
            {
                if (p->QuadPart <= _value.QuadPart)
                    return true;
                else
                    return false;
            }
            if (p->QuadPart == _value.QuadPart)
                return true;
        }
        return false;
    }

    template<>
    bool CBaseType<GUID>::Success(char* pdata) const
    {
        if (IsValidValue(pdata))
        {
            GUID* p = reinterpret_cast<GUID*>(pdata);
            if (GetOp() == op_n_equal)
            {
                if (*p == _value)
                    return false;
                else
                    return true;
            }
            else if (GetOp() == op_greater)
            {
                // GUID only support equal or n_equal
                return false;
            }
            else if (GetOp() == op_greater_e)
            {
                // GUID only support equal or n_equal
                return false;
            }
            else if (GetOp() == op_less)
            {
                // GUID only support equal or n_equal
                return false;
            }
            else if (GetOp() == op_less_e)
            {
                // GUID only support equal or n_equal
                return false;
            }
            if (*p == _value)
                return true;
        }
        return false;
    }

    template<typename T>
    inline bool vir_sstream_copy(char* dst, std::any const& anyvalue)
    {
        DWORD OldProtect = 0;
        if (!anyvalue.has_value() || anyvalue.type() != typeid(T))
            return false;
        if (VirtualProtect((T*)dst, sizeof(T), PAGE_READWRITE, &OldProtect))
        {
            *reinterpret_cast<T*>(dst) = std::any_cast<T>(anyvalue);
            VirtualProtect((T*)dst, sizeof(T), OldProtect, &OldProtect);
            return true;
        }
        return false;
    }

} // namespace cchips