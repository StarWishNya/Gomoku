#ifndef UI_HPP
#define UI_HPP

#include "game.hpp"
#include "config.hpp"
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
#endif

// 按键枚举
enum class KeyCode {
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4,
    ENTER = 5,
    ESC = 6,
    QUIT = 7
};

class UI {
public:
    UI();
    ~UI();
    
    // 显示主菜单
    void showMainMenu();
    
    // 显示配置菜单
    void showConfigMenu(GameConfig& config);
    
    // 显示棋盘
    void displayBoard(const Board& board);
    
    // 显示带光标的棋盘
    void displayBoardWithCursor(const Board& board, int cx, int cy, bool highlight);
    
    // 显示对方落子（带闪烁效果）
    void displayOpponentMove(const Board& board, int x, int y, bool opponent_is_black);
    
    // 显示当前玩家
    void displayCurrentPlayer(bool is_black);
    
    // 显示游戏结果
    void displayResult(GameResult result);
    
    // 显示错误消息
    void displayError(const std::string& message);
    
    // 显示信息
    void displayInfo(const std::string& message);
    
    // 获取用户输入（坐标）- 使用方向键控制
    bool getMoveInput(int& x, int& y, const Board& board, bool current_player_is_black = true);
    
    // 获取菜单选择
    int getMenuChoice(int min, int max);
    
    // 获取整数输入
    int getIntInput(const std::string& prompt, int min, int max);
    
    // 获取字符串输入
    std::string getStringInput(const std::string& prompt);
    
    // 清屏
    void clearScreen();
    
    // 等待用户按键
    void waitForEnter();

private:
    // 光标位置
    int cursor_x_;
    int cursor_y_;
    
    // 终端原始模式状态
    bool terminal_raw_mode_;
    
#ifdef _WIN32
    // Windows下保存原始控制台模式
    DWORD original_console_mode_;
#else
    // Linux下保存原始终端设置
    struct termios original_termios_;
#endif
    
    // 设置终端原始模式
    void setTerminalRawMode(bool enable);
    
    // 读取单个按键
    KeyCode readKey();
    
    // 将坐标转换为显示坐标（1-based）
    std::string formatCoordinate(int x, int y) const;
    
    // 获取棋子显示字符
    char getStoneChar(CellState state) const;
};

#endif // UI_HPP
