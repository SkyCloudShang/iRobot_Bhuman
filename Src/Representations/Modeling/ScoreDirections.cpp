/* 
 * Copyright:Hust_iRobot
 * File:   ScoreDirections.cpp
 * Author: Shangyunfei
 * Description:
 * Created on October 6, 2017, 10:15 AM
 */

#include "ScoreDirections.h"
#include "Tools/Debugging/DebugDrawings.h"

ScoreDirections::Sector::Sector(const Vector2f& leftLimit, const Vector2f& rightLimit)
:leftLimit(leftLimit),rightLimit(rightLimit){}

void ScoreDirections::draw() const
{
    DECLARE_DEBUG_DRAWING("representation:ScoreDirections","drawingOnField");
    {
        Drawings::PenStyle style=Drawings::PenStyle::solidPen;
        for(const Sector&sector:sectors)
        {
            LINE("representation:ScoreDirections",sector.leftLimit.x(),sector.leftLimit.y(),
                    sector.rightLimit.x(),sector.rightLimit.y(),50,style,ColorRGBA::white);
            style=Drawings::PenStyle::dottedPen;
        }
    }
}

