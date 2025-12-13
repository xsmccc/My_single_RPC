#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h> // readv 需要这个头文件
#include <unistd.h>

ssize_t Buffer::readFd(int fd, int* saveErrno) {
    char extrabuf[65536]; // 栈上准备 64K 空间-栈分配快，几乎不耗时
    
    struct iovec vec[2];
    const size_t writable = writeableBytes();
    
    // 第一块：Buffer 里的空闲空间
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;
    
    // 第二块：栈上的临时空间
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    // 如果 Buffer 够大，就不用第二块了
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;//此判断为经验化优化；堆>栈，则不用栈；否则用栈
    
    const ssize_t n = ::readv(fd, vec, iovcnt);//读取的所有vec的字节之和
    
    if (n < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {//将有符号整数转换为无符号整数进行比较
        // 第一块就装下了
        writeIndex_ += n;
    } else {
        // 第一块满了，剩下的在栈上
        writeIndex_ = buffer_.size();
        // 把栈上的数据追加进去 (会触发自动扩容)
        append(extrabuf, n - writable);
    }
    
    return n;
}