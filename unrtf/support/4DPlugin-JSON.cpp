#include "4DPlugin-JSON.h"

void json_wconv(const wchar_t *value, CUTF16String *u16) {
    
    size_t wlen = wcslen(value);
    
#if VERSIONWIN
    *u16 = CUTF16String((const PA_Unichar *)value, wlen);
#else
    CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)value, wlen*sizeof(wchar_t), kCFStringEncodingUTF32LE, true);
    if(str)
    {
        CFIndex len = CFStringGetLength(str);
        std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
        CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
        *u16 = CUTF16String((const PA_Unichar *)&buf[0], len);
        CFRelease(str);
    }
#endif
}

void ob_set_p(PA_ObjectRef obj, const wchar_t *_key, PA_Picture value) {
    
    if(obj)
    {
        if(value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Picture);
            CUTF16String ukey;
            json_wconv(_key, &ukey);
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
            
            PA_SetPictureVariable(&v, value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
    }
}

void ob_set_s(PA_ObjectRef obj, const char *_key, const char *_value) {

    if(obj)
    {
        CUTF8String u8k = CUTF8String((const uint8_t *)_key);
        CUTF8String u8v = CUTF8String((const uint8_t *)_value);
        CUTF16String u16k, u16v;
        
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), (LPWSTR)&buf[0], len)){
                u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
        u8v = CUTF8String((const uint8_t *)_value);
        len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8v.c_str(), u8v.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8v.c_str(), u8v.length(), (LPWSTR)&buf[0], len)){
                u16v = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
#else
        
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8k.c_str(), u8k.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
        str = CFStringCreateWithBytes(kCFAllocatorDefault, u8v.c_str(), u8v.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16v = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
#endif
        
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)u16k.c_str());
        PA_Unistring value = PA_CreateUnistring((PA_Unichar *)u16v.c_str());
        
        PA_Variable v = PA_CreateVariable(eVK_Unistring);
        PA_SetStringVariable(&v, &value);
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
        
    }

}

