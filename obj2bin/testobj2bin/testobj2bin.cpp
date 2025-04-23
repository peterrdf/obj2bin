// testobj2bin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../obj2bin/obj2bin.h"
#include "_log.h"

#include <atlbase.h>

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// ************************************************************************************************
class _console_log : public _log
{

public: // Methods

    _console_log()
        : _log()
    {
    }

    virtual ~_console_log()
    {
    }

    virtual void logWrite(enumLogEvent enLogEvent, const std::string& strEvent) override
    {
        switch (enLogEvent) {
            case enumLogEvent::info:
            {
                std::cout << "\nInformation: " << strEvent.c_str();
            }
            break;

            case enumLogEvent::warning:
            {
                std::cout << "\nWarning: " << strEvent.c_str();
            }
            break;

            case enumLogEvent::error:
            {
                std::cout << "\nError: " << strEvent.c_str();
            }
            break;

            default:
            {
                std::cout << "\nUnknown: " << strEvent.c_str();
            }
            break;
        } // switch (enLogEvent)
    }
};

// ************************************************************************************************
void STDCALL LogCallbackImpl(enumLogEvent enLogEvent, const char* szEvent)
{
    switch (enLogEvent) {
        case enumLogEvent::info:
        {
            std::cout << "\nInformation: " << szEvent;
        }
        break;

        case enumLogEvent::warning:
        {
            std::cout << "\x1B[33m" << "\nWarning: " << szEvent << "\033[0m";

        }
        break;

        case enumLogEvent::error:
        {
            std::cout << "\x1B[31m" << "\nError: " << szEvent << "\033[0m";
        }
        break;

        default:
        {
            std::cout << "\x1B[33m" << "\nUnknown: " << szEvent << "\033[0m";
        }
        break;
    } // switch (enLogEvent)
}

// ************************************************************************************************
_console_log* g_pLog = nullptr;

// ************************************************************************************************
void convertFile(const fs::path& pthInputFile, const fs::path& pthOutputFolder)
{
    string strExtension = pthInputFile.extension().string();
    _string::toLower(strExtension);

    if (strExtension != ".obj") {
        return;
    }

    g_pLog->logWrite(enumLogEvent::info, _string::sformat("Importing: '%s'", pthInputFile.string().c_str()));

    fs::path pthOutputSubFolder = pthOutputFolder;
    fs::create_directories(pthOutputSubFolder);

    fs::path pthOutputFile = pthOutputSubFolder;
    pthOutputFile.append(pthInputFile.filename());
    pthOutputFile += ".bin";

    SaveAsBIN(pthInputFile.string().c_str(), pthOutputFile.string().c_str());
}

// ************************************************************************************************
void convertFiles(const fs::path& pthInputFolder, const fs::path& pthOutputFolder)
{
    for (const auto& entry : fs::directory_iterator(pthInputFolder)) {
        if (fs::is_directory(entry)) {
            fs::path pthOutputSubFolder = pthOutputFolder;
            pthOutputSubFolder.append(entry.path().filename());
            fs::create_directories(pthOutputSubFolder);

            convertFiles(entry.path(), pthOutputSubFolder);

            continue;
        }

        convertFile(entry.path(), pthOutputFolder);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cout << "\nUsage:";
        std::cout << "\n-convert \"INPUT FOLDER\" \"OUTPUT FOLDER\"";

        return 0;
    }

    fs::path pthExe = argv[0];
    auto pthRootFolder = pthExe.parent_path();
    wstring strRootFolder = pthRootFolder.wstring();
    strRootFolder += L"\\";

    string strCommand = argv[1];

    _console_log log;
    g_pLog = &log;

    SetOBJ2BINOptions(LogCallbackImpl);

    fs::path pthInputFolder = argv[2];
    fs::path pthOutputFolder = argv[3];
    for (const auto& entry : fs::directory_iterator(pthInputFolder)) {
        if (fs::is_directory(entry)) {
            fs::path pthOutputSubFolder = pthOutputFolder;
            pthOutputSubFolder.append(entry.path().filename());
            fs::create_directories(pthOutputSubFolder);

            convertFiles(entry.path(), pthOutputSubFolder);

            continue;
        }

        convertFile(entry.path(), pthOutputFolder);
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
