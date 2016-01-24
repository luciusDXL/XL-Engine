#if _WIN32
  #include <Windows.h>
#else
  #include <dlfcn.h>
#endif

#include "dynamicLibrary.h"
#include "../log.h"
#include <sstream>
#include <iostream>

#if _OSX
	static string dynamicLibraryExtension("dylib");
#elif _WIN32
    static string dynamicLibraryExtension("dll");
#else
	static string dynamicLibraryExtension("so");
#endif

DynamicLibrary::DynamicLibrary(void *handle)
{
	m_handle = handle;
}

DynamicLibrary::~DynamicLibrary()
{
	if (m_handle)
	{
		#if _WIN32
			FreeLibrary( (HMODULE)m_handle );
		#else
			dlclose( m_handle );
		#endif
	}
}

DynamicLibrary *DynamicLibrary::load(const string& name, string& errorString)
{
	if (name.empty())
	{
		LOG( LOG_ERROR, "DynamicLibrary: Empty path." );
		return NULL;
	}

	void *handle = NULL;
	string path = name + "." + dynamicLibraryExtension;

#if _WIN32
	handle = LoadLibraryA( path.c_str() );
	if ( !handle )
	{
		DWORD errorCode = GetLastError();
		LOG( LOG_ERROR, "LoadLibrary(%s) failed, errorCode: %x", name.c_str(), errorCode );
		return NULL;
	}
#else
	handle = dlopen( path.c_str(), RTLD_NOW );
	if (!handle)
	{
		string dlErrorString;
		const char *zErrorString = dlerror();
		LOG( LOG_ERROR, "dlopen(%s) failed, error: %s", name.c_str(), zErrorString ? zErrorString : "unknown" );
		return NULL;
	}
#endif

	return new DynamicLibrary(handle);
}

void *DynamicLibrary::getSymbol(const std::string& symbol)
{
	if ( !m_handle )
		return NULL;

#if _WIN32
	return GetProcAddress( (HMODULE)m_handle, symbol.c_str() );
#else
	return dlsym( m_handle, symbol.c_str() );
#endif
}