bool ob_set_s(PA_ObjectRef obj, const wchar_t *_key, const char *_value, int _encoding) {

    bool success = false;
    
    if(_encoding == 0) {
        
        return ob_set_s(obj, _key, _value);
        
    }else{
#if VERSIONMAC
    
    CFStringEncoding encoding = _CFStringConvertWindowsCodepageToEncoding(_encoding);
    
    if((encoding == kCFStringEncodingInvalidId) || !CFStringIsEncodingAvailable(encoding))
    {
        
    }else{
        
        NSString *str = (NSString *)CFStringCreateWithBytes(kCFAllocatorDefault,
                                                            (UInt8 *)_value,
                                                            strlen(_value),
                                                            encoding,
                                                            true);
        if(str)
        {
            uint32_t size = (uint32_t)(((size_t)[str length] * sizeof(PA_Unichar)) + sizeof(PA_Unichar));
            std::vector<uint8_t> buf(size);
            if([str getCString:(char *)&buf[0] maxLength:size encoding:NSUnicodeStringEncoding])
            {
                CUTF16String u16;
                u16 = CUTF16String((const PA_Unichar *)&buf[0], (size_t)[str length]);
                ob_set_a(obj, _key, &u16);
                success = true;
            }else
            {

            }
            [str release];
        }
        
        else{
            //try older API
            TextEncoding textEncoding = TECConvertWindowsCodepageToTextEncoding(_encoding);
            OptionBits flags = kUnicodeForceASCIIRangeMask|kUnicodeStringUnterminatedMask;
            UnicodeMapping mapping;
            mapping.otherEncoding = textEncoding;
            mapping.unicodeEncoding = kTextEncodingUnicodeDefault;
            mapping.mappingVersion = kUnicodeUseLatestMapping;
            TextToUnicodeInfo info;
            CreateTextToUnicodeInfoByEncoding(textEncoding,&info);
            ByteCount sourceLen = strlen(_value);
            ConstLogicalAddress source = _value;
            ByteCount lengthRead = 0;
            ByteCount lengthReturned = 0;
            
            unsigned int size = (unsigned int)((sourceLen * 4) + 1);
            std::vector<uint8_t> buf(size);
            
            if(info)
            {
                ConvertFromTextToUnicode(info,
                                         sourceLen,
                                         source,
                                         flags,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         size,
                                         &lengthRead,
                                         &lengthReturned,
                                         (UniChar *)&buf[0]);
            }
            
            else{
                //try even older API
                TECObjectRef converter;
                TECCreateConverter(&converter, textEncoding, kTextEncodingUnicodeDefault);
                if(converter)
                    TECConvertText(converter,
                                   (ConstTextPtr)source,
                                   sourceLen,
                                   &lengthRead,
                                   (TextPtr)&buf[0],
                                   size,
                                   &lengthReturned);
                
            }
            
            str = [[NSString alloc]initWithBytes:(const void *)&buf[0] length:size encoding:NSUnicodeStringEncoding];
            
            if(str)
            {
                uint32_t size = (uint32_t)(([str length] * sizeof(PA_Unichar)) + sizeof(PA_Unichar));
                std::vector<uint8_t> buf(size);
                
                if([str getCString:(char *)&buf[0] maxLength:size encoding:NSUnicodeStringEncoding])
                {
                    CUTF16String u16;
                    u16 = CUTF16String((const PA_Unichar *)&buf[0], (size_t)[str length]);
                    ob_set_a(obj, _key, &u16);
                    success = true;
                }else{

                }
                
                [str release];
            }
            
        }
        
    }
    
#else
    
    LPSTR mstr;
    UINT ulen, len, mlen;
    DWORD codepage = Param3.getIntValue();
    DWORD mode = 0;
    
    mstr = (LPSTR)Param1.getBytesPtr();
    mlen = Param1.getBytesLength();
    
    IMultiLanguage2 *mlang = NULL;
    CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void **)&mlang);
    
    if(mlang)
    {
        mlang->ConvertStringToUnicode(&mode, codepage, mstr, &mlen, NULL, &ulen);
        len = ((ulen * 2) + 2);
        std::vector<uint8_t> buf(len);
        HRESULT result = mlang->ConvertStringToUnicode(&mode, codepage, mstr, &mlen, (WCHAR *)&buf[0], &ulen);
        
        switch(result){
            case E_FAIL:
                returnValue.setIntValue(ERR_CONVERSION_FAILED);
                break;
            case S_FALSE:
                returnValue.setIntValue(ERR_INVALID_ENCODING);
                break;
            case S_OK:
                Param2.setUTF16String((const PA_Unichar *)&buf[0], ulen);
                break;
        }
        
        mlang->Release();
    }
    
#endif
    }
        
    return success;
}

bool ob_set_s(PA_ObjectRef obj, const wchar_t *_key, const char *_value, std::string& encoding) {
    
    bool success = false;
    
    iconv_t conv = iconv_open(encoding.c_str(), "utf-8");

    if (conv != (iconv_t)-1) {
        
        size_t inLen = strlen(_value);
        size_t outLen = inLen*4;
        std::vector<unsigned char>buf(outLen+sizeof(PA_Unichar));
        char *pIn  = (char *)_value;
        char *pOut  = (char *)&buf[0];
        size_t res = iconv(conv, &pIn, &inLen, &pOut, &outLen);
        if(res != -1) {
            ob_set_s(obj, _key, (const char *)&buf[0]);
            success = true;
        }
        else {
            CUTF8String u8 = CUTF8String((const uint8_t*)_value);
            CUTF16String u16;
#ifdef _WIN32
            int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)u8.c_str(), u8.length(), NULL, 0);

            if (len) {
                std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
                if (MultiByteToWideChar(CP_ACP, 0, (LPCSTR)u8.c_str(), u8.length(), (LPWSTR)&buf[0], len)) {
                    u16 = CUTF16String((const PA_Unichar*)&buf[0]);
                    ob_set_a(obj, _key, (const wchar_t*)u16.c_str());
                    success = true;
                }
            }
#else
            CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8.c_str(), u8.length(), kCFStringEncodingUTF8, true);
            if (str) {
                CFIndex len = CFStringGetLength(str);
                std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
                CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar*)&buf[0]);
                u16 = CUTF16String((const PA_Unichar*)&buf[0]);
                CFRelease(str);
            }
