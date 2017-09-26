/**
 * @author Felix Thielke
 */

#include "ColorScanlineRegionizer.h"

MAKE_MODULE(ColorScanlineRegionizer, perception)

void ColorScanlineRegionizer::update(ColorScanlineRegionsVertical& colorScanlineRegionsVertical)
{
  colorScanlineRegionsVertical.scanlines.clear();
  colorScanlineRegionsVertical.scanlines.reserve((theScanGrid.lines.size() + theScanGrid.lowResStep - theScanGrid.lowResStart - 1) / theScanGrid.lowResStep);
  colorScanlineRegionsVertical.lowResStart = 0;
  colorScanlineRegionsVertical.lowResStep = 1;

  if(theScanGrid.lines.empty())
    return;

  for(size_t i = theScanGrid.lowResStart; i < theScanGrid.lines.size(); i += theScanGrid.lowResStep)
  {
    colorScanlineRegionsVertical.scanlines.emplace_back(static_cast<unsigned short>(theScanGrid.lines[i].x));
    scanVertical(theScanGrid.lines[i], theScanGrid.fieldLimit, colorScanlineRegionsVertical.scanlines.back().regions);
  }
}

void ColorScanlineRegionizer::update(ColorScanlineRegionsHorizontal& colorScanlineRegionsHorizontal)
{
  colorScanlineRegionsHorizontal.scanlines.clear();

  if(theScanGrid.lines.empty())
    return;

  int prevY = theECImage.colored.height + minHorizontalScanlineDistance * theECImage.colored.height / 320;
  for(const int y : theScanGrid.y)
  {
    if(prevY - y < (int)minHorizontalScanlineDistance * theECImage.colored.height / 320)
    {
      continue;
    }
    prevY = y;

    colorScanlineRegionsHorizontal.scanlines.emplace_back(y);
    ColorScanlineRegionsHorizontal::Scanline& scanline = colorScanlineRegionsHorizontal.scanlines.back();

    FieldColors::Color curColor;
    unsigned int count = 0;
    int x = 0;

    for(size_t i = theScanGrid.lowResStart; i < theScanGrid.lines.size(); i += theScanGrid.lowResStep)
    {
      const ScanGrid::Line& line = theScanGrid.lines[i];
      if(line.yMax >= y)
      {
        if(count == 0)
        {
          x = line.x;
          count = 1;
          curColor = theECImage.colored[y][x];
        }
        else
        {
          while(x < line.x)
          {
            x++;
            const FieldColors::Color color = theECImage.colored[y][x];
            if(color != curColor)
            {
              if(count >= minHorizontalRegionSize)
              {
                scanline.regions.emplace_back(x - count, x, curColor);
                count = 1;
              }
              else if(!scanline.regions.empty())
              {
                count /= 2;
                scanline.regions.back().range.right += count;
              }
              curColor = color;
            }
            else
            {
              count++;
            }
          }
        }
      }
      else if(count != 0)
      {
        if(count >= minHorizontalRegionSize)
          scanline.regions.emplace_back(x - count, x, curColor);
        else if(!scanline.regions.empty())
          scanline.regions.back().range.right += count;
        count = 0;
      }
    }

    if(count != 0)
    {
      if(count >= minHorizontalRegionSize)
        scanline.regions.emplace_back(x - count, x, curColor);
      else if(!scanline.regions.empty())
        scanline.regions.back().range.right += count;
      count = 0;
    }
  }
}

void ColorScanlineRegionizer::scanVertical(const ScanGrid::Line& line, const int top, std::vector<ScanlineRegion>& regions) const
{
  auto y = theScanGrid.y.begin() + line.yMaxIndex;
  const auto yEnd = theScanGrid.y.end();
  const int width = theECImage.colored.width;
  if(y != yEnd && *y > top && line.yMax - 1 > top)
  {
    int prevY = line.yMax - 1 > *y ? line.yMax - 1 : *y++;
    int currentY = prevY + 1;
    const FieldColors::Color* pImg = &theECImage.colored[prevY][line.x];
    FieldColors::Color currentColor = *pImg;
    for(; y != yEnd && *y > top; ++y)
    {
      pImg += (*y - prevY) * width;

      // If color changes, determine edge position between last and current scanpoint
      const FieldColors::Color& color = *pImg;
      if(color != currentColor)
      {
        const int otherColorThreshold = std::max(static_cast<int>((prevY - *y) * minColorRatio), 1);
        const int yMin = std::max(*y - otherColorThreshold + 1, 0);
        int counter = 0;
        int yy = std::min(prevY - 1, line.yMax - 1);
        for(const FieldColors::Color* pImg2 = pImg + (yy - *y) * width; yy >= yMin && counter < otherColorThreshold; --yy, pImg2 -= width)
          if(*pImg2 != currentColor)
            ++counter;
          else
            counter = 0;

        // Enough pixels of different colors were found: end previous region and start a new one.
        if(counter == otherColorThreshold)
        {
          yy += otherColorThreshold + 1;
          ASSERT(currentY > yy);
          regions.emplace_back(yy, currentY, currentColor);
          currentColor = color;
          currentY = yy;
        }
      }
      prevY = *y;
    }
    ASSERT(currentY > top + 1);
    regions.emplace_back(top + 1, currentY, currentColor);
  }
}
