#pragma once

#include <Windows.h>
#include <string>

class UserAppInfo
{
    int nThread_ = 0;
    HANDLE hEventToRun_;
    std::wstring exe_;
    std::wstring arg_;
    std::wstring result_;
public:
    UserAppInfo(HANDLE hEventToRun, const std::wstring& exe, const std::wstring& arg, int nThread) :
        hEventToRun_(hEventToRun), exe_(exe), arg_(arg), nThread_(nThread) {}

    HANDLE waitEvent() const {
        return hEventToRun_;
    }
    std::wstring executable() const {
        return exe_;
    }
    std::wstring arguments() const {
        return arg_;
    }
    void SetThreadNumber(int v) {
        nThread_ = v;
    }
    int threadNumber() const {
        return nThread_;
    }
    void SetResult(const std::wstring& v) {
        result_ = v;
    }
    std::wstring result() const {
        return result_;
    }
};
