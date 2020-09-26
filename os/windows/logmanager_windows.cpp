/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cctype>
#include <ctime>
#include <sstream>
#include <vector>
#include "logmanager_windows.h"
#include "mainwindow.h"

const char SVCNAME[] = "Stubby";

static void winerr(const char* operation, DWORD err)
{
    char msg[512];

    if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       err,
                       0,
                       msg,
                       sizeof(msg),
                       NULL) == 0 )
        qCritical("Error: %s: errno=%d\n", operation, err);
    else
        qCritical("Error: %s: %s\n", operation, msg);
}

static void winlasterr(const char* operation)
{
    winerr(operation, GetLastError());
}

LogMgrWindows::LogMgrWindows(MainWindow *parent) :
    QObject(parent),
    m_mainWindow(parent),
    m_log(NULL),
    m_eventResources(NULL),
    m_logTimer(NULL)
{

    m_log = OpenEventLog(NULL, SVCNAME);
    if ( !m_log )
        winlasterr("Open event log");

    char subkey[128];
    char fname[MAX_PATH];
    LSTATUS status;
    DWORD datasize;

    snprintf(subkey, sizeof(subkey),
             "SYSTEM\\CurrentControlSet\\Services"
             "\\EventLog\\Application\\%s", SVCNAME);
    status = RegGetValue(HKEY_LOCAL_MACHINE, subkey, "EventMessageFile",
                         RRF_RT_REG_SZ, NULL, fname, &datasize);
    if ( status == ERROR_SUCCESS )
        m_eventResources = LoadLibraryEx(fname, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE);
    if ( m_eventResources == NULL )
        winlasterr("Load resources module");
    // Read and throw away previous log entries.
    readServiceLog();

    m_logTimer = new QTimer(this);
    connect(m_logTimer, &QTimer::timeout, this, QOverload<>::of(&LogMgrWindows::logTimerExpired));
}

LogMgrWindows::~LogMgrWindows()
{
    if ( m_eventResources )
        FreeLibrary(m_eventResources);
    if ( m_log )
        CloseEventLog(m_log);
    if (m_logTimer) {
        m_logTimer->stop();
        delete m_logTimer;
    }
}

void LogMgrWindows::logTimerExpired() {
    QString log = (readServiceLog());
    if (!log.isEmpty())
        m_mainWindow->logMsg(log);
}

QString LogMgrWindows::readServiceLog()
{
    QString res;

    if ( !m_log )
        return res;

    std::vector<char> evtBuf(sizeof(EVENTLOGRECORD) * 10);

    for(;;)
    {
        DWORD bytesRead;
        DWORD minBytesNeeded;

        if ( ReadEventLog(m_log, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0,
                          evtBuf.data(), static_cast<DWORD>(evtBuf.capacity()),
                          &bytesRead, &minBytesNeeded) == 0 )
        {
            DWORD err = GetLastError();
            if ( err == ERROR_HANDLE_EOF )
                break;
            if ( err != ERROR_INSUFFICIENT_BUFFER )
            {
                winerr("Read event log", err);
                break;
            }
            evtBuf.resize(minBytesNeeded);
            if ( ReadEventLog(m_log, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0,
                              evtBuf.data(), static_cast<DWORD>(evtBuf.capacity()),
                              &bytesRead, &minBytesNeeded) == 0 )
            {
                winerr("Read event log", err);
                break;
            }
        }

        char* pStart = evtBuf.data();
        char* pEnd = pStart + bytesRead;

        while ( pStart < pEnd )
        {
            EVENTLOGRECORD* evt = reinterpret_cast<EVENTLOGRECORD*>(pStart);
            pStart += evt->Length;

            if ( logMessageSource(evt) == SVCNAME ) {
                res.append(formatLogMessage(evt));
            }
        }
    }
    return res;
}

QString LogMgrWindows::logMessageSource(PEVENTLOGRECORD pevt)
{
    // The source name follows the EVENTLOGRECORD structure in memory.
    const char* p = reinterpret_cast<const char*>(pevt) + sizeof(EVENTLOGRECORD);
    return QString::fromUtf8(p);
}

QString LogMgrWindows::formatLogMessage(PEVENTLOGRECORD pevt)
{
    std::time_t t = static_cast<std::time_t>(pevt->TimeGenerated);
    std::tm tm;
    char msg[512];
    const char* etype;
    std::ostringstream out;

    switch (pevt->EventType)
    {
    case EVENTLOG_ERROR_TYPE:
        etype = "ERROR";
        break;

    case EVENTLOG_WARNING_TYPE:
        etype = "WARN";
        break;

    default:
        etype = "INFO";
        break;
    }

    localtime_s(&tm, &t);
    std::strftime(msg, sizeof(msg), "%F %T %z", &tm);
    out << msg << ' ' << etype << ' ';

    std::vector<char *> args(pevt->NumStrings);
    char* p = reinterpret_cast<char*>(pevt) + pevt->StringOffset;
    for ( int i = 0; i < pevt->NumStrings; ++i )
    {
        args[i] = p;
        p += strlen(p) + 1;
    }

    char** argv = args.data();

    if ( m_eventResources &&
         FormatMessage(
             FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
             m_eventResources,
             pevt->EventID,
             0,
             msg,
             sizeof(msg),
             reinterpret_cast<va_list*>(argv)) )
    {
        for ( char* endp = msg + strlen(msg) - 1;
              endp >= msg && (*endp == '\r' || *endp == '\n' || *endp == '.' );
              --endp ) {
              *endp = '\0';
        }
        out << " " << msg;
    }
    else
    {
        for (auto s : args)
            out << " " << s;
    }
    out << '\n';

    return QString(out.str().c_str());
}


void LogMgrWindows::start()
{
    m_logTimer->start(500);
}

void LogMgrWindows::stop()
{
   m_logTimer->stop();
}

ILogMgr *ILogMgr::factory(MainWindow *parent)
{
    return new LogMgrWindows(parent);
}

