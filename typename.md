# typename 关键字

<!-- TOC -->

- [typename 关键字](#typename-关键字)
  - [文档说明](#文档说明)
  - [参考链接](#参考链接)
  - [`typename` 的使用场合](#typename-的使用场合)
    - [模板定义中的 `typename`](#模板定义中的-typename)
    - [标明**内嵌依赖类型名**](#标明内嵌依赖类型名)
  - [`typename` 在内嵌依赖类型中的应用](#typename-在内嵌依赖类型中的应用)
    - [类作用域](#类作用域)
    - [引入 `typename` 的真实原因](#引入-typename-的真实原因)
  - [使用 `typename` 的规则](#使用-typename-的规则)

<!-- /TOC -->

## 文档说明


## 参考链接
* [C++中typename关键字的使用方法和注意事项](https://blog.csdn.net/vanturman/article/details/80269081)

## `typename` 的使用场合
### 模板定义中的 `typename`
用在模板定义里，标明其后的模板参数是类型参数。可以和 `class` 混合使用，二者功能完全相同。

```cpp
template<typename T>
T foo(const T& t) {
    // do something
}

等价于

template<class T>
T foo(const T& t) {
    // do something
}
```

### 标明**内嵌依赖类型名**

```cpp
class MyArray 
{ 
public：
typedef int LengthType;
.....
}

template<class T>
void MyMethod( T myarr ) 
{ 
typedef typename T::LengthType LengthType; 
LengthType length = myarr.GetLength; 
}
```

此处 `typename` 的作用就是告诉C++编译器，`typename` 后面的字符串 `T::LengthType` 是一个类型名称，而不是成员函数或者成员变量。  
如果前面没有 `typename`，编译器没有办法知道 `T::LengthType` 是一个类型还是成员名称（静态数据成员或者静态函数），所以编译无法通过。  

`ArrayList` 模块源码中有多处使用了 `typename` 关键字。比如  

```cpp
using InterLockGuard = typename std::conditional<is_thread_safe, LockGuard, EmptyLockGuard>::type;
```

在这里，`typename` 关键字的含义是指出后面的 `std::conditional<is_thread_safe, LockGuard, EmptyLockGuard>::type` 是类型名而不是变量  

```cpp
static_assert(std::is_same<typename traits::return_tye, bool>::value && traits::args_count == 1 && std::is_same<typename traits::template argument<0>::type, const T>::value, "handler must be a bool(const T&) functor");
```

同样的，使用 `typename` 指出 `return_type` 和 `type` 都是类型名而不是变量

## `typename` 在内嵌依赖类型中的应用
### 类作用域
我们首先需要回顾一个概念：类作用域。在类的外部访问类的成员名称的时候，可以使用类作用域操作符，形如 `MyClass::name` 的调用通常有三种类型：静态数据成员、静态函数成员和嵌套类型

```cpp
struct MyClass 
{
    static int A;               // 静态数据成员
    static int B();             // 静态函数成员
    typedef int C;              // 嵌套类型
};
```

我们可以使用 `MyClass::A`、`MyClass::B`、`MyClass::C` 来调用以上三种类型

### 引入 `typename` 的真实原因

假如我们有一个这样的函数模板

```cpp
template<class T>
void foo() {
    T::iterator *iter;
}
```

这个函数模板的含义就是定义了一个指针 `iter`，它指向的类型是包含在类 `T` 作用域中的 `iterator`。类 `T` 中可能有这样的一个结构

```cpp
struct ContainsAType
{
    struct iterator
    {
        // do something
    };
};
```

`iterator` 是嵌套类型，所以我们可以按照 `ContainsAType::iterator` 的方式去调用。那么 `iter` 就是一个  `ContainsAType::iterator` 类型的指针

但是，如果类 `T` 是这样的一种结构:

```cpp
struct ContainAType
{
    static int iterator;
};
```

`iterator` 是 `T` 中的一个静态数据成员。我们可以按照 `ContainAType::iterator` 的方式去调用，此时我们实例化 `foo`

```cpp
foo<ContainAType>();
```

那么，`T::iterator *iter` 会被实例化为 `ContainAType::iterator *iter`。  
这里，`iterator` 是一个静态成员变量，编译器将 `ContainAType::iterator *iter` 解释为乘法表达式。但是 `iter` 没有定义，所以编译报错

```cpp
error C2065: ‘iter’ : undeclared identifier
```

而对于上述问题，C++标准委员会就决定引入一个新的关键字 `typename`
> 对于用于模板定义的依赖于模板参数的名称，只有在实例化的参数中存在这个类型名，或者这个名称前使用了typename关键字来修饰，编译器才会将该名称当成是类型。除了以上这两种情况，绝不会被当成是类型

因此，如果你想告诉编译器 `T::iterator` 是类型，而不是变量，只需要用 `typename` 来修饰

```cpp
template<class T>
T foo() {
    typename T::iterator *iter;
}
```

再比如下面的例子

```cpp
template<class T>
void foo() {
    typedef typename T::iterator iterator_type;
}
```

编译器将上述代码解释为**给模板类 `T` 中的嵌套类型 `iterator` 起一个 `iterator_type` 的别名**

## 使用 `typename` 的规则
* 如果类型是依赖于模板参数的限定名，那么在它之前就必须加上 `typename`（除非是基类列表，或者在类的初始化成员列表中）
* 在其他情况下 `typename` 是可选的，也就是说对于一个不依赖于模板参数的限定名，该名称是可选的。例如 `vector<int>`
* 在以下情况禁止使用 `typename`
  * 模板定义之外。即 `typename` 只能用于模板的定义中
  * 非限定类型。比如前面介绍过的 `int`，`vector<int>` 之类
  * 基类列表中。比如 `template <class T> class C1 : T::InnerType` 不能在 `T::InnerType` 前面加 `typename` 
  * 构造函数的初始化列表中