/*
Window.hpp
Window保存光标位置和正文的可见区域,根据Buffer的行数和行长限制移动范围.
移动只改变##光标##和##视口##,文件内容的修改由Buffer完成.
*/
#ifndef MINIVIM_WINDOW_HPP
#define MINIVIM_WINDOW_HPP

#include "Buffer.hpp"
#include "Types.hpp"


namespace sjtu {

class Window {

public:
    void Resize(ScreenSize terminal_size);
    //终端大小改变时 重新计算视口
    void ApplyMotion(const Buffer& buffer, Motion motion);
    //获取hjkl这样的“移动“数据
    void EnsureCursorVisible(const Buffer& buffer);
    //确保光标可见 应该是让屏幕跟着操作移动
    //allow_line_end为true时允许停在最后一个字符之后,供插入模式使用
    //false时应该给Normal使用 区分光标是停在字符之间还是字符上
    void SetCursor(const Buffer& buffer, Position position, bool allow_line_end);
    const Position& GetCursor() const ;
    //获取光标位置
    const Viewport& GetViewport() const ;

private:
    //四个方向实现单步移动操作
    void MoveLeft(const Buffer& buffer, size_t count);
    void MoveRight(const Buffer& buffer, size_t count);
    void MoveUp(const Buffer& buffer, size_t count);
    void MoveDown(const Buffer& buffer, size_t count);

    Position cursor_{}; //Buffer中的光标位置
    Viewport viewport_{}; //正文可见区域及其滚动偏移
    //实际绘制的位置 就是cursor_ - viewport_
    size_t desired_column_{0}; //上下移动时希望保持的显示列,经过短行时也保留这个目标
};

} // namespace sjtu

#endif // MINIVIM_WINDOW_HPP
