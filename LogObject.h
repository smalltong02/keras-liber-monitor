#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <random>
#include "SpecialLog.h"
#include "SafePtr.h"

namespace cchips {

using LOGPAIR = std::pair<std::string, std::string>;
class CLogEntry;
class CLogObject;
extern std::unique_ptr<CLogObject> g_log_object;

class CLogObject
{
public:
	using logtype = enum _logtype {
		log_invalid = -1,
		log_event = 0,
		log_debug,
		log_error,
	};

	~CLogObject() = default;

	bool Initialize();

	bool AddLog(std::shared_ptr<CLogEntry>& log) { if (!log) return false; m_cache_logs->push_back(log); return true; }

	static std::unique_ptr<CLogObject> GetInstance()
	{
		if (m_reference_count == 0)
		{
			CLogObject* p = new CLogObject();
			if (p)
			{
				m_reference_count++;
				return std::unique_ptr<CLogObject>(p);
			}
		}
		return nullptr;
	}

private:
	CLogObject() = default;
	static int m_reference_count;
	sf::safe_ptr<std::vector<std::shared_ptr<CLogEntry>>> m_cache_logs;
};

class CLogEntry
{
public:
	using iterator = std::map<std::string, std::string>::iterator;
	using const_iterator = std::map<std::string, std::string>::const_iterator;

	CLogEntry(const std::string& name, CLogObject::logtype type = CLogObject::logtype::log_invalid) : log_name(name), m_log_type(type) {}
	~CLogEntry() = default;
	const std::string& GetName() const { return log_name; }
	const CLogObject::logtype GetLogType() const { return m_log_type; }
	const int GetLogSize() const { return (int)log_elements.size(); }
	bool AddLog(const LOGPAIR& log_pair) {
		assert(log_pair.first.length());
		assert(log_pair.second.length());
		if (!log_pair.first.length()) return false;
		if (!log_pair.second.length()) return false;
		const auto& it = log_elements.find(log_pair.first);
		assert(it == log_elements.end());
		if (it != log_elements.end()) return false;
		log_elements[log_pair.first] = log_pair.second;
		return true;
	}
	bool AddLog(const std::shared_ptr<CLogEntry>& log_entry) {
		assert(log_entry);
		if (!log_entry) return false;
		std::stringstream ss;
		for (const auto& entry : *log_entry)
		{
			if (!ss.str().length())
				ss << "{ ";
			else
				ss << "; ";
			ss << entry.first << ": " << entry.second;
		}
		if (ss.str().length())
			ss << " }";
		if (ss.str().length())
			return AddLog(LOGPAIR((*log_entry).GetName(), ss.str()));
		return false;
	}

	iterator		begin() { return log_elements.begin(); }
	const_iterator	begin() const { return log_elements.begin(); }
	iterator		end() { return log_elements.end(); }
	const_iterator	end() const { return log_elements.end(); }
private:
	std::string log_name;
	CLogObject::logtype m_log_type;
	std::map<std::string, std::string> log_elements;
};

class CLogHandle
{
public:
	CLogHandle(std::shared_ptr<PVOID> handle) {
		assert(handle != nullptr);
		if (!handle) return;
		m_handle = std::reinterpret_pointer_cast<CLogEntry>(handle);
		m_valid = true;
	}
	CLogHandle(const std::string& name, CLogObject::logtype type = CLogObject::logtype::log_invalid) {
		m_handle = std::make_shared<CLogEntry>(name, type);
		assert(m_handle != nullptr);
		m_valid = true;
	}
	~CLogHandle() {
		if (!m_handle || !m_handle->GetLogSize()) return;
		if (m_handle.use_count() == 1 && m_handle->GetLogType() != CLogObject::logtype::log_invalid)
		{
			if (g_log_object != nullptr)
			{
				g_log_object->AddLog(m_handle);
			}
		}
	}
	std::string GetName() const { 
		if (m_valid)
		{
			assert(m_handle != nullptr);
			return m_handle->GetName();
		}
		return std::string("");
	}
	bool AddLog(const LOGPAIR& log_pair) {
		if (m_valid)
		{
			assert(m_handle != nullptr);
			return m_handle->AddLog(log_pair);
		}
		return false;
	}
	bool AddLog(const std::unique_ptr<CLogHandle>& handle) {
		assert(handle != nullptr);
		if (!handle) return false;
		if (m_valid)
		{
			assert(m_handle != nullptr);
			return m_handle->AddLog(std::reinterpret_pointer_cast<CLogEntry>(handle->GetHandle()));
		}
		return false;
	}
	int LogCounts() const {
		assert(m_handle != nullptr);
		if (!m_handle) return false;
		return m_handle->GetLogSize();
	}
	std::shared_ptr<PVOID> GetHandle() const { 
		if(m_valid)
			return std::reinterpret_pointer_cast<PVOID>(m_handle); 
		return nullptr;
	}
	bool FreeHandle() {
		m_valid = false;
		return true;
	}
private:
	bool m_valid;
	std::shared_ptr<CLogEntry> m_handle;
};

#define BEGIN_LOG(NAME) \
	std::shared_ptr<CLogHandle> local_logger = std::make_shared<CLogHandle>(NAME);
#define LOGGING(KEY, VALUE) \
	local_logger->AddLog(LOGPAIR(KEY, VALUE));
#define END_LOG(LOGGER) \
	if (local_logger->LogCounts()) \
		std::reinterpret_pointer_cast<CLogEntry>(LOGGER)->AddLog(std::reinterpret_pointer_cast<CLogEntry>(local_logger->GetHandle()));
#define LOGGER local_logger

#ifdef _OUTPUT_CONSOLE_STREAM
#define error_log(format,...) SAMPLE_LOG_STREAM(std::cerr,format, ##__VA_ARGS__)
#ifdef _DEBUG 
#define debug_log(format,...) SAMPLE_LOG_STREAM(std::cout,format, ##__VA_ARGS__)
#else
#define debug_log()
#endif
#else
template<typename ...Args>
void error_log(const char* format, Args...args)
{
	std::stringstream os;
	SAMPLE_LOG_STREAM(os, format, args...);
	assert(os.str().length());
	if (!os.str().length()) return;
	std::unique_ptr<CLogHandle> error_handle = std::make_unique<CLogHandle>("error_log", CLogObject::logtype::log_event);
	if(error_handle) (*error_handle).AddLog(LOGPAIR("error", os.str()));
	return;
}
#ifdef _DEBUG 
template<typename ...Args>
void debug_log(const char* format, Args...args)
{
	std::stringstream os;
	SAMPLE_LOG_STREAM(os, format, args...);
	assert(os.str().length());
	if (!os.str().length()) return;
	std::unique_ptr<CLogHandle> debug_handle = std::make_unique<CLogHandle>("debug_log", CLogObject::logtype::log_debug);
	if(debug_handle) (*debug_handle).AddLog(LOGPAIR("debug", os.str()));
	return;
}
#else
#define debug_log()
#endif
#endif

} // namespace cchips