#pragma once

#include <string>
using namespace std;

class DynamicLibrary
{
public:

  static DynamicLibrary *load(const string& path, string& errorString);
  ~DynamicLibrary();
  
  void *getSymbol(const string& name);

private:
  DynamicLibrary();
  
  DynamicLibrary(void *handle);
  DynamicLibrary(const DynamicLibrary &);
  
private:
  void *m_handle;
};
