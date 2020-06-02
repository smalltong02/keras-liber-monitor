#include "stdafx.h"
#include "ServicesBase.h"

ServiceBase* ServiceBase::m_service = nullptr;

ServiceBase::ServiceBase(const CString& name,
    const CString& displayName,
    DWORD dwStartType,
    DWORD dwErrCtrlType,
    DWORD dwAcceptedCmds,
    const CString& depends,
    const CString& account,
    const CString& password)
    : m_name(name),
    m_displayName(displayName),
    m_dwStartType(dwStartType),
    m_dwErrorCtrlType(dwErrCtrlType),
    m_depends(depends),
    m_account(account),
    m_password(password),
    m_svcStatusHandle(nullptr) {

    m_svcStatus.dwControlsAccepted = dwAcceptedCmds;
    m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_svcStatus.dwCurrentState = SERVICE_START_PENDING;
    m_svcStatus.dwWin32ExitCode = NO_ERROR;
    m_svcStatus.dwServiceSpecificExitCode = 0;
    m_svcStatus.dwCheckPoint = 0;
    m_svcStatus.dwWaitHint = 0;
}

void ServiceBase::SetStatus(DWORD dwState, DWORD dwErrCode, DWORD dwWait) {
    m_svcStatus.dwCurrentState = dwState;
    m_svcStatus.dwWin32ExitCode = dwErrCode;
    m_svcStatus.dwWaitHint = dwWait;

    ::SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

void ServiceBase::WriteToEventLog(const CString& msg, WORD type) {
    //HANDLE hSource = RegisterEventSource(nullptr, m_name);
    //if (hSource) {
    //    const TCHAR* msgData[2] = { m_name, msg };
    //    ReportEvent(hSource, type, 0, 0, nullptr, 2, 0, msgData, nullptr);
    //    DeregisterEventSource(hSource);
    //}
}

// static
void WINAPI ServiceBase::SvcMain(DWORD argc, TCHAR* argv[]) {
    if (!m_service) return;

    m_service->m_svcStatusHandle = ::RegisterServiceCtrlHandlerEx(m_service->GetName(),
        ServiceCtrlHandler, NULL);
    if (!m_service->m_svcStatusHandle) {
        m_service->WriteToEventLog(_T("Can't set service control handler"),
            EVENTLOG_ERROR_TYPE);
        return;
    }

    m_service->Start(argc, argv);
}

// static
DWORD WINAPI ServiceBase::ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType,
    void* evtData, void* /*context*/) {
    switch (ctrlCode) {
    case SERVICE_CONTROL_STOP:
        m_service->Stop();
        break;

    case SERVICE_CONTROL_PAUSE:
        m_service->Pause();
        break;

    case SERVICE_CONTROL_CONTINUE:
        m_service->Continue();
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        m_service->Shutdown();
        break;

    case SERVICE_CONTROL_SESSIONCHANGE:
        m_service->OnSessionChange(evtType, reinterpret_cast<WTSSESSION_NOTIFICATION*>(evtData));
        break;

    default:
        break;
    }

    return 0;
}

bool ServiceBase::RunInternal(ServiceBase* svc) {
    m_service = svc;

    TCHAR* svcName = const_cast<CString&>(m_service->GetName()).GetBuffer();

    SERVICE_TABLE_ENTRY tableEntry[] = {
      {svcName, SvcMain},
      {nullptr, nullptr}
    };

    return ::StartServiceCtrlDispatcher(tableEntry) == TRUE;
}

void ServiceBase::Start(DWORD argc, TCHAR* argv[]) {
    SetStatus(SERVICE_START_PENDING);
    OnStart(argc, argv);
    SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Stop() {
    SetStatus(SERVICE_STOP_PENDING);
    OnStop();
    SetStatus(SERVICE_STOPPED);
}

void ServiceBase::Pause() {
    SetStatus(SERVICE_PAUSE_PENDING);
    OnPause();
    SetStatus(SERVICE_PAUSED);
}

void ServiceBase::Continue() {
    SetStatus(SERVICE_CONTINUE_PENDING);
    OnContinue();
    SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Shutdown() {
    OnShutdown();
    SetStatus(SERVICE_STOPPED);
}

//static
bool ServiceInstaller::Install(const ServiceBase& service) {
    std::string servicePath;
    servicePath.resize(MAX_PATH);

    GetCurrentDirectory(MAX_PATH, &servicePath[0]);
    if (!servicePath.length()) return false;

    servicePath = std::string(&servicePath[0]) + std::string("\\ServicesTest.exe");

    ServiceHandle svcControlManager = ::OpenSCManager(nullptr, nullptr,
        SC_MANAGER_CONNECT |
        SC_MANAGER_CREATE_SERVICE);
    if (!svcControlManager) {
        return false;
    }

    const CString& depends = service.GetDependencies();
    const CString& acc = service.GetAccount();
    const CString& pass = service.GetPassword();

    ServiceHandle servHandle = ::CreateService(svcControlManager,
        service.GetName(),
        service.GetDisplayName(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        service.GetStartType(),
        service.GetErrorControlType(),
        servicePath.c_str(),
        nullptr,
        nullptr,
        (depends.IsEmpty() ? nullptr : depends.GetString()),
        (acc.IsEmpty() ? nullptr : acc.GetString()),
        (pass.IsEmpty() ? nullptr : pass.GetString()));
    if (!servHandle) {
        return false;
    }

    BOOL bSuccess = StartServiceA(servHandle, 0, NULL);
    if (!bSuccess)
    {
        DWORD error = GetLastError();
        error = 0;
    }

    return true;
}

//static
bool ServiceInstaller::Uninstall(const ServiceBase& service) {
    ServiceHandle svcControlManager = ::OpenSCManager(nullptr, nullptr,
        SC_MANAGER_CONNECT);

    if (!svcControlManager) {
        return false;
    }

    ServiceHandle servHandle = ::OpenService(svcControlManager, service.GetName(),
        SERVICE_QUERY_STATUS |
        SERVICE_STOP |
        DELETE);

    if (!servHandle) {
        return false;
    }

    SERVICE_STATUS servStatus = {};
    if (::ControlService(servHandle, SERVICE_CONTROL_STOP, &servStatus)) {
        while (::QueryServiceStatus(servHandle, &servStatus)) {
            if (servStatus.dwCurrentState != SERVICE_STOP_PENDING) {
                break;
            }
        }
    }

    if (!::DeleteService(servHandle)) {
        return false;
    }

    return true;
}