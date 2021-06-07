#include <iostream>

#include <Windows.h>

HANDLE ghEventWaitRun;
int main()
{
    HANDLE ghEventWaitRun = CreateEvent(NULL, TRUE, FALSE, NULL);

    std::cout << "Hello World!\n";
}
