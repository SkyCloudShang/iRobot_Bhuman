/**
 * @file Module.cpp
 * The class attributes of the module handling schema.
 * @author Thomas Röfer
 */

#include "Module.h"
#include "Tools/Streams/InStreams.h"

ModuleBase* ModuleBase::first = nullptr;

void loadModuleParameters(Streamable& parameters, const char* moduleName, const char* fileName)
{
  std::string name;
  if(!fileName)
  {
    name = moduleName;
    name[0] = static_cast<char>(tolower(name[0]));
    if(name.size() > 1 && isupper(name[1]))
      for(int i = 1; i + 1 < static_cast<int>(name.size()) && isupper(name[i + 1]); ++i)
        name[i] = static_cast<char>(tolower(name[i]));
    name += ".cfg";
  }
  else
    name = fileName;
  InMapFile stream(name);
  ASSERT(stream.exists());
  stream >> parameters;
}
