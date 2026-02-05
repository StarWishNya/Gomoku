#ifndef UI_HPP
#define UI_HPP

#include "game.hpp"
#include "config.hpp"
#include <string>

class UI {
public:
    UI();
    
    // 显示主菜单
    void showMainMenu();
    
    // 显示配置菜单
    void showConfigMenu(GameConfig& config);
    
    // 显示棋盘
    void displayBoard(const Board& board);
    
    // 显示当前玩家
    void displayCurrentPlayer(bool is_black);
    
    // 显示游戏结果
    void displayResult(GameResult result);
    
    // 显示错误消息
    void displayError(const std::string& message);
    
    // 显示信息
    void displayInfo(const std::string& message);
    
    // 获取用户输入（坐标）
    bool getMoveInput(int& x, int& y, const Board& board);
    
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
    // 将坐标转换为显示坐标（1-based）
    std::string formatCoordinate(int x, int y) const;
    
    // 获取棋子显示字符
    char getStoneChar(CellState state) const;
};

#endif // UI_HPP
