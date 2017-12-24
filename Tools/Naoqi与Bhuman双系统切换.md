# 1,问题来源</br>
   由于Bhuman系统是独立于Naoqi系统的一个系统，因此在对自己Nao机器人安装了Bhuman系统后，是不会影响到原先的Naoqi系统的。只是在安装了
Bhuman系统之后，系统的启动加载文件autoload.ini文件被修改成了Bhuman写的autoload.ini文件，因此每次开机之后，机器人默认进入了Bhuman系统环境。
这对大家再次使用Naoqi系统带来了很大的不便。但是，有一种途径是可以很方便的解决这个问题的。

# 2,解决方案</br>
   在启动了Nao机器人之后，使用```ssh 192.168.1.111 -l nao```命令远程登录上Nao机器人(此处密码为nao)之后，进入bin文件夹下，首先
使用./su命令获取系统的root权限(此处密码为root)，然后运行./tonaoqi命令，最后执行./reboot命令，机器人会重启，之后就进入了Naoqi系统环境。同样
的道理，若想从naoqi系统进入bhuman系统，有一个./tobhuman命令可以完成此目的。

# 3,原理</br>
   以上使用的./tonaoqi命令和./tobhuman命令使用的办法都是将之前保存的ini文件复制到系统的autoload.ini文件。以此来切换开机启动时加载的
动态库文件，以实现不同系统环境间的切换。
