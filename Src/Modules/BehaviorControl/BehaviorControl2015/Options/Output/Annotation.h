/* 
 * Copyright:Hust_iRobot
 * File:   Annotation.h
 * Author: Shangyunfei
 * Description:
 * Created on 2017?10?1?, ??10:08
 */

#ifndef ANNOTATION_H
#define ANNOTATION_H
std::string lastAnnotationSend;

option(Annotation,(const std::string&) annotation)
{
    initial_state(send)
    {
        transition
        {
            if(state_time)
            goto waitForNewAnnotation;
        }
        action
        {
            ANNOTATION("Behavior", annotation);
            lastAnnotationSend = annotation;
        }
    }
    
    state(waitForNewAnnotation)
    {
        transition
        {
            if(annotation != lastAnnotationSend)
            goto send;
        }
    }
}



#endif /* ANNOTATION_H */

