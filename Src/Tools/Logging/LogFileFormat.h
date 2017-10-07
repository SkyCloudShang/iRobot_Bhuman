/**
 * File:   LogFileFormat.h
 * Author: arne
 *
 * Created on September 25, 2013, 2:54 PM
 */

#pragma once

#include "Tools/Streams/Enum.h"

namespace Logging
{
  GLOBAL_ENUM(LogFileFormat,
  {,
    logFileUncompressed,
    logFileCompressed,
    logFileMessageIDs,
    logFileStreamSpecification,
  });
}
