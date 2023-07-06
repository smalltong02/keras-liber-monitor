#pragma once
#include <string>
#include <chrono>
#include "PeFile.h"
#include "PeFormat.h"
#include "stringutils.h"
#include "..\PackageWrapper.h"
#include "..\LogObject.h"

namespace fs = std::filesystem;

namespace cchips {
    // *This mixin can be used if you want your class to have reference object
    // * semantics.

    class NonCopyable {
    public:
        // Disable copy constructor and assignment operator to prevent copying.
        // They are declared public to make diagnostics messages more precise.
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;

    protected:
        // Public constructor is not needed in a mixin, so prevent the
        // compiler from generating a public one.
        NonCopyable() = default;

        // Protected non-virtual destructor disables polymorphic destruction, which
        // is the appropriate behavior in this case.
        ~NonCopyable() = default;
    };

    class CAbstractHandler
    {
    public:
        virtual ~CAbstractHandler() = default;
        virtual bool Initialize() = 0;

        virtual bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) = 0;
        virtual bool ScanPeriod(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) {
            const auto sta = std::chrono::steady_clock::now();
            bool bret = Scan(pe_format, json_result);
            m_scan_period = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sta);
            return bret;
        }
        virtual std::chrono::microseconds GetScanPeriod() const { return m_scan_period; }
    private:
        std::chrono::microseconds m_scan_period{};
    };

    class CPeInsideHandler : public CAbstractHandler
    {

    };
} // namespace cchips
