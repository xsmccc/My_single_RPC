#pragma once
#include <vector>
#include <string>
#include <algorithm>

class Buffer{
public:
    static const size_t kCheapPrepend = 8;//头部预留空间-TCP网络包的头部都很小
    static const size_t kInitialSize = 1024;

    //explicit防止隐式转换带来的bug
    explicit Buffer(size_t initialSize = kInitialSize)
    : buffer_(kCheapPrepend + initialSize)
    , readIndex_(kCheapPrepend)
    ,writeIndex_(kCheapPrepend) {}

    // 可读字节数
    size_t readableBytes() const {return writeIndex_ - readIndex_;}

    // 可写字节数
    size_t writeableBytes() const {return buffer_.size() - writeIndex_;}

    // 读指针的位置 (数据开始的地方)
    const char* peek() const {return begin()+readIndex_;}

    // 把 string 里的数据全部拿出来，并移动 readIndex
    std::string retrieveAllAsString(){
        std::string str(peek(),readableBytes());
        retrieveAll();
        return str;
    }

    // 复位游标
    void retrieveAll() {
        readIndex_ = kCheapPrepend;
        writeIndex_ = kCheapPrepend;
    }

    // 写数据 (append)
    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    void append(const char* data, size_t len) {
        ensureWriteableBytes(len); // 确保空间够用
        std::copy(data, data + len, beginWrite());
        writeIndex_ += len;
    }
    
    // 从 fd 读取数据
    ssize_t readFd(int fd, int* saveErrno);

private:
    // 获取 vector 底层指针-常量正确性设计
    char* begin() {return &*buffer_.begin();}               //针对普通对象调用-可修改数据  
    const char* begin() const {return &*buffer_.begin();}   //常量引用-仅读权限，不可修改数据

    // 写指针位置
    char* beginWrite() {return begin()+writeIndex_;}

    // 扩容逻辑
    void ensureWriteableBytes(size_t len) {
        if(writeableBytes() < len){
            makeSpace(len);
        }
    }

    // 扩容的具体实现
    void makeSpace(size_t len){
        if(writeableBytes() + readIndex_ < len + kCheapPrepend){//已读的数据前面的空间就是可用的
            buffer_.resize(writeIndex_ + len);
        }
        else{   //如果可以通过搬运就容纳数据
            size_t readable = readableBytes();
            std::copy(begin()+readIndex_,   //开始readIndex
                    begin()+writeIndex_,    //结束writeIndex
                    begin()+kCheapPrepend); //搬到预留空间后
            readIndex_ = kCheapPrepend;
            writeIndex_ = readIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readIndex_;  //数据处理的位置
    size_t writeIndex_; //数据写的位置

};