#pragma once
#include "hkxutils.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Base/Reflection/Registry/hkDynamicClassNameRegistry.h>
#include <Common/Base/Reflection/Registry/hkDefaultClassNameRegistry.h>

// Scene
#include <Common/Serialize/Util/hkSerializeUtil.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Common/Serialize/Util/hkNativePackfileUtils.h>
#include <Common/Serialize/Util/hkLoader.h>
#include <Common/Serialize/Version/hkVersionPatchManager.h>
#include <Common/Compat/Deprecated/Packfile/Binary/hkBinaryPackfileReader.h>
#include <Common/Compat/Deprecated/Packfile/Xml/hkXmlPackfileReader.h>

#include <iostream>

enum hkPackFormat
{
   HKPF_XML,
   HKPF_DEFAULT,
   HKPF_WIN32,
   HKPF_AMD64,
   HKPF_XBOX,
   HKPF_XBOX360,
   HKPF_TAGFILE,
   HKPF_TAGXML,
};

// Custom enumeration reflection data
extern EnumLookupType PackFlags[];

extern hkPackfileWriter::Options GetWriteOptionsFromFormat(hkPackFormat format);
extern void HK_CALL errorReport(const char* msg, void* userContext);

extern hkResource* hkSerializeUtilLoad( hkStreamReader* stream
                                , hkSerializeUtil::ErrorDetails* detailsOut=nullptr
                                , const hkClassNameRegistry* classReg=nullptr
                                , hkSerializeUtil::LoadOptions options=hkSerializeUtil::LOAD_DEFAULT );


extern hkResult hkSerializeUtilSave( hkPackFormat pkFormat, hkVariant &root, hkOstream &stream
                                    , hkSerializeUtil::SaveOptionBits flags
                                    , const hkPackfileWriter::Options& packFileOptions );
extern hkResult hkSerializeLoad(hkStreamReader *reader, hkVariant &root, hkResource *&resource);
extern hkResource *hkSerializeLoadResource(hkStreamReader *reader);

extern hkResult LoadDefaultRegistry();
