#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#ifndef _WIN32
#include "safeunistd.h"
#endif
#endif

#ifdef _WIN32
#include <unistd.h>
#endif

#include <stdlib.h>

#include "path.h"
#include "malloc.h"

#include <string>
#include <vector>

char *search_in_path(const char *name, char *suffix)
{
    std::string path;
    
#if defined(__APPLE__)
    NSBundle *thisBundle = [NSBundle bundleWithIdentifier:THIS_BUNDLE_ID];
    if(thisBundle){
        NSString *str = [thisBundle resourcePath];
        if(str){
            path = [str UTF8String];
            path += (const char *)"/";
         }
    }
#else
    wchar_t fDrive[_MAX_DRIVE], fDir[_MAX_DIR], fName[_MAX_FNAME], fExt[_MAX_EXT];
    wchar_t thisPath[_MAX_PATH] = {0};
    
    HMODULE hplugin = GetModuleHandleW(THIS_BUNDLE_NAME);
    
    GetModuleFileNameW(hplugin, thisPath, _MAX_PATH);
    
    _wsplitpath_s(thisPath, fDrive, fDir, fName, fExt);
    std::wstring windowsPath = fDrive;
    windowsPath+= fDir;
    
    //remove delimiter to go one level up the hierarchy
    if(windowsPath.at(windowsPath.size() - 1) == L'\\')
        windowsPath = windowsPath.substr(0, windowsPath.size() - 1);
    
    _wsplitpath_s(windowsPath.c_str(), fDrive, fDir, fName, fExt);
    std::wstring resourcesPath = fDrive;
    resourcesPath += fDir;
    resourcesPath += L"Resources\\";
    
    std::wstring u16 = (const wchar_t *)resourcesPath;
    
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), NULL, 0, NULL, NULL);
    
    if(len){
        std::vector<uint8_t> buf(len + 1);
        if(WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), (LPSTR)&buf[0], len, NULL, NULL)){
            path = std::string((const char *)&buf[0]);
        }
    }else{
        path = std::string((const char *)"");
    }
#endif

    path += (const char *)name;
    path += (const char *)".";
    path += (const char *)suffix;
    
    char *_path = 0;
    
    if ((_path = my_malloc(path.length() + 1)) == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }
    strcpy(_path, (char *)path.c_str());
    return _path;
}
