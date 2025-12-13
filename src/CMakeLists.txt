# 1. 查找当前目录下的所有源文件 (.cpp, .cc)
aux_source_directory(. SRC_LIST)

# 2. 生成动态库 (libmprpc.so)
add_library(mprpc SHARED ${SRC_LIST})

# 3. 链接依赖库
target_link_libraries(mprpc protobuf pthread)