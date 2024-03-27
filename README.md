# 智商网盘

一个简单的网盘，包含客户端和服务端。

总之，能用。

## 功能

- 登陆/注册
- 上传下载（有进度条）
- 添加/删除好友
- 查找用户
- 显示在线用户
- 好友间聊天
- 好友间分享文件
- 移动文件/文件夹
- 删除文件/文件夹
- 重命名

## 简单的说明

数据库需要手动创建对应的数据库和数据表，参考或者直接运行`database.sql`即可。

详细的一些说明请参阅 [这里](introduction.md) 从0开始做的一些笔记。

### 关于配置文件

客户端和服务端的配置文件在`server.config`和`client.config`中，下面是两个配置模板

#### 服务器

```json
{
    "MySQLDatabase": "数据库名",
    "MySQLHost": "MySQL数据库地址",
    "MySQLPassword": "MySQL数据库密码",
    "MySQLPort": MySQL数据库端口,
    "MySQLUsername": "MySQL数据库用户名",
    "ServerIP": "服务器监听IP",
    "ServerPort": 服务器监听端口,
    "UserFilePath": "用户文件保存的路径，会以uid为文件夹分开"
}
```

#### 客户端

```json
{
    "ServerHost": "服务器地址",
    "ServerPort": 端口
}
```

## 感谢

使用了[RemixIcon](https://github.com/Remix-Design/RemixIcon)的部分图标，感谢他们的付出！
