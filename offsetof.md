# `offsetof` 

<!-- TOC -->

- [`offsetof`](#offsetof)
  - [作用](#作用)
  - [返回值](#返回值)
  - [头文件](#头文件)
  - [代码示例一](#代码示例一)
  - [代码示例二](#代码示例二)

<!-- /TOC -->

## 作用
计算结构体成员相对于结构体开头的偏移量

## 返回值
返回一个 `size_t` 类型的常量

## 头文件
C标准库 `<stddef.h>`，C++标准库 `<cstddef>`

## 代码示例一
```cpp
#include <stddef.h>
#include <stdio.h>

struct address {
   char name[50];
   char street[50];
   int phone;
};
   
int main()
{
   printf("address 结构中的 name 偏移 = %d 字节。\n",
   offsetof(struct address, name));
   
   printf("address 结构中的 street 偏移 = %d 字节。\n",
   offsetof(struct address, street));
   
   printf("address 结构中的 phone 偏移 = %d 字节。\n",
   offsetof(struct address, phone));

   return(0);
} 
```

让我们编译并运行上面的程序，这将产生以下结果：

```cpp
address 结构中的 name 偏移 = 0 字节。
address 结构中的 street 偏移 = 50 字节。
address 结构中的 phone 偏移 = 100 字节。
```

## 代码示例二
```cpp
struct ListNode
{
    ListNode* prev;
    ListNode* next;
    char data[];
};

offsetof(ListNode, data);       // 结构体成员data相对于结构体开头的偏移量 =8
```

```cpp
ListNode* node = (ListNode*)(data - offsetof(ListNode, data));
// data的地址减去data相对于结构体开头的偏移量，即可以得到结构体的地址
```
