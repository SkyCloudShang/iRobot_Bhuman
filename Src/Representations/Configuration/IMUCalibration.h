#pragma once

#include "Tools/Math/Eigen.h"
#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(IMUCalibration,
{,
  (AngleAxisf)(AngleAxisf::Identity()) rotation,
});
