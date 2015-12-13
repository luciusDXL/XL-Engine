#pragma once

#include <string>
#include "../xlServices.h"

using namespace std;

class DynamicLibrary;
typedef void (*XL_RunFunc)(short int, char*[], XLEngineServices*); 

class PluginManager
{
public:

	static bool init();
	static void destroy();

	static XL_RunFunc initGame(const string& path);
	static void unloadGame();

private:
	static DynamicLibrary *m_gameLib;
	static XL_RunFunc m_runFunc;
};
