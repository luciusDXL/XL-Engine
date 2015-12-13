#include "PluginManager.h"
#include "DynamicLibrary.h"
#include "../types.h"

DynamicLibrary *PluginManager::m_gameLib;
XL_RunFunc PluginManager::m_runFunc;

bool PluginManager::init()
{
	m_gameLib = NULL;
	return true;
}

void PluginManager::destroy()
{
	//Unload the current game.
	unloadGame();
}

XL_RunFunc PluginManager::initGame(const string& path)
{
	//first unload the game, since we can only have one at a time right now.
	unloadGame();

	//load the game lib.
	string errorString;
	m_gameLib = DynamicLibrary::load(path, errorString);
    if (!m_gameLib) // not a dynamic library? 
	{
		return NULL;
	}

	//get the run function.
    m_runFunc = (XL_RunFunc)(m_gameLib->getSymbol("RunGame"));
    if (!m_runFunc) // dynamic library missing entry point?
	{
		return NULL;
	}

	return m_runFunc;
}

void PluginManager::unloadGame()
{
	if ( m_gameLib )
	{
		delete m_gameLib;
		m_gameLib = NULL;
	}
}
