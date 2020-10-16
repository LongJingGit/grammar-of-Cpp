# C++ 中内存操作运算符

<!-- TOC -->

- [C++ 中内存操作运算符](#c-中内存操作运算符)
  - [new operator](#new-operator)
  - [operator new](#operator-new)
  - [placement new](#placement-new)
  - [new、operator new 和 placement new 区别](#newoperator-new-和-placement-new-区别)
  - [placement new 存在的理由](#placement-new-存在的理由)
    - [用 `placement new` 解决 `buffer` 的问题](#用-placement-new-解决-buffer-的问题)
    - [增大时空效率的问题](#增大时空效率的问题)
  - [placement new 的使用步骤](#placement-new-的使用步骤)
  - [placement delete](#placement-delete)
- [参考链接](#参考链接)

<!-- /TOC -->

## new operator
`new operator` / `delete operator` 就是 `new` / `delete` 操作符，不能被重载。

我们按照如下方式使用 `new operator` 时：

```cpp
string* sp = new string(“hello world”);
```

上面代码实际上会执行三个步骤：

```cpp
// 1：调用 operator new 函数申请原始空间
void *raw = operator new(strlen("hello world"));

// 2：通过 placement new 调用 string 类的构造函数，初始化申请空间
new raw string("hello world");

// 3：返回对象指针
string* sp = static_cast<string*>(raw);
```

事实上，内存分配是由 `operator new(size_t)` 来完成的。如果类 `string` 重载了 `operator new` ，那么将调用 `string::operator new(size_t)`，否则会调用全局的 `::operator new(size_t)`，后者由C++默认提供

## operator new
`operator new` 是函数，只分配所要求的空间。有如下三种形式：

```cpp
// 分配 `size` 个字节的存储空间，并将对象类型进行内存对齐。
// 如果成功，返回一个非空的指针指向首地址。失败则抛出 `bad_alloc` 异常
void* operator new (std::size_t size) throw (std::bad_alloc);
```

```cpp
// 分配空间失败时不抛出异常，而是返回一个指向 `NULL` 的空指针
void* operator new (std::size_t size, const std::nothrow_t& nothrow_constant) throw();
```

以上两种 `operator new` 都不会调用对象的构造函数，并且可以被重载。需要注意的是，`operator new` 分为全局成员和类成员，使用 `new` 操作符产生类对象时调用的是其成员函数的 `operator new()`。如果重载了 `operator new`，那么同时需要重载 `opertor delete`

`new operator` 与 `delete operator` 的行为是不能够也是不应该被改变的，这是 C++ 标准作出的承诺。而 `operator new` 和 `operator delete` 和 C 语言中的 `malloc` 与 `free` 对应，只负责分配及释放空间。但是 `operator new` 分配的空间必须用 `operator delete` 来释放，而不能使用 `free`，因为它们对内存使用的登记方式不同。反过来也是一样。你可以重载 `operator new` 和 `operator delete` 以实现对内存管理的不同要求，但你不能重载 `new operator` 或 `delete operator` 以改变它们的行为

为什么有必要写自己的 `operator new` 和 `operator delete`？  
答案通常是：**为了效率**。缺省的 `operator new` 和 `operator delete` 具有非常好的通用性，它的这种灵活性也使得在某些特定的场合下，可以进一步改善它的性能。尤其在那些需要动态分配大量的但很小的对象的应用程序里，情况更是如此。具体可参考《Effective C++》中的第二章内存管理

```cpp
// placement new 版本。本质上是对 operator new 的重载。
// 不分配内存，而是调用合适的构造函数在指针 ptr 所指的地方构造一个对象，之后返回实参指针 ptr
void* operator new (std::size_t size, void* ptr) throw();
```

`operator new` 代码示例

```cpp
#include <iostream>
#include <string>
using namespace std;

class X
{
public:
    X() {
        cout << "X's constructor" << endl;
    }
    ~X() {
        cout << "X's destructor" << endl;
    }

    void* operator new(size_t size, string str) {
        cout << "operator new size " << size << " with string " << str << endl;
        return ::operator new(size);
    }
 
    void operator delete(void* pointer) {
        cout << "operator delete" << endl;
        ::operator delete(pointer);
    }
private:
    int num;
};
 
int main() {
    X *p = new("A new class") X;
    delete p;
    getchar();
    return 0;
}
```

## placement new
一般来说，使用 `new` 申请空间时，是从系统的**堆**（heap）中分配空间。申请所得的空间的位置是根据当时的内存的实际使用情况决定的。但是，在某些特殊情况下，可能需要在已分配的特定内存创建对象，这就是所谓的**定位放置new**（`placement new`）操作

`placement new` 是重载 `operator new` 的一个标准的、全局的版本，它不能够被自定义的版本替代（不像普通版本的 `operator new` 和 `operator delete` 能够被替换）

```cpp
void *operator new( size_t, void *ptr ) throw() { return ptr; }
```

`placement new` 的执行忽略了 `size_t` 参数，只返回第二个参数。其结果是允许用户把一个对象放到一个特定的地方，达到调用构造函数的效果。

括号里的参数 `ptr` 是一个指针，它指向一个内存缓冲器，`placement new` 将在这个内存缓冲器上分配一个对象。`placement new` 是这个被构造对象的地址。

`placement new` 主要适用于：
1. 在对时间要求非常高的应用程序中，因为这些程序分配的时间是确定的；
2. 长时间运行而不被打断的程序；
3. 执行一个垃圾收集器 (garbage collector)。

代码示例如下：

```cpp
#include <iostream>
using namespace std;

class A
{
private:    
    int num;

public:
    A() {
        cout << "A's constructor" << endl;
    }

    ~A() {
        cout << "A's destructor" << endl;
    }
    
    void show() {
        cout << "num:" << num << endl;
    }
};

int main() {
    char mem[100];
    mem[0]='A';
    mem[1]='\0';
    mem[2]='\0';
    mem[3]='\0';
    cout << (void*)mem << endl;
    A* p = new (mem) A;
    cout << p << endl;
    p->show();
    p->~A();
}
```

程序的运行结果是

```
0024F924
A's constructor
0024F924
num:65
A's destructor
```

阅读以上程序，注意以下几点：  
1. 用定位放置 `new` 操作，既可以在栈 `stack` 上生成对象，也可以在堆 `heap` 上生成对象。如本例就是在栈上生成一个对象。
2. 使用语句 `A* p = new (mem) A;` 定位生成对象时，指针 `p` 和数组名 `mem` 指向同一片存储区。所以，与其说定位放置 `new` 操作是申请空间，还不如说是利用已经请好的空间，真正的申请空间的工作是在此之前完成的。
3. 使用语句 `A *p = new (mem) A;` 定位生成对象时，会自动调用类 `A` 的构造函数，但是由于对象的空间不会自动释放（对象实际上是借用别人的空间），所以必须显示的调用类的析构函数，如本例中的 `p->~A()`。
4. 如果有这样一个场景，我们需要大量的申请一块类似的内存空间，然后又释放掉，比如在在一个 `server` 中对于客户端的请求，每个客户端的每一次上行数据我们都需要为此申请一块内存，当我们处理完请求给客户端下行回复时释放掉该内存，表面上看者符合 C++ 的内存管理要求，没有什么错误，但是仔细想想很不合理，为什么我们每个请求都要重新申请一块内存呢，要知道每一次内存的申请，系统都要在内存中找到一块合适大小的连续的内存空间，这个过程是很慢的（相对而言)，极端情况下，如果当前系统中有大量的内存碎片，并且我们申请的空间很大，甚至有可能失败。此时我们可以使用 `placement new` 来构造对象，那么就会在我们指定的内存空间中构造对象。

下面是一个在堆上生成对象的例子：

```cpp
#include <iostream>
using namespace std;
 
class B
{
public:
    B() {
        cout<<"B's constructor"<<endl;
    }
 
    ~B() {
        cout<<"B's destructor"<<endl;
    }
 
    void SetNum(int n) {
        num = n;
    }
 
    int GetNum() {
        return num;
    }

private:
    int num;
};
 
int main() {
    char* mem = new char[10 * sizeof(B)];
    cout << (void*)mem << endl;
    B *p = new (mem) B;
    cout << p << endl;
    p->SetNum(10);
    cout << p->GetNum() << endl;
    p->~B();
    delete[] mem;
    getchar();
}
```

`placement new` 允许我们将对象创建在已经申请好的内存空间中，但是没有所谓的 `placement delete`，因为根本没有申请内存，所以就没必要 `placement delete`    

## new、operator new 和 placement new 区别
1. `new` ：是一个操作符，不能被重载，其行为总是一致的。它先调用 `operator new` 分配内存，然后调用构造函数初始化那段内存。`new` 操作符的执行过程：
   1. 调用 `operator new` 分配内存
   2. 调用构造函数生成类对象
   3. 返回相应的指针
2. `operator new`：要实现不同的内存分配行为，应该重载 `operator new`，而不是 `new`。`operator new` 就像 `operator +` 一样，是可以重载的。如果类中没有重载 `operator new`，那么调用的就是全局的 `::operator new` 来完成堆的分配。同理，`operator new[]`、 `operator delete`、 `operator delete[]` 也是可以重载的
3. `placement new`：只是 `operator new` 重载的一个版本。它并不分配内存，只是返回指向已经分配好的某段内存的一个指针。因此不能删除它，但需要调用对象的析构函数。如果你想在已经分配的内存中创建一个对象，使用 `new` 是行不通的。也就是说 `placement new` 允许你在一个已经分配好的内存中（栈或者堆中）构造一个新的对象。原型中 `void* p` 实际上就是指向一个已经分配好的内存缓冲区的的首地址

## placement new 存在的理由
### 用 `placement new` 解决 `buffer` 的问题
用 `new` 分配的数组缓冲时，由于调用了默认构造函数，因此执行效率上不佳。若没有默认构造函数则会发生编译错误。如果你想在预分配的内存上创建对象，用缺省的 `new` 操作符是行不通的。要解决这个问题，可以用 `placement new` 构造，它允许你构造一个新对象到预分配的内存上

### 增大时空效率的问题
使用 `new` 操作符分配内存需要在堆中查找足够大的剩余空间，显然这个操作速度是很慢的，而且有可能出现无法分配内存的异常（空间不够）。 `placement new` 就可以解决这个问题。我们构造对象都是在一个预先准备好了的内存缓冲区中进行，不需要查找内存，内存分配的时间是常数；而且不会出现在程序运行中途出现内存不足的异常。所以，`placement new` 非常适合那些对时间要求比较高，长时间运行不希望被打断的应用程序

## placement new 的使用步骤
1. 提前分配缓存  
有三种分配内存的方式：

    1）在堆上分配缓存
   
    ```cpp
    class Task;
    char *buff = new[sizeof(Task)];
    ```

   2）在栈上分配缓存

    ```cpp
    class Task;
    char buff[N * sizeof(Task)];
    ```

   3）直接通过地址来使用

    ```cpp
    void *buf = reinterpret_cast<void*> (0xF00F);
    ```

2. 对象的分配  
在刚才已分配的缓存区调用 `placement new` 来构造一个对象
  
    ```cpp
    Task *ptask = new (buf) Task
   ```

3. 使用  
按照普通的方式使用分配的对象

    ```cpp
    ptask->memberfunction();
    ptask-> member;
    ```

4. 对象的析构  
一旦你使用完这个对象，你必须调用它的析构函数来毁灭它

    ```cpp
    ptask->~Task(); // 调用外在的析构函数
    ```

5. 释放缓存  
你可以反复利用缓存并给它分配一个新的对象（重复步骤2，3，4）。如果你不打算使用这个缓存，你可以释放掉它

    ```cpp
    delete[] buff;
    ```
> 跳过任何步骤就可能导致运行时间的崩溃，内存泄露，以及其它的意想不到的情况。如果你确实需要使用 `placement new`，请认真遵循以上的步骤

## delete operator
和 `new operator` 关键字一样， `delete operator` 是 C++ 保留关键词，是操作符，无法重载，无法改变其含义。

调用 `delete operator` 释放对象时完成了两项工作：
* 调用对象的析构函数
* 调用 `operator delete()` 函数释放内存空间

比如我们按照如下方式调用 `delete`

```cpp
string* sp = new string(“hello world”);
delete sp;
```

等价于

```cpp
ps->~string();          // 用于清理内存数据，对应placement new()
operator delete(ps);    // 释放内存空间，对应于operator new()
```

## placement delete 
`placement new()` 是 `operator new()` 附带额外参数的重载版本，对应的，`placement delete()` 即 `operator delete()` 的重载版本。

默认实现版本如下：

```cpp
void operator delete  (void*, void*) noexcept { }
```

默认 `placement delete()` 的额外参数是 `void*`，为空实现，什么也不做。当然，我们可以定义其它附带类型的重载版本

使用默认版本 `placement new()` 与重载 `placement delete()` 来演示定位构造对象和析构对象

```cpp
#include <iostream>
using namespace std;

class A
{
private:
    int num;
public:
    A() {
        cout << "A's constructor" << endl;
    }

    ~A() {
        cout << "A's destructor" << endl;
    }

    void show() {
        cout << "num:" << num << endl;
    }

    // 自定义 placement delete()
    static void operator delete(void* a, void* b) {
        A*  pA = (A*)a;
        pA->~A();
    }
};

int main() {
    char mem[100];
    mem[0] = 'A';
    mem[1] = '\0';
    mem[2] = '\0';
    mem[3] = '\0';
    cout << (void*)mem << endl;
    A* p = new (mem) A;                 // 调用placement new()，间接调用类A的构造函数，构造类A对象
    cout << p << endl;
    p->show();
    A::operator delete(p,(void*)NULL);  // 调用placement delete(),间接调用类A的析构函数，释放类A对象
}
```

程序运行结果如下：

```
000000000014F8E0
A's constructor
000000000014F8E0
num:65
A's destructor
```

需要说明的是：
1. C++ 标准中默认版本的 `placement delete()` 为空实现，不调用类型对象析构函数
2. C++ 中 `placement delete()` 的调用没有像 `placement new()` 的调用有单独的语法格式（`placement new expression`），需要以函数调用的书写格式来调用
3. 上面对 `placement delete()` 的重载只是实现了调用类对象析构函数的功能，实际通过类对象直接调用析构函数即可，<font color=red>那 placement delete() 存在的意义是什么呢？</font>   
    
    对于自定义的 `placement new`，运行期系统将会寻找参数个数和类型都与 `operator new` 相同的某个 `operator delete` 来进行善后工作。  
    
    因此，对于一个带有额外参数的 `operator new` 没有带有相同额外参数的对应版本 `operator delete`，那么 `new` 的内存分配动作需要取消来恢复原状的时候，就不会有任何的 `operator delete` 被调用。  
    
    也就是说，当调用 `placement new()` 构建对象时，如果在构造函数中抛出异常，C++ 只能调用相应的 `placement delete()` 释放由 `placement new()` 获取的内存资源，否则就会有**内存泄露**。

代码示例如下：

```cpp
#include <iostream>
using namespace std;

struct A {};    // 临时中间对象
struct E {};    // 异常对象

class T 
{
public:
    T() { 
        std::cout << "T's constructor" << endl;
        throw E(); 
    }
};

// placement new
void* operator new (std::size_t, const A& a) {
    std::cout << "Placement new called." << std::endl;
    return (void*)&a;
}

// placement delete
void operator delete (void *, const A &) {
    std::cout << "Placement delete called." << std::endl;
}

int main() {
    A a;
    try 
    {
        T* p = new(a) T;
    }
    catch (E exp)
    { 
        std::cout << "Exception caught." << std::endl;
    }
    return 0;
}
```

程序输出结果：

```
Placement new called.
T's constructor
Placement delete called.
Exception caught.
```

当在 `class T` 构造函数中抛出异常时，对应版本的 `placement delete()` 将被调用。

# 参考链接
* [C++ new 的三种面貌](https://blog.csdn.net/K346K346/article/details/45489913)
* [C++中的new、operator new与placement new](https://www.cnblogs.com/luxiaoxun/archive/2012/08/10/2631812.html)
* [C++ delete的三种面貌](https://blog.csdn.net/k346k346/article/details/49514063)
* [C++中使用placement new](https://blog.csdn.net/caoshangpa/article/details/78876443?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-3.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-3.channel_param)
* [《Effective C++》 条款52]()