#ifndef __CAMERA_H
#define __CAMERA_H

#include "time.h"

void Camera_Output(void);
void CameraSynic(stRTC_Time* ptSysTime);
void CameraSynic_clear(void);

#endif //__CAMERA_H