#endif
        }
        iconv_close(conv);
    }
    
    return success;
}

bool ob_set_s(PA_ObjectRef obj, const wchar_t *_key, const char *_value) {
    
    bool success = false;
    
    if(obj)
    {
        if(_value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Unistring);
            CUTF16String ukey;
            
            json_wconv(_key, &ukey);
            
            CUTF8String u8 = CUTF8String((const uint8_t *)_value);
            CUTF16String u16;
            
#ifdef _WIN32
            int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), NULL, 0);
            
            if(len){
                std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
                if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), (LPWSTR)&buf[0], len)){
                    u16 = CUTF16String((const PA_Unichar *)&buf[0]);
                    success = true;
                }
            }else{
                u16 = CUTF16String((const PA_Unichar *)L"");
            }
            
#else
            CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8.c_str(), u8.length(), kCFStringEncodingUTF8, true);
            if(str){
                CFIndex len = CFStringGetLength(str);
                std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
                CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
                u16 = CUTF16String((const PA_Unichar *)&buf[0]);
                CFRelease(str);
                success = true;
            }else{
                u16 = CUTF16String((const PA_Unichar *)L"");
            }
#endif
            if(success) {
                PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
                PA_Unistring value = PA_CreateUnistring((PA_Unichar *)u16.c_str());
                
                PA_SetStringVariable(&v, &value);
                PA_SetObjectProperty(obj, &key, v);
                
                PA_DisposeUnistring(&key);
                PA_ClearVariable(&v);
            }
        }
    }
    
    return success;
}

void ob_set_a(PA_ObjectRef obj, const wchar_t *_key, CUTF16String *value) {
    
    if(obj)
    {
            PA_Variable v = PA_CreateVariable(eVK_Unistring);
            CUTF16String ukey;
            json_wconv(_key, &ukey);
 
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
            PA_Unistring uvalue = PA_CreateUnistring((PA_Unichar *)value->c_str());
            
            PA_SetStringVariable(&v, &uvalue);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
    }
}

void ob_set_a(PA_ObjectRef obj, const wchar_t *_key, const wchar_t *_value) {
    
    if(obj)
    {
        if(_value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Unistring);
            CUTF16String ukey;
            CUTF16String uvalue;
            json_wconv(_key, &ukey);
            json_wconv(_value, &uvalue);
            
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
            PA_Unistring value = PA_CreateUnistring((PA_Unichar *)uvalue.c_str());
            
            PA_SetStringVariable(&v, &value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
    }
}

void ob_set_o(PA_ObjectRef obj, const wchar_t *_key, PA_ObjectRef value) {
    
    if(obj)
    {
        if(value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Object);
            CUTF16String ukey;
            json_wconv(_key, &ukey);
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
            
            PA_SetObjectVariable(&v, value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
    }
}

void ob_set_o(PA_ObjectRef obj, const char *_key, PA_ObjectRef value) {
    
    if(obj)
    {
        CUTF8String u8k = CUTF8String((const uint8_t *)_key);
        
        CUTF16String u16k;
        
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), (LPWSTR)&buf[0], len)){
                u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
#else
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8k.c_str(), u8k.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
#endif
        
        if(value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Object);
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)u16k.c_str());
            
            PA_SetObjectVariable(&v, value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
        
    }
    
}

void ob_set_c(PA_ObjectRef obj, const char *_key, PA_CollectionRef value) {
    
    if(obj)
    {
        CUTF8String u8k = CUTF8String((const uint8_t *)_key);
        
        CUTF16String u16k;
                
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), (LPWSTR)&buf[0], len)){
                u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
#else
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8k.c_str(), u8k.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
#endif
    
        if(value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Collection);
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)u16k.c_str());
            
            PA_SetCollectionVariable(&v, value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
    }
}

void ob_set_c(PA_ObjectRef obj, const wchar_t *_key, PA_CollectionRef value) {
    
    if(obj)
    {
        if(value)
        {
            PA_Variable v = PA_CreateVariable(eVK_Collection);
            CUTF16String ukey;
            json_wconv(_key, &ukey);
            PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
            
            PA_SetCollectionVariable(&v, value);
            PA_SetObjectProperty(obj, &key, v);
            
            PA_DisposeUnistring(&key);
            PA_ClearVariable(&v);
        }
    }
}

