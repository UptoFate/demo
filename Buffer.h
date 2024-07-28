#ifndef __BUFFER__
#define __BUFFER__

#include <string>
#include <iostream>
#include <cstring>

class Buffer
{
private:
    std::string buf_;       //string 既可以存放文本也可以存放二进制数据
    const uint16_t sep_;    //报文的分隔符：0.无分隔符（固定长度、视频会议） 1.四字节分隔符 2."\r\n\r\n" 分隔符（http）
public:
    Buffer(uint16_t sep = 0);
    ~Buffer();

    void append(const char* data, size_t size); //把数据加入Buffer
    void appendwithsep(const char*data, size_t size);  //把数据加入Buffer 附加报头
    void eraser(size_t pos, size_t nn);         //从pos开始删除nn个字节      
    size_t size();          //返回Buffer大小
    const char* data();     //返回Buffer首地址
    void clear();
    bool pickmessage(std::string &ss);          //从buf_中拆出一个报文放于ss中若无返回false
};




#endif