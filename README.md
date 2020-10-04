# grammar-of-c-plus-plus
C/C++语言法

## 版本管理
两台客户机均在分支上进行开发
```vim
git checkout -b <branchname>    // 新建名字为<branchname>的分支，病切换到该分支
```

如果分支<branchname>已存在，则使用如下指令切换到该分支
```vim
git checkout <branchname>
```

查看分支
```vim
git branch
```

提交到远程分支
```vim
git add <修改的文件>
git commit -m "代码提交信息"
git push origin <branchname>
```

合并到主分支
```vim
git checkout main       // 切换到主分支
git merge <branchname>  // 将<branchname>的内容合并到主分支
```

将合并后的主分支提交到远程
```vim
git push origin main
```

合并后删除分支信息
```vim
git branch -d <branchnames>
```

更新远端仓库到本地
```vim
git pull
```