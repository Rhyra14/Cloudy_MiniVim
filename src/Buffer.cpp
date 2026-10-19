#include <fstream> //用来读写文件
#include <utility> //move要的头文件
#include <iostream>

#include "Buffer.hpp"

namespace sjtu {

Buffer::Buffer(const std::filesystem::path& path){ // 读文件
    if(path.empty()) {
        path_ = "";
        lines_ = {""};
        return ;
    }
    path_ = path;
    if(!std::filesystem::exists(path_)) {
        lines_ = {""};
        return ;
    } //判断一下文件不存在的情况
    
    std::ifstream input(path_);
    if(!input.is_open()) {
        lines_ = {""};
        return ;
    }//判断一下打不开的情况

    std::string line;
    while(std::getline(input, line)) lines_.push_back(line);
    if(lines_.empty()) lines_ = {""};
    //从path指向的文件构造Buffer,你需要打开文件
    // 并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
}

Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path) { //初始化Buffer
    lines_ = std::move(lines);
    path_ = std::move(path);
    if(lines_.empty()) lines_ = {""};
    //move可以避免拷贝 提升效率
}
//上述完成了Buffer的初始化 下面要获得一些数据（？

std::size_t Buffer::GetLineCount() const {
    //返回文件行数
    return lines_.size();
}

const std::string& Buffer::GetLineAt(std::size_t row) const {
    //返回第row行的内容
    return lines_[row];
}

std::string Buffer::GetDisplayName() const {
    //返回文件名,若是新文件,返回"[No Name]"
    if(path_.empty()) return "[No Name]";
    return path_.filename().string();
}

bool Buffer::IsModified() const {
    //返回文件和上次保存比起来是否被修改过
    return modified_;
}
//上面四个函数 获取了一些数据 包括一个保存信息的判断
//下面就是一些具体的操作了

void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value) {
    //在第row行第col列插入一个value, 注意##越界检查##
    if(row >= lines_.size()) return ;
    if(column > lines_[row].size()) return;
    lines_[row].insert(lines_[row].begin() + column, value);
    modified_ = true;
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
    //在第row行第col列删除一个value
    if(row >= lines_.size()) return ;
    if(column >= lines_[row].size()) return ;
    lines_[row].erase(lines_[row].begin() + column);
    modified_ = true;
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    if(row >= lines_.size()) return ;
    if(column > lines_[row].size()) return ;
    std::string tail = lines_[row].substr(column); // 获取截断点以后字符串
    lines_[row].resize(column); // 保留左边字符串
    lines_.insert(lines_.begin() + row + 1, tail);
    modified_ = true;
}

void Buffer::JoinLine(std::size_t row) {
    //把第row + 1行合并进第row行
    if(row + 1 >= lines_.size()) return ;
    lines_[row] += lines_[row + 1];
    lines_.erase(lines_.begin() + row + 1);
    modified_ = true;
}

//读写保存part
void Buffer::Save() {
   //把文件内容保存, 直接调用WriteTo方法
   if(path_.empty()) throw std::runtime_error("No file name, cannnot save");
   WriteTo(path_);
   modified_ = false;
}

void Buffer::SaveAs(const std::filesystem::path& path) {
    WriteTo(path);
    path_ = path;
    modified_ = false;
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
    //实际将缓冲区中的内容写入path指向的文件中
    std::ofstream output(path);
    if(!output.is_open()) throw std::runtime_error("Cannot open file for writing");
    for(std::size_t i=0; i<lines_.size(); i++) output << lines_[i] << std::endl;
}

} // namespace sjtu

/*
总结一下buffer干的事情 主要就四层
(i) 读文件 然后初始化
读文件用到了fstream下的ifstream去传path_的路径信息 ; 
初始化信息用std::move(utility下)可以避免复制信息从而提高效率

(ii) 一些基本信息获得 

(iii) 实现一些基本操作 增加和删除元素 分割和合并行
通过字符串里面的insert, erase 实现 分割用到了substr(字串),和一个没见过的resize(强制修改长度)

(iv) 写文件
用到fstream下的ofstream存path 然后依次打印

(ii)~(iv)中都要注意modified_的标记更新 边界条件的判断贯穿始终
*/