void ob_set_n(PA_ObjectRef obj, const wchar_t *_key, double value) {
    
    if(obj)
    {
        PA_Variable v = PA_CreateVariable(eVK_Real);
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        PA_SetRealVariable(&v, value);
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

void ob_set_0(PA_ObjectRef obj, const wchar_t *_key) {
    
    if(obj)
    {
        PA_Variable v = PA_CreateVariable(eVK_Null);
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

void ob_set_n(PA_ObjectRef obj, const char *_key, double value) {
    
    if(obj)
    {
        CUTF8String u8k = CUTF8String((const uint8_t *)_key);
        CUTF16String u16k, u16v;
        
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), (LPWSTR)&buf[0], len)){
                u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
#else
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8k.c_str(), u8k.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
#endif

        PA_Variable v = PA_CreateVariable(eVK_Real);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)u16k.c_str());

        PA_SetRealVariable(&v, value);
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

void ob_set_0(PA_ObjectRef obj, const char *_key) {
    
    if(obj)
    {
        CUTF8String u8k = CUTF8String((const uint8_t *)_key);
        CUTF16String u16k, u16v;
        
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), NULL, 0);
        if(len){
            std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
            if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8k.c_str(), u8k.length(), (LPWSTR)&buf[0], len)){
                u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            }
        }
#else
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, u8k.c_str(), u8k.length(), kCFStringEncodingUTF8, true);
        if(str){
            CFIndex len = CFStringGetLength(str);
            std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
            CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
            u16k = CUTF16String((const PA_Unichar *)&buf[0]);
            CFRelease(str);
        }
#endif

        PA_Variable v = PA_CreateVariable(eVK_Null);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)u16k.c_str());

        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

void ob_set_i(PA_ObjectRef obj, const wchar_t *_key, PA_long32 value) {
    
    if(obj)
    {
        PA_Variable v = PA_CreateVariable(eVK_Longint);
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        PA_SetLongintVariable(&v, value);
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

void ob_set_b(PA_ObjectRef obj, const wchar_t *_key, bool value) {
    
    if(obj)
    {
        PA_Variable v = PA_CreateVariable(eVK_Boolean);
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        PA_SetBooleanVariable(&v, value);
        PA_SetObjectProperty(obj, &key, v);
        
        PA_DisposeUnistring(&key);
        PA_ClearVariable(&v);
    }
}

bool ob_is_defined(PA_ObjectRef obj, const wchar_t *_key) {
    
    bool is_defined = false;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        is_defined = PA_HasObjectProperty(obj, &key);
        
        PA_DisposeUnistring(&key);
    }
    return is_defined;
}

bool ob_get_s(PA_ObjectRef obj, const wchar_t *_key, CUTF8String *value) {
    
    bool is_defined = false;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        is_defined = PA_HasObjectProperty(obj, &key);
        
        if(is_defined)
        {
            is_defined = false;
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Unistring)
            {
                is_defined = true;
                PA_Unistring uvalue = PA_GetStringVariable(v);
                
                CUTF16String u = CUTF16String(uvalue.fString, uvalue.fLength);
#ifdef _WIN32
                int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u.c_str(), u.length(), NULL, 0, NULL, NULL);
                
                if(len){
                    std::vector<uint8_t> buf(len + 1);
                    if(WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u.c_str(), u.length(), (LPSTR)&buf[0], len, NULL, NULL)){
                        *value = CUTF8String((const uint8_t *)&buf[0]);
                    }
                }else{
                    *value = CUTF8String((const uint8_t *)"");
                }
                
#else
                CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)u.c_str(), u.length());
                if(str){
                    
                    size_t size = CFStringGetMaximumSizeForEncoding(
                                                                    CFStringGetLength(str),
                                                                    kCFStringEncodingUTF8) + sizeof(uint8_t);
                    std::vector<uint8_t> buf(size);
                    CFIndex len = 0;
                    CFStringGetBytes(str,
                                     CFRangeMake(
                                                 0,
                                                 CFStringGetLength(str)),
                                     kCFStringEncodingUTF8,
                                     0, true, (UInt8 *)&buf[0], size, &len);
                    
                    *value = CUTF8String((const uint8_t *)&buf[0], len);
                    CFRelease(str);
                }
