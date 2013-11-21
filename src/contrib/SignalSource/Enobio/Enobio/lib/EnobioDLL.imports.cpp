// Import definitions for EnobioDLL.h, generated by dylib_imports.sh
#include "EnobioDLL.imports.h"
#include "DylibImports.h"

#if !DYNAMIC_IMPORTS

namespace Dylib { bool EnobioDLL_Loaded() { return true; } }

#else // DYNAMIC_IMPORTS

extern "C" {
Enobio* (__cdecl *CEnobioCreate)() = 0;
bool (__cdecl *CEnobioInitialize)(Enobio* enobioClass,int sampleBlockSize) = 0;
bool (__cdecl *CEnobioCaptureStart) (Enobio* enobioClass)  = 0;
bool (__cdecl *CEnobioWaitForData)(Enobio* enobioClass,int** buf,int sampleBlockSize) = 0;
bool (__cdecl *CEnobioCaptureStop) (Enobio* enobioClass) = 0;
bool (__cdecl *CEnobioClose)(Enobio* enobioClass) = 0;
int (__cdecl *CEnobioGetLastError)(Enobio* enobioClass) = 0;
const char * (__cdecl *CEnobioStrError)(int NumError,Enobio* enobioClass) = 0;
bool (__cdecl *CEnobioIsInitialized)(Enobio* enobioClass) = 0;
int* (__cdecl *CEnobioChannelStatus)(Enobio*) = 0;
void (__cdecl *CEnobioResetBuffer)(Enobio* enobioClass) = 0;
}

static const Dylib::Import imports[] =
{
  { "CEnobioCreate", (void**)&CEnobioCreate, Dylib::Import::cMangled },
  { "CEnobioInitialize", (void**)&CEnobioInitialize, Dylib::Import::cMangled },
  { "CEnobioCaptureStart", (void**)&CEnobioCaptureStart, Dylib::Import::cMangled },
  { "CEnobioWaitForData", (void**)&CEnobioWaitForData, Dylib::Import::cMangled },
  { "CEnobioCaptureStop", (void**)&CEnobioCaptureStop, Dylib::Import::cMangled },
  { "CEnobioClose", (void**)&CEnobioClose, Dylib::Import::cMangled },
  { "CEnobioGetLastError", (void**)&CEnobioGetLastError, Dylib::Import::cMangled },
  { "CEnobioStrError", (void**)&CEnobioStrError, Dylib::Import::cMangled },
  { "CEnobioIsInitialized", (void**)&CEnobioIsInitialized, Dylib::Import::cMangled },
  { "CEnobioChannelStatus", (void**)&CEnobioChannelStatus, Dylib::Import::cMangled },
  { "CEnobioResetBuffer", (void**)&CEnobioResetBuffer, Dylib::Import::cMangled },
  { 0, 0, 0 }
};

// Here you may specify a custom error message to be displayed when the library cannot be found.
static const char* notFoundMsg = "";
// Here you may specify an URL to some local or remote help resource.
static const char* notFoundURL = "";
RegisterDylib( EnobioDLL, imports, notFoundMsg, notFoundURL );

#endif // DYNAMIC_IMPORTS