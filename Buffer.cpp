#include "Buffer.h"

Buffer::Buffer(uint16_t sep):sep_(sep)
{

}

Buffer::~Buffer()
{

}

void Buffer::append(const char* data, size_t size)
{
    buf_.append(data, size);
}

void Buffer::appendwithsep(const char*data, size_t size)
{
    if(sep_==0)
    {
        buf_.append(data, size);
    }
    else if(sep_==1)
    {
        buf_.append((char*)&size, 4);
        buf_.append(data, size);
    }

}

void Buffer::eraser(size_t pos, size_t nn)
{
    buf_.erase(pos,nn);
}

size_t Buffer::size()
{
    return buf_.size();
}

const char* Buffer::data()
{
    return buf_.data();
}

void Buffer::clear()
{
    buf_.clear();
}

bool Buffer::pickmessage(std::string &ss)
{
    if(buf_.size()==0)return false;
    if(sep_==0)
    {
        ss = buf_;
        buf_.clear();
    }
    else if (sep_==1)
    {
        int len;
        memcpy(&len, buf_.data(), 4);

        if(buf_.size()<len+4)return false;

        ss = buf_.substr(4,len);
        buf_.erase(0,len+4);
    }
    return true;
    
}