#endif
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    
    return is_defined;
}

bool ob_get_a(PA_ObjectRef obj, const wchar_t *_key, CUTF16String *value) {
    
    bool is_defined = false;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        is_defined = PA_HasObjectProperty(obj, &key);
        
        if(is_defined)
        {
            is_defined = false;
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Unistring)
            {
                is_defined = true;
                PA_Unistring uvalue = PA_GetStringVariable(v);
                *value = CUTF16String(uvalue.fString, uvalue.fLength);
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    
    return is_defined;
}

bool ob_get_d(PA_ObjectRef obj, const wchar_t *_key, short *dd, short *mm, short *yyyy) {

    bool is_defined = false;

    if (obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        is_defined = PA_HasObjectProperty(obj, &key);

        if (is_defined)
        {
            is_defined = false;

            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if (PA_GetVariableKind(v) == eVK_Date)
            {
                is_defined = true;
                PA_GetDateVariable(v, dd, mm, yyyy);
            }
        }

        PA_DisposeUnistring(&key);
    }

    return is_defined;
}

bool ob_get_b(PA_ObjectRef obj, const wchar_t *_key) {
    
    bool value = false;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        if(PA_HasObjectProperty(obj, &key))
        {
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Boolean)
            {
                value = PA_GetBooleanVariable(v);
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    
    return value;
}

double ob_get_n(PA_ObjectRef obj, const wchar_t *_key) {
    
    double value = 0.0f;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        if(PA_HasObjectProperty(obj, &key))
        {
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Real)
            {
                value = PA_GetRealVariable(v);
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    
    return value;
}

PA_ObjectRef ob_get_o(PA_ObjectRef obj, const wchar_t *_key) {

    PA_ObjectRef value = NULL;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        if(PA_HasObjectProperty(obj, &key))
        {
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Object)
            {
                value = PA_GetObjectVariable(v);
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    return value;
}

PA_CollectionRef ob_get_c(PA_ObjectRef obj, const wchar_t *_key) {
    
    PA_CollectionRef value = NULL;
    
    if(obj)
    {
        CUTF16String ukey;
        json_wconv(_key, &ukey);
        PA_Unistring key = PA_CreateUnistring((PA_Unichar *)ukey.c_str());
        
        if(PA_HasObjectProperty(obj, &key))
        {
            PA_Variable v = PA_GetObjectProperty(obj, &key);
            if(PA_GetVariableKind(v) == eVK_Collection)
            {
                value = PA_GetCollectionVariable(v);
            }
        }
        
        PA_DisposeUnistring(&key);
    }
    return value;
}

void ob_stringify(PA_ObjectRef obj, CUTF8String *value) {
    
    PA_Variable    _params[1];
    _params[0] = PA_CreateVariable(eVK_Object);
    PA_SetObjectVariable(&_params[0], PA_DuplicateObject(obj));
    PA_Variable vjson = PA_ExecuteCommandByID( /*JSON Stringify */1217, _params, 1);
    PA_ClearVariable(&_params[0]);
    
    PA_Unistring ujson = PA_GetStringVariable(vjson);
    
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ujson.fString, ujson.fLength, NULL, 0, NULL, NULL);
    
    if(len){
        std::vector<uint8_t> buf(len + 1);
        if(WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ujson.fString, ujson.fLength, (LPSTR)&buf[0], len, NULL, NULL)){
            *value = CUTF8String((const uint8_t *)&buf[0]);
        }
    }else{
        *value = CUTF8String((const uint8_t *)"");
    }
    
#else
    CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)ujson.fString, ujson.fLength);
    if(str){
        
        size_t size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + sizeof(uint8_t);
        std::vector<uint8_t> buf(size);
        CFIndex len = 0;
        CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, true, (UInt8 *)&buf[0], size, &len);
        
        *value = CUTF8String((const uint8_t *)&buf[0], len);
        CFRelease(str);
    }
    
#endif
    
    PA_ClearVariable(&vjson);
}
