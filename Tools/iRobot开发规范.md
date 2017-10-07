开发规范
===
在所有的Bhuman代码开发的过程中,所有人需统一使用同一模板进行开发.具体操作方法如下:<br>
<br>
#1,在使用netbeans打开bhuman代码的前提下,点击Tools->Templates->C++,
接下来需要对C++ Header files和C++ Source files分别进行设置.<br>
<br>
#2,首先选择C++ Header files,点击下方的Open in Editor,然后将弹出的文件
统一替换为如下所示:
```C++
/* 
 * Copyright:Hust_iRobot
 * File:   ${NAME}.${EXTENSION}
 * Author: Shangyunfei(替换成自己的名字)
 * Description:
 * Created on ${DATE}, ${TIME}
 */

#ifndef ${GUARD_NAME}
#define	${GUARD_NAME}



#endif	/* ${GUARD_NAME} */
```
其中Description处,留给大家填写此文件定义了什么接口,实现了什么功能.保存后退出,使用NetBeans尝试建立新的C++ Header files,
若建立出的新文件如下图所示,则设置成功.
![image](https://github.com/SkyCloudShang/Pictures/blob/master/C%2B%2Bheaderstd.png)
<br>

#3,接下来需要对C++ Source files进行设置.对C++ source files,按照上面同样的步骤进行如下替换:
```C++
/* 
 * Copyright:Hust_iRobot
 * File:   ${NAME}.${EXTENSION}
 * Author: Shangyunfei(替换成自己的名字)
 * Description:
 * Created on ${DATE}, ${TIME}
 */
 ```
 
同样进行测试的结果若如下图所示,则配置成功.
![image](https://github.com/SkyCloudShang/Pictures/blob/master/C%2B%2Bsourcestd.png)
<br>


