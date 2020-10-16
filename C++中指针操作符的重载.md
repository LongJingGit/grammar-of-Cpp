# C++ 中 * 和 -> 运算符的重载

<!-- TOC -->

- [C++ 中 * 和 -> 运算符的重载](#c-中--和---运算符的重载)
  - [代码示例一](#代码示例一)
  - [代码示例二](#代码示例二)

<!-- /TOC -->

## 代码示例一

```cpp
class CPerson {
public:
    CPerson(int param) {
        m_param = param;
    }
    void PrintPerson () {
        cout << "param = " << m_param << end;
    }

private:
    int m_param;
};

class CSmartPointer {
public:
    CSmartPointer (CPerson* person) {
        this->m_person = person;
    }

    ~CSmartPointer () {
        if (m_person != NULL) {
            delete m_person;
        }
    }

    CPerson& operator*() {
        return *m_person;
    }

    CPerson* operator->() {
        return m_person;
    }

private:
    CPerson* m_person;
};

void test() {
//    CPerson* person = new CPerson(100);     // 如果不释放会造成内存泄漏
//    person->PrintPerson();

    SmartPointer pointer(new CPerson(100));   // 局部变量离开作用域自动调用析构函数，从而释放内存
    pointer->PrintPerson();
    // 由于 `SmartPointer` 重载了 `->` 运算符，所以调用 `pointer->` 之后返回了 `m_person`。
    // 由于 `m_person` 是指针，所以可以直接使用 `m_person->PrintPerson()` 访问它的成员函数
}
```

## 代码示例二

```cpp
#include <iostream>
using namespace std;

template<typename T>
class MyPtr {
public:
    MyPtr(T* ptr = NULL) : m_ptr(ptr) {}

    T& operator*() const {
        return *m_ptr;
    }

    T* operator->() {
        return m_ptr;
    }

private:
    T* m_ptr;    
};

struct Alienware {
    void show() {
        cout << "I'm Alienware" << endl;
    }
};

int main() {
    MyPtr<Alienware> m(new Alienware());
    
    m->show();      // 相当于执行 m.operator->()->show();
    // 因为重载了 `->` 运算符，所以当执行 `m->` 的时候，返回值为 `Alienware* m_ptr`
    // 可以通过 `m_ptr->show()` 调用它本身的成员函数

    (*m).show();    // 相当于执行 m.operator*().show();
    // 因为重载了 `*` 运算符，所以当执行 `(*m)` 的时候，返回值为 `Alienware& m_ptr`
    // 于是通过 `m_ptr.show()` 来调用它本身的成员函数

    // `*/->` 运算符重载再理解：
    // `*`：  解引用运算符，解引用指针，得到指向的内容。返回值为指针指向的内容
    // `->`： 通过指针变量访问结构体的成员。返回值为结构体的某个成员

    return 0;
}
```
