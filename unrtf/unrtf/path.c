#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include "safeunistd.h"
#endif

#include <stdlib.h>

#include "path.h"
#include "malloc.h"

char *search_in_path(const char *name, char *suffix)
{
    CUTF8String path;
    
#if __APPLE__
    NSBundle *thisBundle = [NSBundle bundleWithIdentifier:THIS_BUNDLE_ID];
    if(thisBundle)
    {
        NSString *resourcePath = [thisBundle resourcePath];
        if(resourcePath)
        {
            C_TEXT t;
            t.setUTF16String(resourcePath);
            t.copyUTF8String(&path);
            path += (const uint8_t *)"/";
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
    
    C_TEXT t;
    t.setUTF16String((const PA_Unichar *)resourcesPath.c_str(), resourcesPath.length());
    t.copyUTF8String(&path);
    
#endif

    path += (const uint8_t *)name;
    path += (const uint8_t *)".";
    path += (const uint8_t *)suffix;
    
    char *_path = 0;
    
    if ((_path = my_malloc(path.length() + 1)) == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }
    strcpy(_path, (char *)path.c_str());
    return _path;
}
