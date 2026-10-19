#include "Window.hpp"
#include "TextLayout.hpp"

#include <algorithm>
#include <cctype>
//cctype提供isprint判断函数->是否是可打印字符

namespace sjtu {


void Window::Resize(ScreenSize terminal_size) {
    //底部留一行给命令或提示,其余作为正文区域;
    //正文行数和列数都至少取1
    //修改视口即可
    if(terminal_size.rows_ > 1) viewport_.rows_ = terminal_size.rows_ - 1;
    else viewport_.rows_ = 1;
    if(terminal_size.columns_ > 0) viewport_.columns_ = terminal_size.columns_;
    else viewport_.columns_ = 1;
}// 搞定viewport的初始化 根据当前的屏幕物理大小去确定可视区域
//下面主要就是cursor_ 和 viewport_ 之间的关系

void Window::ApplyMotion(const Buffer& buffer, Motion motion) {
    //1. 根据方向调用对应的移动函数,Basic中每次移动一步,在Advanced中你可以改变count/添加别的case
    //2. 将行列限制在Normal模式的合法范围内(Buffer应始终至少有一行)
    //3. 调整视口,让移动后的光标可见(EnsureCursorVisible)
    switch(motion) {    
        case Motion::Left: MoveLeft(buffer, 1); break;
        case Motion::Right: MoveRight(buffer, 1); break;
        case Motion::Up: MoveUp(buffer, 1); break;
        case Motion::Down: MoveDown(buffer, 1); break;
    }
    EnsureCursorVisible(buffer);
}

void Window::EnsureCursorVisible(const Buffer& buffer) {
    //1. 光标高于或低于可见区域时,调整top_,使光标刚好进入区域
    //->不管是往上越界 还是 往下越界 都是调整top_
    //2. 把光标的字符下标换算成显示列,再用相同思路调整left_
    //->不管是往左越界 还是 往右越界 都是调整left
    if(cursor_.row_ < viewport_.top_) viewport_.top_ = cursor_.row_;
    else if(cursor_.row_ >= viewport_.top_ + viewport_.rows_) viewport_.top_ = cursor_.row_ - viewport_.rows_ + 1;
    const std::string& line = buffer.GetLineAt(cursor_.row_);
    size_t cur_col = BufferColumnToRenderColumn(line, cursor_.column_);
    if(cur_col < viewport_.left_) viewport_.left_ = cur_col;
    else if(cur_col >= viewport_.left_ + viewport_.columns_) viewport_.left_ = cur_col - viewport_.columns_ + 1;
}

const Position& Window::GetCursor() const {
    //返回当前光标位置的只读引用
    return cursor_;
}

const Viewport& Window::GetViewport() const {
    //返回当前可见区域的只读引用,供Renderer绘制
   return viewport_;
}

void Window::SetCursor(const Buffer& buffer, Position position, bool allow_line_end) {
    //该函数作用是 强制调整cursor_位置到potision
    //注意要同步更新desired_column_ 同时发挥上面一个关键函数的作用 Ensure new cursor_ visible
    //allow_line_end 是用来判断行末的 也就是insert模式(光标在字符之间) 和 normal模式(光标在字符上)
    //1. 先限制行号,再根据该行长度和allow_line_end限制列号
    //2. 用新位置更新上下移动时的目标显示列
    //3. 调整视口,保证光标可见
    size_t max_row = buffer.GetLineCount() - 1;
    size_t max_col;
    size_t target_row = std::min(max_row, position.row_);
    size_t target_col;
    const std::string& line = buffer.GetLineAt(target_row);
    if(allow_line_end) max_col = line.size();
    else {
        if(line.empty()) max_col = 0;
        else max_col = line.size() - 1;
    }
    target_col = std::min(max_col, position.column_);
    cursor_.row_ = target_row;
    cursor_.column_ = target_col;
    desired_column_ = BufferColumnToRenderColumn(line, target_col);
    EnsureCursorVisible(buffer);
}

//下列是实现四个基本操作
void Window::MoveLeft(const Buffer& buffer, std::size_t count) {
    //向左移动count个字符,最多到行首,并更新目标显示列
    if(count > cursor_.column_) cursor_.column_ = 0;
    else cursor_.column_ -= count;
    const std::string& line = buffer.GetLineAt(cursor_.row_);
    desired_column_ = BufferColumnToRenderColumn(line, cursor_.column_);
}

void Window::MoveRight(const Buffer& buffer, std::size_t count) {
    //向右移动count个字符,最多到最后一个字符,并更新目标显示列
    const std::string& line = buffer.GetLineAt(cursor_.row_);
    std::size_t max_col;
    if(line.empty()) max_col = 0;
    else max_col = line.size() - 1;
    if(cursor_.column_ + count > max_col) cursor_.column_ = max_col;
    else cursor_.column_ += count;
    desired_column_ = BufferColumnToRenderColumn(line, cursor_.column_);
}


void Window::MoveUp(const Buffer& buffer, std::size_t count) {
    //先算目标行,最多到第一行,再将期望的显示列换算成目标行的字符下标
    //经过短行时不要更新desired_screen_column_,这样继续移动到长行时能回到原来的列
    if(cursor_.row_ >= count) cursor_.row_ -= count;
    else cursor_.row_ = 0;
    const std::string& line = buffer.GetLineAt(cursor_.row_);
    std::size_t max_col;
    if(line.empty()) max_col = 0;
    else max_col = line.size() - 1;
    std::size_t target_col = RenderColumnToBufferColumn(line, desired_column_);
    cursor_.column_ = std::min(target_col, max_col);
}

void Window::MoveDown(const Buffer& buffer, std::size_t count) {
    //先算目标行,最多到最后一行,再根据desired_screen_column_寻找目标字符
    //与向上移动一样,保留期望显示列
    std::size_t last_row = buffer.GetLineCount() - 1;
    if(cursor_.row_ + count > last_row) cursor_.row_ = last_row;
    else cursor_.row_ += count;
    const std::string& line = buffer.GetLineAt(cursor_.row_);
    std::size_t target_col = RenderColumnToBufferColumn(line, desired_column_);
    std::size_t max_col;
    if(line.empty()) max_col = 0;
    else max_col = line.size() - 1;
    cursor_.column_ = std::min(target_col, max_col);
}

} // namespace sjtu

/*
总结一下Window干的事情
*/