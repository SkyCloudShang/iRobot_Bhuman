/**
 * @file Tools/Debugging/DebugDrawings.cpp
 *
 * Functions for Debugging
 *
 * @author Michael Spranger
 */

#include "DebugDrawings.h"
#include "Platform/BHAssert.h"

void DrawingManager::addDrawingId(const char* name, const char* typeName)
{
  if(drawings.find(name) == drawings.end())
  {
    char id = static_cast<char>(drawings.size());
    Drawing& drawing = drawings[name];
    drawing.id = id;
    drawing.processIdentifier = processIdentifier;

    std::unordered_map< const char*, char>::const_iterator i = types.find(typeName);
    if(i == types.end())
    {
      drawing.type = static_cast<char>(types.size());
      types[typeName] = drawing.type;
      unsigned int key = static_cast<unsigned int>(processIdentifier) << 24 | static_cast<unsigned int>(drawing.type);
      typesById[key] = typeName;
    }
    else
      drawing.type = i->second;

    unsigned int key = static_cast<unsigned int>(processIdentifier) << 24 | static_cast<unsigned int>(id);
    drawingsById[key] = name;
  }
}

void DrawingManager::clear()
{
  types.clear();
  drawings.clear();
  strings.clear();
  drawingsById.clear();
  typesById.clear();
}

const char* DrawingManager::getString(const std::string& string)
{
  std::unordered_map<std::string, const char*>::iterator i = strings.find(string);
  if(i == strings.end())
  {
    strings[string];
    i = strings.find(string);
    i->second = i->first.c_str();
  }
  return i->second;
}

In& operator>>(In& stream, DrawingManager& drawingManager)
{
  // note that this operator appends the data read to the drawingManager
  // clear() has to be called first to replace the existing data

  int size;
  stream >> size;;
  for(int i = 0; i < size; ++i)
  {
    std::string str;
    int id;
    stream >> id >> str;
    const char* name = drawingManager.getString(str);
    unsigned int key = static_cast<unsigned int>(drawingManager.processIdentifier) << 24 | static_cast<unsigned int>(id);
    drawingManager.types[name] = static_cast<char>(id);
    drawingManager.typesById[key] = name;
  }

  stream >> size;
  for(int i = 0; i < size; ++i)
  {
    std::string str;
    int id, type;
    stream >> id >> type >> str;
    const char* name = drawingManager.getString(str);
    DrawingManager::Drawing& entry = drawingManager.drawings[name];
    entry.id = static_cast<char>(id);
    entry.type = static_cast<char>(type);
    entry.processIdentifier = drawingManager.processIdentifier;
    unsigned int key = static_cast<unsigned int>(entry.processIdentifier) << 24 | static_cast<unsigned int>(entry.id);
    drawingManager.drawingsById[key] = name;
  }

  return stream;
}

Out& operator<<(Out& stream, const DrawingManager& drawingManager)
{
  stream << static_cast<int>(drawingManager.types.size());
  for(std::unordered_map<const char*, char>::const_iterator iter = drawingManager.types.begin(); iter != drawingManager.types.end(); ++iter)
  {
    stream << static_cast<int>(iter->second);
    stream << iter->first;
  }

  stream << static_cast<int>(drawingManager.drawings.size());
  for(std::unordered_map< const char*, DrawingManager::Drawing>::const_iterator iter = drawingManager.drawings.begin(); iter != drawingManager.drawings.end(); ++iter)
  {
    stream << static_cast<int>(iter->second.id);
    stream << static_cast<int>(iter->second.type);
    stream << iter->first;
  }

  return stream;
}
