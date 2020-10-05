# grammar-of-Cpp

<!-- TOC -->

- [grammar-of-Cpp](#grammar-of-cpp)
  - [索引](#索引)
  - [版本管理](#版本管理)
    - [主线合并分支](#主线合并分支)
    - [分支合并主线](#分支合并主线)
    - [管理本地分支与远程分支](#管理本地分支与远程分支)

<!-- /TOC -->

## 索引
* [仿函数](https://github.com/JINGLONGGIT/grammar-of-c-plus-plus/blob/main/%E4%BB%BF%E5%87%BD%E6%95%B0.md)  
* [`offsetof`]()  
* [柔性数组]()  
* [静态断言 `static_assert`]()  
* [`typename` 关键字]()  

## 版本管理
两台客户机均在分支上进行开发
```vim
git checkout -b <branchname>    // 新建名字为<branchname>的分支，病切换到该分支
```

如果分支<branchname>已存在，则使用如下指令切换到该分支
```vim
git checkout <branchname>
```

查看本地所有分支
```vim
git branch
```

查看本地和远程所有分支
```cpp
git branch -a
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

### 主线合并分支
进入分支，更新分支代码
```cpp
[main]:   git checkout <branchname>       // 进入分支
[branch]: git pull                        // 更新分支代码
```

切换到主线
```cpp
[branch]: git checkout main
```

在主线上合并分支
```cpp
[main]: git merge <branchname>
```

提交合并后的代码
```cpp
[main]: git commit -m "提交信息"
```

将提交推送到远程主线
```cpp
[main]: git push origin main
```

### 分支合并主线
进入主线，更新主线代码
```cpp
[main]: git pull
```

切换分支
```cpp
[main]: git checkout <branchname>
```

在分支上合并主线
```cpp
[branch]: git merge main
```

提交合并后的代码
```cpp
[branch]: git commit -m "提交信息"
```

将提交推送到远端
```cpp
[branch]: git push origin <branchname>
```

### 管理本地分支与远程分支
查看本地所有分支
```cpp
git branch
```

查看本地和远程所有分支
```cpp
git branch -a
```

只查看远程的分支
```cpp
git branch -r
```

删除本地分支
```cpp
git branch -d <branchname>
```

删除远程分支
```cpp
git push origin --delete <branchname>
```

获取远程的所有分支
```cpp
git clone       // 只能clone远程的main分支，无法clone其他分支
```

获取远程的所有分支
```cpp
git clone           // 先git整个项目，此时只有main主线

git branch -a       // 查看远程的所有分支

// 假设远程分支内容如下
remotes/origin/HEAD -> origin/main
remotes/origin/functor
remotes/origin/main

// checkout远程的functor分支，在本地起名为funcotr分支，并切换到本地functor分支
git checkout -b functor origin/functor   
```

git已经删除了远程分支，但是在本地使用 `git branch -a` 仍然可以看到的解决方案
```cpp
git remote show origin    // 查看remote地址，远程分支，还有本地分支与之相对应关系等信息

git remote prune origin   // 在本地删除远程不存在的分支
```