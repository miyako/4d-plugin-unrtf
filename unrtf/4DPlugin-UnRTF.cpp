/* --------------------------------------------------------------------------------
 #
 #  4DPlugin-UnRTF.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : UnRTF
 #	author : miyako
 #	2021/11/28
 #  
 # --------------------------------------------------------------------------------*/

#include "4DPlugin-UnRTF.h"

/*
 
 for now, these options are fixed
 
 */

int nopict_mode = 0; /* TRUE => Do not write \pict's to files */
int dump_mode   = 0; /* TRUE => Output a dump of the RTF word tree */
int debug_mode  = 0; /* TRUE => Output debug comments within HTML */
int lineno      = 0; /* Used for error reporting and final line count. */
int simple_mode = 0; /* TRUE => Output HTML without SPAN/DIV tags -- This would
                  probably be more useful if we could pull out <font> tags
                  as well. */
int inline_mode = 0; /* TRUE => Output HTML without HTML/BODY/HEAD -- This is
                  buggy. I've seen it output pages of </font> tags. */
/* marcossamaral - 0.19.9 */

int verbose_mode  = 0; /* TRUE => Output additional informations about unrtf */
int no_remap_mode = 0; /* don't remap codepoints */
int quiet = 1;         /* TRUE => don't output header comments */

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    
	try
	{
        switch(selector)
        {
			// --- UnRTF
            
			case 1 :
				UnRTF(params);
				break;

        }

	}
	catch(...)
	{

	}
}

#pragma mark -

static bool getUnRTFConf(std::string& path) {
#if VERSIONMAC
    NSBundle *thisBundle = [NSBundle bundleWithIdentifier:THIS_BUNDLE_ID];
    if(thisBundle){
        NSString *str = [thisBundle resourcePath];
        if(str){
            path = [str UTF8String];
            path += "/";
            return true;
         }
    }
    return false;
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
    
    CUTF16String u16 = (const PA_Unichar *)resourcesPath.c_str();
    
    u16_to_u8(u16, path);
    
    return true;
#endif
}

typedef enum {
 
    unrtf_format_text = 1,
    unrtf_format_vt = 2,
    unrtf_format_latex = 3,
    unrtf_format_html = 4
    
}unrtf_format_t;

static void UnRTF(PA_PluginParameters params) {

    PA_ObjectRef status = PA_CreateObject();
    ob_set_b(status, L"success", false);
    
    PA_Handle h = PA_GetBlobHandleParameter(params, 1);
    
    if(h) {
        
        unrtf_format_t fmt = unrtf_format_html;
        
        std::string encoding;
        int codepage = 0;
        
        std::string path;
        if(getUnRTFConf(path)) {
            PA_ObjectRef options = PA_GetObjectParameter(params, 2);
            if(options) {
                
                codepage = ob_get_n(options, L"codepage");

//                CUTF8String _encoding;
//                if(ob_get_s(options, L"encoding", &_encoding) &&(_encoding.length())) {
//                    encoding = (const char *)_encoding.c_str();
//                }
                
                CUTF8String format;
                if(ob_get_s(options, L"format", &format)) {
                    if(format == (const uint8_t *)"tex"){
                        fmt = unrtf_format_latex;goto set_format;
                    }
//                    if(format == (const uint8_t *)"vt"){
//                        fmt = unrtf_format_vt;goto set_format;
//                    }
                    if(format == (const uint8_t *)"txt"){
                        fmt = unrtf_format_text;goto set_format;
                    }
                    if(format == (const uint8_t *)"html"){
                        fmt = unrtf_format_html;goto set_format;
                    }
                }
            }
        }
        
        set_format:
        
        switch (fmt) {
            case unrtf_format_text:
                path += "text.conf";
                break;
//            case unrtf_format_vt:
//                path += "vt.conf";
//                break;
            case unrtf_format_latex:
                path += "latex.conf";
                break;
            case unrtf_format_html:
            default:
                path += "html.conf";
                break;
        }
        
        OutputPersonality *op = NULL;
        op = user_init(op, (char *)path.c_str());
        
        if(op) {
//            add_alias(op, 0xE9, "é");
#if VERSIONWIN
			std::FILE* f = std::tmpfile();
			std::fwrite(PA_LockHandle(h), sizeof(char), (size_t)PA_GetHandleSize(h), f);
			std::rewind(f);
#else
            FILE *f = fmemopen(PA_LockHandle(h), PA_GetHandleSize(h), "r");
#endif
            if(f) {
                
                Word *word;
                try {
                    word = word_read(f);
                    std::string output;
                    word_print(word, output, op);
                    bool success = ob_set_s(status, L"result", output.c_str(), codepage);
                    if(success) {
                        ob_set_b(status, L"success", true);
                    }else{
                        ob_set_s(status, L"error", "bad encoding");
                    }
                } catch (const std::invalid_argument& e) {
                    ob_set_s(status, L"error", e.what());
                }
#if VERSIONWIN
                fclose(f);
#else
                fclose(f);
#endif
            }
            PA_UnlockHandle(h);
        }

    }
 
    PA_ReturnObject(params, status);
}

#pragma mark -

static void u16_to_u8(CUTF16String& u16, std::string& u8) {
    
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), NULL, 0, NULL, NULL);
    
    if(len){
        std::vector<uint8_t> buf(len + 1);
        if(WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), (LPSTR)&buf[0], len, NULL, NULL)){
            u8 = std::string((const char *)&buf[0]);
        }
    }else{
        u8 = std::string((const char *)"");
    }

#else
    CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)u16.c_str(), u16.length());
    if(str){
        size_t size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + sizeof(uint8_t);
        std::vector<uint8_t> buf(size);
        CFIndex len = 0;
        CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, true, (UInt8 *)&buf[0], size, &len);
        u8 = std::string((const char *)&buf[0], len);
        CFRelease(str);
    }
#endif
}

static void u8_to_u16(std::string& u8, CUTF16String& u16) {
    
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), NULL, 0);
    
    if(len){
        std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
        if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), (LPWSTR)&buf[0], len)){
            u16 = CUTF16String((const PA_Unichar *)&buf[0]);
        }
    }else{
        u16 = CUTF16String((const PA_Unichar *)L"");
    }
    
#else
    CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)u8.c_str(), u8.length(), kCFStringEncodingUTF8, true);
    if(str){
        CFIndex len = CFStringGetLength(str);
        std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
        CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
        u16 = CUTF16String((const PA_Unichar *)&buf[0]);
        CFRelease(str);
    }
#endif
}
