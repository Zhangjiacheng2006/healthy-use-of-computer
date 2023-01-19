# 健康使用电脑
> 模仿手机的健康使用手机功能，制作的程序
## 目前只支持部分Windows系统
## 功能
1. 获取每日电脑使用时间
2. 设置停用时间
3. 设置应用限额/可用时长
4. 设置应用白名单
5. 设置密码
## 安装方法

本程序分为两部分，time-monitor 与 using-stat。

- time-monitor.exe 用来在后台观测程序使用情况
- using-stat.exe 用来和使用者互动

常规使用下载 [time-monitor.exe](https://github.com/Zhangjiacheng2006/healthy-use-of-computer/releases/download/v0.3.0-alpha/time-monitor.exe) 与 [using-stat.exe](https://github.com/Zhangjiacheng2006/healthy-use-of-computer/releases/download/v0.3.0-alpha/using-stat.exe) 即可

可以将 time-monitor.exe 放置到菜单 C:\Users\...\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup 中以便开机自启动

请确保 using-stat.exe 与 help.txt 在同一文件夹下

编译 time-monitor.cpp ：
`g++ healthy-use-of-computer \time-monitor-3.0.cpp -o time-monitor.exe -lgdi32`

编译 using-stat.cpp ：
`g++ healthy-use-of-computer \using-stat-2.0.cpp -o using-stat.exe`

## 备注

using-stat.exe 使用方法见 help.txt

对应用（字符串s）的规则会应用到每个标题中含有s的窗口上
