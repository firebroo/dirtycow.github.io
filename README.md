# Usage
这里仅仅是演示,我修改备份的/etc/passwd

```shell
sudo cp /etc/passwd /etc/passwd.bak
./dirtyc0w /etc/passwd.bak
```
查看/etc/passwd.bak当前用户的uid和gid就被改为0了。提权成功

# Dirty COW

Hello

To add a new FAQ entry please send a PR for index.html.

If you wish to learn more, or share what you currently know of the vulnerability head on to the wiki (open to everyone): https://github.com/dirtycow/dirtycow.github.io/wiki

All code, images and documentation in this page and the website is in the public domain ([CC0](https://creativecommons.org/publicdomain/zero/1.0/)).
