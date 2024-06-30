#include "Buffer.h"

Buffer::Buffer()
{

}

Buffer::~Buffer()
{

}

void Buffer::append(const char* data, size_t size)
{
    buf_.append(data, size);
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