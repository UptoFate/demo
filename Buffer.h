#ifndef __BUFFER__
#define __BUFFER__

#include <string>
#include <iostream>

class Buffer
{
private:
    std::string buf_;   //string 既可以存放文本也可以存放二进制数据

public:
    Buffer();
    ~Buffer();

    void append(const char* data, size_t size); //把数据加入Buffer
    void eraser(size_t pos, size_t nn);         //从pos开始删除nn个字节      
    size_t size();          //返回Buffer大小
    const char* data();     //返回Buffer首地址
    void clear();
};




#endif