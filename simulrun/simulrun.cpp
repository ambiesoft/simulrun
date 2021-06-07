#include "stdafx.h"

#include <iostream>
#include <string>
#include <Windows.h>
#include <process.h>
#include <memory>
#include <vector>
#include "../../lsMisc/CHandle.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/CommandLineString.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/stdosd/stdosd.h"

#include "UserAppInfo.h"

#define I18N(s) (s)

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace std;

CHandle ghEventWaitRun;
constexpr wchar_t NEWLINE[] = L"\r\n";
constexpr wchar_t APPNAME[] = L"simulrun";
constexpr wchar_t APPVERSION[] = L"1.0.2";

void ErrorExit(const wstring& str)
{
    wcerr << str << endl;
    exit(1);
}
void ErrorExitWithLastError(
    const wstring& str = wstring(),
    const DWORD dwLE = GetLastError())
{
    wstring msg;
    if (!str.empty())
    {
        msg += str;
        msg += NEWLINE;
    }
    msg += GetLastErrorString(dwLE);

    wcerr << msg << endl;
    exit(dwLE);
}

unsigned __stdcall start_of_thread(void* pArg)
{
    UserAppInfo* appInfo = (UserAppInfo*)pArg;

    WaitForSingleObject(appInfo->waitEvent(), INFINITE);

    if (!OpenCommon(NULL, appInfo->executable().c_str(), appInfo->arguments().c_str()))
        appInfo->SetResult(stdFormat(L"Thread %d failed to open", appInfo->threadNumber()));
    else
        appInfo->SetResult(stdFormat(L"Thread %d opened at %d", appInfo->threadNumber(), GetTickCount()));

    _endthreadex(0);
    return 0;
}

unique_ptr<HANDLE> rawPointer(const vector<CHandle>& vc)
{
    HANDLE* p = new HANDLE[vc.size()];
    for (size_t i = 0; i < vc.size(); ++i)
        p[i] = vc[i];

    return unique_ptr<HANDLE>(p);
}

int main()
{
    CCommandLineParser parser(I18N(L"Run executable simultaineously"), APPNAME);

    int nRunCount = 0;
    parser.AddOption(L"-n", 1, &nRunCount, ArgEncodingFlags_Default,
        I18N(L"Specify count to run"));

    bool bCmdHelp = false;
    parser.AddOptionRange({ L"/?",L"-h",L"--help" },
        0,
        &bCmdHelp,
        ArgEncodingFlags_Default,
        I18N(L"Shows help"));

    bool bCmdVersion = false;
    parser.AddOptionRange({ L"-v",L"--version" },
        0,
        & bCmdVersion,
        ArgEncodingFlags_Default,
        I18N(L"Shows version"));

    wstring dummy;
    parser.AddOption(L"", 10, &dummy, ArgEncodingFlags_Default,
        I18N(L"Command Line"));

    parser.Parse();

    if (bCmdHelp)
    {
        wcout << parser.getHelpMessage() << endl;
        return 0;
    }
    if (bCmdVersion)
    {
        wcout << APPNAME << L" v" << APPVERSION << endl;
        return 0;
    }
    if (nRunCount <= 0)
        ErrorExit(I18N(L"Run-count must be more than 1"));

    CCommandLineString cmd;
    int nIndexN = cmd.getIndex(L"-n");
    if (nIndexN <= 0)
        ErrorExit(I18N(L"-n must be specified"));
    int nAppArgStartPos = nIndexN + 2;
    wstring usrApp = cmd.getArg(nAppArgStartPos);
    if (usrApp.empty())
        ErrorExit(I18N(L"Executable not found"));
    wstring usrArg = cmd.subString(nAppArgStartPos + 1);
    
    ghEventWaitRun = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!ghEventWaitRun)
        ErrorExitWithLastError(L"CreateEvent");

    vector<UserAppInfo> vAppInfo;
    vAppInfo.reserve(nRunCount);
    for (int i = 0; i < nRunCount; ++i)
    {
        vAppInfo.emplace_back(UserAppInfo(ghEventWaitRun, usrApp, usrArg, i));
    }
    
    vector<CHandle> threads;
    threads.reserve(nRunCount);
    for (int i = 0; i < nRunCount; ++i)
    {
        unsigned dwThreadId = 0;
        CHandle t ((HANDLE)_beginthreadex(
            NULL,       // security
            0,          // stacksize
            start_of_thread,
            &vAppInfo[i],       // arg
            0,          // flags
            &dwThreadId       // thread id
        ));
        if (!t || dwThreadId == 0)
            ErrorExitWithLastError(I18N(L"Failed to create a thread"));

        threads.emplace_back(move(t));
    }

    assert(threads.size() == (size_t)nRunCount);

    if (!SetEvent(ghEventWaitRun))
        ErrorExit(L"SetEvent");

    WaitForMultipleObjects(
        (DWORD)threads.size(),
        rawPointer(threads).get(),
        TRUE,
        INFINITE);

    for (auto&& appInfo: vAppInfo)
    {
        wcout << appInfo.result() << NEWLINE;
    }
    return 0;
}
