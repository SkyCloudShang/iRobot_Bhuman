/**
 * @file OutStreams.cpp
 *
 * Implementation of out stream classes.
 *
 * @author Thomas Röfer
 * @author Martin Lötzsch
 */

#include <cstdio>
#include <cstring>

#include "OutStreams.h"
#include "Platform/File.h"
#include "Platform/BHAssert.h"
#include "Tools/Motion/SensorData.h"
#include "Tools/Debugging/Debugging.h"
#include "Tools/Math/Angle.h"

void OutBinary::writeString(const char* d, PhysicalOutStream& stream)
{
  size_t size = strlen(d);
  stream.writeToStream(&size, sizeof(unsigned));
  stream.writeToStream(d, size);
}

void OutBinary::writeAngle(const Angle& d, PhysicalOutStream& stream)
{
  writeFloat(d, stream);
}

void OutText::writeBool(bool value, PhysicalOutStream& stream)
{
  if(value)
    stream.writeToStream(" true", 5);
  else
    stream.writeToStream(" false", 6);
}

void OutText::writeString(const char* value, PhysicalOutStream& stream)
{
  stream.writeToStream(" ", 1);
  bool containsSpaces = !*value || *value == '"' || strcspn(value, " \n\r\t") < strlen(value);
  if(containsSpaces)
    stream.writeToStream("\"", 1);
  for(; *value; ++value)
    if(*value == '"' && containsSpaces)
      stream.writeToStream("\\\"", 2);
    else if(*value == '\n')
      stream.writeToStream("\\n", 2);
    else if(*value == '\r')
      stream.writeToStream("\\r", 2);
    else if(*value == '\t')
      stream.writeToStream("\\t", 2);
    else if(*value == '\\')
      stream.writeToStream("\\\\", 2);
    else
      stream.writeToStream(value, 1);
  if(containsSpaces)
    stream.writeToStream("\"", 1);
}

void OutText::writeData(const void* p, size_t size, PhysicalOutStream& stream)
{
  for(size_t i = 0; i < size; ++i)
    writeChar(*((const char*&)p)++, stream);
}

