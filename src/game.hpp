#ifndef GAME_HPP
#define GAME_HPP

#include "config.hpp"
#include <vector>
#include <string>

enum class CellState {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

enum class GameResult {
    NONE,           // 游戏进行中
    BLACK_WIN,      // 黑棋胜利
    WHITE_WIN,      // 白棋胜利
    DRAW,           // 平局
    FORBIDDEN_MOVE  // 禁手（仅竞技模式）
};

class Board {
public:
    Board(int size);
    
    // 获取棋盘大小
    int getSize() const { return size_; }
    
    // 获取指定位置的棋子状态
    CellState getCell(int x, int y) const;
    
    // 放置棋子（返回是否成功）
    bool placeStone(int x, int y, CellState player);
    
    // 检查位置是否有效
    bool isValidPosition(int x, int y) const;
    
    // 检查位置是否为空
    bool isEmpty(int x, int y) const;
    
    // 清空棋盘
    void clear();
    
    // 获取棋盘状态（用于显示）
    std::vector<std::vector<CellState>> getBoardState() const;

private:
    int size_;
    std::vector<std::vector<CellState>> board_;
};

class Game {
public:
    Game(const GameConfig& config);
    
    // 获取当前玩家（true=黑棋，false=白棋）
    bool getCurrentPlayer() const { return current_player_; }
    
    // 获取游戏结果
    GameResult getResult() const { return result_; }
    
    // 获取棋盘
    const Board& getBoard() const { return board_; }
    
    // 获取配置
    const GameConfig& getConfig() const { return config_; }
    
    // 执行一步棋
    GameResult makeMove(int x, int y);
    
    // 检查游戏是否结束
    bool isGameOver() const;
    
    // 重置游戏
    void reset();
    
    // 设置当前玩家
    void setCurrentPlayer(bool is_black) { current_player_ = is_black; }

private:
    // 检查五连
    bool checkFiveInRow(int x, int y, CellState player) const;
    
    // 检查禁手（仅对黑棋）
    bool checkForbidden(int x, int y) const;
    
    // 检查双三禁手
    bool checkDoubleThree(int x, int y) const;
    
    // 检查双四禁手
    bool checkDoubleFour(int x, int y) const;
    
    // 检查长连禁手（超过5子）
    bool checkLongLine(int x, int y) const;
    
    // 计算指定方向的连子数
    int countLine(int x, int y, int dx, int dy, CellState player) const;
    
    // 检查活三
    bool checkLiveThree(int x, int y, int dx, int dy) const;
    
    // 检查活四
    bool checkLiveFour(int x, int y, int dx, int dy) const;
    
    // 检查冲四
    bool checkRushFour(int x, int y, int dx, int dy) const;
    
    Board board_;
    GameConfig config_;
    bool current_player_;  // true=黑棋，false=白棋
    GameResult result_;
};

#endif // GAME_HPP
