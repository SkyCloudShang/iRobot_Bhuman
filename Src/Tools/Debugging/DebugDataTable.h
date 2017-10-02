/**
 * @file Tools/Debugging/DebugDataTable.h
 * This file declares a table for generic handling of streamable debug data.
 * Representations mentioned in the table will be overwritten with the table
 * entry.
 *
 * @author Michael Spranger
 * @author Tobias Oberlies
 * @author Thomas Röfer
 */

#pragma once

#include "Tools/Streams/InStreams.h"
#include <string>

class InMessage;

/**
 * @class DebugDataTable
 *
 * A singleton class that maintains the debug data table.
 */
class DebugDataTable
{
private:
  std::unordered_map<std::string, char*> table;

  friend class Process; /**< A process is allowed to create the instance. */

  /**
   * No other instance of this class is allowed except the one accessible via getDebugDataTable
   * therefore the constructor is private.
   */
  DebugDataTable() = default;
  DebugDataTable(const DebugDataTable&) = delete;

public:
  ~DebugDataTable();

  /**
   * Registers the object with the debug data table and updates the object if the
   * respective entry in the table has been modified through RobotControl.
   */
  template<class T> void updateObject(const char* name, T& t, bool once);
  void processChangeRequest(InMessage& in);
};

template<class T> void DebugDataTable::updateObject(const char* name, T& t, bool once)
{
  // Find entry in debug data table
  std::unordered_map<std::string, char*>::iterator iter = table.find(name);
  if(iter != table.end())
  {
    InBinaryMemory stream(iter->second);
    stream >> t;
    if(once)
    {
      delete[] iter->second;
      table.erase(iter);
    }
  }
}