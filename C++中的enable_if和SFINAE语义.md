# C++11 std::enable_if 和 SFINAE

<!-- TOC -->

- [C++11 std::enable_if 和 SFINAE](#c11-stdenable_if-和-sfinae)
  - [enable_if 和 SFINAE 是什么意思](#enable_if-和-sfinae-是什么意思)
  - [代码示例一](#代码示例一)
  - [代码示例二：](#代码示例二)
  - [代码示例三](#代码示例三)
  - [参考链接](#参考链接)

<!-- /TOC -->

## enable_if 和 SFINAE 是什么意思
SFINAE : substitution failure is not an error 替代失败不是错误。  
当调用模板函数时编译器会根据传入的参数推导最合适的模板函数，在这个推导过程中如果某一个或者几个模板函数推导出来的结果是无法编译通过的，但是只要有一个可以正确推导出来，那么那几个推导的可能产生编译错误的模板函数并不会引发编译错误。

## 代码示例一

```cpp
struct Test {
    typedef int foo;
};

template <typename T> 
void f(typename T::foo) {} // Definition #1

template <typename T> 
void f(T) {}               // Definition #2

int main() {
    f<Test>(10); // Call #1.
    f<int>(10);  // Call #2. Without error (even though there is no int::foo) thanks to SFINAE.
}
```

在函数模板的推导过程中，调用 `f<int>(10)` 模板函数会最终匹配到 `void f(T){}`。但是在匹配的过程中，同样也会去推导 `void f(typename T::foo){}`，此时 `int::foo` 是一个语法错误，但是编译器并不会报编译错误。这就是 `SFINAE`

## 代码示例二：
`std::enable_if` 和 `SFINAE` 的结合使用

```cpp
template <typename T>
typename std::enable_if<std::is_trivial<T>::value>::type SFINAE_test(T value)
{
    std::cout<<"T is trival"<<std::endl;
}

template <typename T>
typename std::enable_if<!std::is_trivial<T>::value>::type SFINAE_test(T value)
{
    std::cout<<"T is none trival"<<std::endl;
}
```

这两个函数如果是普通函数的话，根据重载的规则是不会通过编译的（返回值不同不能作为重载）。即便是模板函数，如果这两个函数都能推导出正确的结果，也会产生重载二义性的问题。但正是因为 `enable_if` 和 `SFINAE` 的使用，使得这两个函数的返回值在同一个函数调用的推导过程中只有一个合法，遵循 `SFINAE` 原则，则可以顺利通过编译

```cpp
SFINAE_test(std::string("123"));
SFINAE_test(123);
```

当第一个函数调用进行模板函数推导的时候，第一个版本的模板函数 `std::is_trivial<T>::value` 为 `false`，继而 `std::enable_if<std::is_trivial<T>::value>::type` 这个类型未定义，不能正确推导，编译器区寻找下一个可能的实现  

接下来找到第二个模板函数，`!std::is_trivial<T>::value` 的值是 `true` ，继而 `std::enable_if<std::is_trivial<T>::value>::type` 是 `void` 类型，推导成功。这时候 `SFINAE_test(std::string("123"))` ；调用有了唯一确定的推导，即第二个模板函数，所以程序打印 `T is none trival`。  

与此相似的过程，第二个函数调用打印出 `T is trival`。

## 代码示例三

```cpp
template<bool InlineMode>
ElementType* ToElement(NodeType* ele, typename std::enable_if<InlineMode, int>::type = 0) {
    return ele;
}
```

```cpp
tempalte<bool InlineMode>
ElementType* ToElement(NodeType* ele, typename std::enable_if<!InlineMode, int>::type = 0) {
    return &(ele->element);
}
```

如果 `enable_if` 的第一个模板参数表达式为真，则 `std::enable_if<>::type` 得到一个类型
* 如果 `enable_if` 只有一个模板参数，则 `type` 成员得到的类型是 `void`
* 否则，`type` 成员得到的类型是第二个模板参数的类型

如果 `enable_if` 的第一个模板参数表达式为假，`type` 成员未定义，由于模板有一个功能叫做 `SFINAE`，就会使这个带有 `enable_if` 表达式的模板函数在匹配时被忽略

结合上面的代码，当 `InlineMode` 为真时，匹配第一个函数模板；反之当 `InlineMode` 为假，也就是 `!InlineMode` 为真时，匹配第二个函数模板

## 参考链接
* [std::enable_if](https://blog.csdn.net/JY_95/article/details/102147567)
* [C++11新特性--std::enable_if和SFINAE](https://www.jianshu.com/p/a961c35910d2)
