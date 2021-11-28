/* --------------------------------------------------------------------------------
 #
 #	4DPlugin-UnRTF.h
 #	source generated by 4D Plugin Wizard
 #	Project : UnRTF
 #	author : miyako
 #	2021/11/28
 #  
 # --------------------------------------------------------------------------------*/

#ifndef PLUGIN_UNRTF_H
#define PLUGIN_UNRTF_H

#include "4DPluginAPI.h"
#include "4DPlugin-JSON.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if VERSIONMAC
#include <stdio.h>
#else
#include <io.h>
#include <fcntl.h>
FILE *fmemopen(const char* ptr, size_t sz, int p[2])
{
    int rc = _pipe(p, (unsigned int)sz, _O_BINARY);
    if(rc != 0)
        return NULL;
    // まず書き込んでやる
    _write(p[WRITE], ptr, (unsigned int)sz);
    // 読み込みをFILE構造体に変換
    return _fdopen(p[READ], "rb");
}
#endif

#include "convert.h"
#include "word.h"
#include "parse.h"
#include "output.h"
#include "user.h"
#include "attr.h"

#pragma mark -

static void UnRTF(PA_PluginParameters params);

static void u16_to_u8(CUTF16String& u16, std::string& u8);
static void u8_to_u16(std::string& u8, CUTF16String& u16);

#endif /* PLUGIN_UNRTF_H */