void OutText::writeChar(char d, PhysicalOutStream& stream)
{
  sprintf(buf, " %d", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeSChar(signed char d, PhysicalOutStream& stream)
{
  sprintf(buf, " %u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeUChar(unsigned char d, PhysicalOutStream& stream)
{
  sprintf(buf, " %u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeShort(short d, PhysicalOutStream& stream)
{
  sprintf(buf, " %d", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeUShort(unsigned short d, PhysicalOutStream& stream)
{
  sprintf(buf, " %u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeInt(int d, PhysicalOutStream& stream)
{
  sprintf(buf, " %d", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeUInt(unsigned int d, PhysicalOutStream& stream)
{
  sprintf(buf, " %u", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeFloat(float d, PhysicalOutStream& stream)
{
  sprintf(buf, " %g", double(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeDouble(double d, PhysicalOutStream& stream)
{
  sprintf(buf, " %g", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeAngle(const Angle& d, PhysicalOutStream& stream)
{
  sprintf(buf, " %gdeg", d.toDegrees());
  stream.writeToStream(buf, strlen(buf));
}

void OutText::writeEndL(PhysicalOutStream& stream)
{
  sprintf(buf, "\n");
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeBool(bool value, PhysicalOutStream& stream)
{
  if(value)
    stream.writeToStream("true", 4);
  else
    stream.writeToStream("false", 5);
}

void OutTextRaw::writeString(const char* value, PhysicalOutStream& stream)
{
  stream.writeToStream(value, strlen(value));
}

void OutTextRaw::writeData(const void* p, size_t size, PhysicalOutStream& stream)
{
  for(size_t i = 0; i < size; ++i)
    writeChar(*((const char*&)p)++, stream);
}

void OutTextRaw::writeChar(char d, PhysicalOutStream& stream)
{
  sprintf(buf, "%d", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeSChar(signed char d, PhysicalOutStream& stream)
{
  sprintf(buf, "%u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeUChar(unsigned char d, PhysicalOutStream& stream)
{
  sprintf(buf, "%u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeShort(short d, PhysicalOutStream& stream)
{
  sprintf(buf, "%d", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeUShort(unsigned short d, PhysicalOutStream& stream)
{
  sprintf(buf, "%u", int(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeInt(int d, PhysicalOutStream& stream)
{
  sprintf(buf, "%d", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeUInt(unsigned int d, PhysicalOutStream& stream)
{
  sprintf(buf, "%u", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeFloat(float d, PhysicalOutStream& stream)
{
  sprintf(buf, "%g", double(d));
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeDouble(double d, PhysicalOutStream& stream)
{
  sprintf(buf, "%g", d);
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeAngle(const Angle& d, PhysicalOutStream& stream)
{
  if(d == SensorData::off)
    sprintf(buf, "%g", static_cast<float>(d));
  else
    sprintf(buf, "%gdeg", d.toDegrees());
  stream.writeToStream(buf, strlen(buf));
}

void OutTextRaw::writeEndL(PhysicalOutStream& stream)
{
  sprintf(buf, "\n");
  stream.writeToStream(buf, strlen(buf));
}

OutFile::~OutFile()
{
  if(stream != nullptr)
    delete stream;
}

bool OutFile::exists() const
{
  return stream != nullptr && stream->exists();
}

void OutFile::open(const std::string& name)
{
  stream = new File(name, "wb", false);
}

void OutFile::open(const std::string& name, bool append)
{
  stream = append ? new File(name, "ab", false) : new File(name, "wb", false);
}

void OutFile::writeToStream(const void* p, size_t size)
{
  if(stream != nullptr)
    stream->write(p, size);
}

std::string OutFile::getFullName() const
{
  ASSERT(stream);
  return stream->getFullName();
}

void OutMemory::writeToStream(const void* p, size_t size)
{
  if(memory != nullptr)
  {
    memcpy(memory, p, size);
    memory += size;
    length += size;
  }
}

OutMap::OutMap(Out& stream, bool singleLine) :
  stream(stream), singleLine(singleLine)
{}

void OutMap::writeLn()
{
  if(singleLine)
    stream << " ";
  else
    stream << endl;
}

void OutMap::outUChar(unsigned char value)
{
  Entry& e = stack.back();
  if(e.enumToString)
    stream << e.enumToString(value);
  else
    stream << static_cast<unsigned>(value);
}

void OutMap::outUInt(unsigned int value)
{
  if(stack.back().type != -1)
    stream << value;
}

void OutMap::outString(const char* value)
{
  char buf[2] = {0};
  bool containsSpecialChars = !*value || *value == '"' || strcspn(value, " \n\r\t=,;]}") < strlen(value);
  if(containsSpecialChars)
    stream << "\"";
  for(; *value; ++value)
    if(*value == '"' && containsSpecialChars)
      stream << "\\\"";
    else if(*value == '\n')
      stream << "\\n";
    else if(*value == '\r')
      stream << "\\r";
    else if(*value == '\t')
      stream << "\\t";
    else if(*value == '\\')
      stream << "\\\\";
    else
    {
      buf[0] = *value;
      stream << buf;
    }
  if(containsSpecialChars)
    stream << "\"";
}

void OutMap::select(const char* name, int type, const char* (*enumToString)(int))
{
  Streaming::trimName(name);
  if(!stack.empty())
  {
    ASSERT(name || type >= 0);
    Entry& e = stack.back();
    if(!e.hasSubEntries)
    {
      if(e.type == -1) // array
      {
        stream << "[";
        writeLn();
      }
      else // other attribute or array element
      {
        stream << "{";
        writeLn();
      }
      if(!singleLine)
        indentation += "  ";
      e.hasSubEntries = true;
    }
  }

  if(type < 0) // attribute
  {
    stream << indentation;
    if(name)
      stream << name << " = ";
  }
  else if(type == 0) // first array element
    stream << indentation;
  else if(type > 0) // further array elements
  {
    stream << ",";
    writeLn();
    stream << indentation;
  }

  stack.push_back(Entry(type, enumToString));
}

void OutMap::deselect()
{
  Entry& e = stack.back();
  if(e.hasSubEntries)
  {
    if(!singleLine)
      indentation = indentation.substr(2);
    if(e.type == -1) // array
    {
      writeLn();
      stream << indentation << "]";
    }
    else // other attribute or array element
      stream << indentation << "}";
  }
  else if(e.type == -1) // empty array
    stream << "[]";

  if(e.type < 0) // attribute
  {
    stream << ";";
    writeLn();
  }
  stack.pop_back();
}

void OutMap::write(const void* p, size_t size)
{
  ASSERT(false);
}

OutMapFile::OutMapFile(const std::string& name, bool singleLine) : OutMap(stream, singleLine), stream(name) {}

OutMapMemory::OutMapMemory(void* memory, bool singleLine) : OutMap(stream, singleLine), stream(memory) {}

OutMapSize::OutMapSize(bool singleLine) : OutMap(stream, singleLine) {}
