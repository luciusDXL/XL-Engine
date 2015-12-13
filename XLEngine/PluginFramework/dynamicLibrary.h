#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

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

#endif
