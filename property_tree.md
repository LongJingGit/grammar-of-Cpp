# boost::property_tree::ptree

<!-- TOC -->

- [boost::property_tree::ptree](#boostproperty_treeptree)
  - [参考链接](#参考链接)
  - [property_tree::ptree 接口说明](#property_treeptree-接口说明)
    - [`get<T>(path)`](#gettpath)
    - [`get_optional<T>(path)`](#get_optionaltpath)
    - [`get_child(path)`](#get_childpath)
    - [`get_child_optional(path)`](#get_child_optionalpath)
    - [`get_value<T>(path)`](#get_valuetpath)
    - [`bool empty()`](#bool-empty)
    - [`assoc_iterator find(const key_type &key)`](#assoc_iterator-findconst-key_type-key)
    - [`size_type count(const key_type &key)`](#size_type-countconst-key_type-key)
  - [读写 XML 文件](#读写-xml-文件)
    - [XML文件示例](#xml文件示例)
    - [XML文档结构说明](#xml文档结构说明)
    - [代码示例](#代码示例)
  - [读写 JSON 文件](#读写-json-文件)
    - [JOSN 文件格式说明](#josn-文件格式说明)
    - [JSON文件示例](#json文件示例)
    - [代码示例](#代码示例-1)

<!-- /TOC -->

## 参考链接
* [boost.property_tree解析xml的帮助类以及中文解析问题的解决](https://www.cnblogs.com/qicosmos/p/3555668.html)
* [Boost.PropertyTree 在属性树里怎么访问数据](https://www.cnblogs.com/xujintao/p/8325941.html)
* [boost-使用property_tree来解析xml、json](https://www.cnblogs.com/milanleon/p/7422858.html)
* [BOOST之property_tree 及解析XML详解](https://blog.csdn.net/stelalala/article/details/8967812)
* [C++ Boost JSON解析库的使用与坑](https://www.colabug.com/2018/0622/3274456/)

## property_tree::ptree 接口说明
### `get<T>(path)`
获取节点 `path` 的属性值。这个函数相当于先执行 `get_child()` ，得到想要的节点，然后再执行 `get_value<T>()`。
比如 `ptree.get("a.b")` 等价于 `ptree.get_child("a.b").get_value()`  

该接口无法用来获取注释节点的内容。对于注释节点，使用 `data()` 或者 `get_value<>()` 来获取注释内容  

如果 `path` 不存在，直接抛出异常

### `get_optional<T>(path)`
获取节点 `path` 的属性值。如果 `path` 不存在，不会抛出异常，由用户代码判断异常情况

### `get_child(path)`
获取节点 `path` 下的所有子节点（包括属性、注释、数据）。如果路径不存在，直接抛出异常

### `get_child_optional(path)`
获取 `path` 节点下的所有子节点。如果路径不存在，不抛出异常

### `get_value<T>(path)` 
获取节点 `path` 的值，并转换成期望的数据类型 `T`。如果不希望有任何转换，也可以使用 `data()` 

### `bool empty()`
返回该节点是否含有子节点。比如当一个节点已经是叶子节点的时候，可以用这个函数来判断

### `assoc_iterator find(const key_type &key)`
给定一个名字路径，返回指向该节点的迭代器或者 boost::property_tree::ptree::not_found

### `size_type count(const key_type &key)`
返回指定名称路径的节点的子节点的数目

## 读写 XML 文件
### XML文件示例
有如下 `XML` 文件

```xml
<config>
    <file title="windows" size="10Mb">
    <!-- File Fisrt Comment -->
    <!-- File Second Comment -->
        <paths attr="directory">
        <!-- Paths Comment -->
            <pathname title="123">abc</pathname>
            <pathname title="456">efg</pathname>
            <pathname title="789">hij</pathname>
        </paths>
        <paths>
            <pathname title="111">klm</pathname>
            <pathname title="222">nop</pathname>
            <pathname title="333">qrs</pathname>
        </paths>
    </file>
</config>
```

### XML文档结构说明
对于该 XML 文档来说，`config` 是根节点，下面有一个 `file` 子节点，该 `file` 节点的内容有：
* 两个属性子节点（title/size）：用get获取属性值
* 两个注释子节点（File First Commnet/File Second Comment）：用get_value或者data获取注释内容
* 两个数据子节点（两个path）。包含属性节点、注释节点和具体的数据节点

XML中节点分为属性节点、注释节点、数据节点

每种节点有两种元素组成：属性 + 属性值（注释节点除外：注释节点没有属性，只有值）

### 代码示例
下面展示如何解析该 `XML` 文件

```cpp

#include <iostream>
#include <string>
#include <boost/typeof/typeof.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
 
using namespace std;
using boost::property_tree::ptree;

int main() 
{
    string strTmp;
    // XML文件的路径
    char szXmlFile[] = "E:/test.xml";		
    // 创建ptree属性树的对象
    ptree pt;
    // 将XML文件读入到属性树对象中
    xml_parser::read_xml(szXmlFile, pt);

    // 只获取XML文件中的某些节点
    boost::optional<string> op = pt.get_optional<string>("config.file.<xmlattr>.title");
    if (op) {
        // 输出 windows
        cout << op.get() << endl;
    }

    op = pt.get_optional<string>("config.file.paths.<xmlattr>.attr");
    if (op) {
        // 输出 directory
        cout << op.get() << endl;
    }

    op = pt.get_optional<string>("config.file.<xmlcomment>");
    if (op) {
        // 输出 File Fisrt Comment
        cout << op.get() << endl;
    }

    // 遍历整个XML文件
    // 获取file节点下的所有子节点
    BOOST_AUTO(child, pt.get_child("config.file"));		
    
    // 依次遍历file节点下的所有子节点
    for (BOOST_AUTO(pos, child.begin()); pos != child.end(); ++pos)
    {
        strTmp.clear();
        // 属性子节点：该属性子节点对应着多个属性
        if ("<xmlattr>" == pos->first)
        {
            // 获取title属性的值
            strTmp = pos->second.get<string>("title");
            cout << strTmp << endl;		// 输出windows
            // 获取size属性的值
            strTmp = pos->second.get<string>("size");
            cout << strTmp << endl;		// 输出10Mb
            // 获取noexits属性的值，如果没有则用默认值"This is default"替代
            strTmp = pos->second.get<string>("noexits", "This is default");
            cout << strTmp << endl;		// 输出This is default
        }
        // 注释子节点
        else if ("<xmlcomment>" == pos->first)
        {
            // 没有属性值，直接使用 data() 获取注释内容
            strTmp = pos->second.data();		
            // 第一次输出：File First Comment
            // 第二次输出：File Second Comment
            cout << strTmp << endl;		
        }
        // 数据子节点
        // 这种节点又可以看成是一个新的节点，下面对应着属性子节点、注释子节点、数据子节点
        else 
        {
            // 获取当前节点下的所有子节点
            BOOST_AUTO(nextchild, pos->second.get_child(""));
            // 遍历当前节点下的所有子节点
            for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos != nextchild.end(); ++nextpos)
            {
                strTmp.clear();
                // 属性子节点
                if (nextpos->first == "<xmlattr>")
                {
                    // 获取该节点的值
                    strTmp = nextpos->second.get<string>("attr");
                    cout << strTmp << endl;		// 输出directory
                }
                // 注释子节点
                else if (nextpos->first == "<xmlcomment>")
                {
                    // 获取该节点的值：直接获取注释内容
                    strTmp = nextpos->second.data();
                    cout << strTmp << endl;		// 输出Paths Comment
                }
                // 数据子节点
                else 
                {
                    // 获取属性title的值
                    strTmp = nextpos->second.get<string>("<xmlattr>.title");
                    cout << strTmp << endl;
                    // 获取数据子节点的值。以下两种方案都可以
                    strTmp = nextpos->second.get_value<string>();
                    //strTmp = nextpos->second.data();
                    cout << strTmp << endl;
                    
                    // 依次输出
                    123 abc
                    456 efg
                    789 hij
                    111 klm
                    222 nop
                    333 qrs
                }
            }
        }
    }
}
```

## 读写 JSON 文件
### JOSN 文件格式说明
`JSON` 对象主要有两种格式：键值对和数组。`JSON` 灵活就在于键值对的值还可以键值对或者数组，数组的每个元素也是

### JSON文件示例

```JSON
{
    "rate":{
        "linktype":[0.8, 1.0, 1.0, 0.7, 0.4, 0.7, 0.8, 0.8, 0.9, 0.9, 0.4, 0.8, 1.0],
        "width":[[0.6, 0.8],
                 [0.7, 0.87],
                 [1.0, 1.2],
                 [1.2, 1.4],
                 [1.0, 1.0]],
        "use_toll":[0.33, 1.2]
    },
    "turn_cost":{
        "uturn":{
            "Hturn":0,
            "triangle":1200,
            "normal":[1200, 300, 60, 5]
        }
    }
}
```

### 代码示例

```cpp
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
 
using namespace std;
using boost::property_tree::ptree;

int main()
{    
    ptree pt;
    json_parser::read_json("test.json", pt);

    // 得到 rate.linktype 数组对象所在的节点
    ptree child_linktype = pt.get_child("rate.linktype");	

    // 遍历数组
    BOOST_FOREACH(ptree::value_type &vt, child_linktype) 
    {
        cout << vt.second.get_value<double>() << "\t";
    }
    cout << endl;

    // 直接解析键值对
    cout << pt.get<int>("turn_cost.uturn.triangle") << endl;
 
    return 0;
}
```