华中科技大学iRobot团队
===

在ubuntu上使用Git教程
===
#1,在ubuntu上安装git<br>
直接在终端中执行指令``` sudo apt-get install git ```即可进行安装<br>

#2,配置用户名和密码<br>
输入指令```git config --global user.name "xxx"```和 ```git config --global user.eamil ```"你的邮箱地址".

以上两步,即完成了ubuntu上的git安装.若想通过ssh的方式与github进行连接,则还需单独配置公钥密钥.本项目暂不需要,本处略过.



关于本工程:iRobot_Bhuman代码,按照以下方法来进行下载,编译
---
#1,在自己的笔记本上的任意位置建立一个空文件夹如:在桌面上建立一个"Bhuman"文件夹

#2,在新建的“Bhuman”文件夹下，执行：```git init```创建一个空的git repository或重新初始化一个现有的

#3,进到此文件夹下,打开终端,执行:```git clone https://github.com/SkyCloudShang/iRobot_Bhuman.git```
此时,终端会要求输入用户名和密码,输入自己在github的用户名和密码,即可开始从github上下载源代码

#4,下载完成后,即可开始进行编译.编译demo请参阅CodeRelease2016.pdf文档,需要用到的命令可在Tools文件夹中查阅.
其中,2.3.4.1小结中的Alcommon在这份demo中已经安装,不需再单独安装.那步不用管.其余部分都需按照文档执行.

#5,编译完成后,相关的校准等步骤的相关指令也放在Tools文件夹中,供大家查阅.


必会技能!所有成员务必按照以下顺序来更新上传代码!
---
每次在对代码进行了更改之后,先`编译通过`,之后按照如下顺序往github上传:<br>
<br>
#1,为本地repository将要上传的远程repository添加URL，终端执行：```git remote add origin https://github.com/SkyCloudShang/iRobot_Bhuman.git```

#2,在下载下来的工程文件夹Bhuman/iRobot_Bhuman文件夹下,打开终端,先执行```git pull origin master```指令,pull是从服务器上将最新版本的代码拿下来,先更新到最新的版本

#3,确认你的修改在最新版本代码下可以编译通过,此处需要再编译一次,以防止你的修改和别人的修改会冲突.若没有编译,直接上传可能会导致别人下载下来的
代码编译不通过.所以,更新到最新代码后,必须在自己的电脑上再编译一次!!!

#4,以上第2步通过了之后,再在工程文件夹Bhuman/iRobot_Bhuman文件夹下,执行```git add .```指令,这条指令是将你当前文件夹下所有修改过的文件都加到暂存区.在执行这条指令前,
可以先用```git status```指令和```git diff XXX```指令查看当前的代码和服务器上的代码有什么不同,方便知道哪些要add,哪些不用add.

#5,同样的目录下，执行```git commit -m "增加了..."```指令,这条指令中双引号括起来的部分将在github上能直接反应出来.方便让别人知道此次更新,你做了什么事情.
增加了哪些功能.

#6,同样的目录下，执行```git push origin master```指令,最后的一条指令,往服务器上推送代码.执行完,即将代码上传到github.其他团队成员也可见.
