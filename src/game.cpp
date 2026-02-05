#include "game.hpp"
#include <algorithm>
#include <cmath>

// Board类实现
Board::Board(int size) : size_(size) {
    board_.resize(size_, std::vector<CellState>(size_, CellState::EMPTY));
}

CellState Board::getCell(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return CellState::EMPTY;
    }
    return board_[y][x];
}

bool Board::placeStone(int x, int y, CellState player) {
    if (!isValidPosition(x, y) || !isEmpty(x, y)) {
        return false;
    }
    board_[y][x] = player;
    return true;
}

bool Board::isValidPosition(int x, int y) const {
    return x >= 0 && x < size_ && y >= 0 && y < size_;
}

bool Board::isEmpty(int x, int y) const {
    return isValidPosition(x, y) && board_[y][x] == CellState::EMPTY;
}

void Board::clear() {
    for (auto& row : board_) {
        std::fill(row.begin(), row.end(), CellState::EMPTY);
    }
}

std::vector<std::vector<CellState>> Board::getBoardState() const {
    return board_;
}

// Game类实现
Game::Game(const GameConfig& config) 
    : board_(config.board_size), config_(config), 
      current_player_(true), result_(GameResult::NONE) {
}

GameResult Game::makeMove(int x, int y) {
    if (isGameOver()) {
        return result_;
    }
    
    CellState player = current_player_ ? CellState::BLACK : CellState::WHITE;
    
    // 检查位置是否有效
    if (!board_.isValidPosition(x, y) || !board_.isEmpty(x, y)) {
        return GameResult::NONE;  // 无效移动
    }
    
    // 如果是黑棋且是竞技模式，检查禁手
    if (current_player_ && config_.game_mode == GameMode::COMPETITIVE) {
        if (checkForbidden(x, y)) {
            return GameResult::FORBIDDEN_MOVE;
        }
    }
    
    // 放置棋子
    if (!board_.placeStone(x, y, player)) {
        return GameResult::NONE;
    }
    
    // 检查是否五连
    if (checkFiveInRow(x, y, player)) {
        result_ = current_player_ ? GameResult::BLACK_WIN : GameResult::WHITE_WIN;
        return result_;
    }
    
    // 检查是否平局（棋盘满了）
    bool board_full = true;
    for (int i = 0; i < board_.getSize(); ++i) {
        for (int j = 0; j < board_.getSize(); ++j) {
            if (board_.isEmpty(j, i)) {
                board_full = false;
                break;
            }
        }
        if (!board_full) break;
    }
    
    if (board_full) {
        result_ = GameResult::DRAW;
        return result_;
    }
    
    // 切换玩家
    current_player_ = !current_player_;
    return GameResult::NONE;
}

bool Game::isGameOver() const {
    return result_ != GameResult::NONE;
}

void Game::reset() {
    board_.clear();
    current_player_ = true;
    result_ = GameResult::NONE;
}

bool Game::checkFiveInRow(int x, int y, CellState player) const {
    // 四个方向：横、竖、左斜、右斜
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int d = 0; d < 4; ++d) {
        int count = 1;  // 包括当前棋子
        int dx = directions[d][0];
        int dy = directions[d][1];
        
        // 正向计数
        for (int i = 1; i < 5; ++i) {
            int nx = x + dx * i;
            int ny = y + dy * i;
            if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == player) {
                count++;
            } else {
                break;
            }
        }
        
        // 反向计数
        for (int i = 1; i < 5; ++i) {
            int nx = x - dx * i;
            int ny = y - dy * i;
            if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == player) {
                count++;
            } else {
                break;
            }
        }
        
        if (count >= 5) {
            return true;
        }
    }
    
    return false;
}

bool Game::checkForbidden(int x, int y) const {
    // 禁手检查仅对黑棋有效
    if (!current_player_) {
        return false;
    }
    
    // 检查长连禁手
    if (checkLongLine(x, y)) {
        return true;
    }
    
    // 检查双三禁手
    if (checkDoubleThree(x, y)) {
        return true;
    }
    
    // 检查双四禁手
    if (checkDoubleFour(x, y)) {
        return true;
    }
    
    return false;
}

bool Game::checkLongLine(int x, int y) const {
    // 临时放置黑棋检查
    Board temp_board = board_;
    temp_board.placeStone(x, y, CellState::BLACK);
    
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int d = 0; d < 4; ++d) {
        int count = 1;
        int dx = directions[d][0];
        int dy = directions[d][1];
        
        // 正向计数
        for (int i = 1; i < 6; ++i) {
            int nx = x + dx * i;
            int ny = y + dy * i;
            if (temp_board.isValidPosition(nx, ny) && temp_board.getCell(nx, ny) == CellState::BLACK) {
                count++;
            } else {
                break;
            }
        }
        
        // 反向计数
        for (int i = 1; i < 6; ++i) {
            int nx = x - dx * i;
            int ny = y - dy * i;
            if (temp_board.isValidPosition(nx, ny) && temp_board.getCell(nx, ny) == CellState::BLACK) {
                count++;
            } else {
                break;
            }
        }
        
        if (count > 5) {
            return true;  // 长连禁手
        }
    }
    
    return false;
}

bool Game::checkDoubleThree(int x, int y) const {
    // 简化实现：检查是否在多个方向形成活三
    // 注意：这里假设已经放置了黑棋（在实际调用前会先检查）
    int live_three_count = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    // 临时放置黑棋进行检查
    Board temp_board = board_;
    if (!temp_board.placeStone(x, y, CellState::BLACK)) {
        return false;
    }
    
    // 检查每个方向
    for (int d = 0; d < 4; ++d) {
        // 检查正向和反向是否形成活三
        if (checkLiveThree(x, y, directions[d][0], directions[d][1])) {
            live_three_count++;
        }
    }
    
    return live_three_count >= 2;
}

bool Game::checkDoubleFour(int x, int y) const {
    // 简化实现：检查是否在多个方向形成四（活四或冲四）
    int four_count = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    // 临时放置黑棋进行检查
    Board temp_board = board_;
    if (!temp_board.placeStone(x, y, CellState::BLACK)) {
        return false;
    }
    
    // 检查每个方向
    for (int d = 0; d < 4; ++d) {
        if (checkLiveFour(x, y, directions[d][0], directions[d][1]) ||
            checkRushFour(x, y, directions[d][0], directions[d][1])) {
            four_count++;
        }
    }
    
    return four_count >= 2;
}

bool Game::checkLiveThree(int x, int y, int dx, int dy) const {
    // 简化版活三检测：检查是否形成活三模式
    // 活三：两端都有空位，且可以形成活四
    
    // 检查正向
    int forward_count = 0;
    int forward_empty = 0;
    for (int i = 1; i < 4; ++i) {
        int nx = x + dx * i;
        int ny = y + dy * i;
        if (board_.isValidPosition(nx, ny)) {
            if (board_.getCell(nx, ny) == CellState::BLACK) {
                forward_count++;
            } else if (board_.getCell(nx, ny) == CellState::EMPTY) {
                forward_empty++;
                break;
            } else {
                break;
            }
        }
    }
    
    // 检查反向
    int backward_count = 0;
    int backward_empty = 0;
    for (int i = 1; i < 4; ++i) {
        int nx = x - dx * i;
        int ny = y - dy * i;
        if (board_.isValidPosition(nx, ny)) {
            if (board_.getCell(nx, ny) == CellState::BLACK) {
                backward_count++;
            } else if (board_.getCell(nx, ny) == CellState::EMPTY) {
                backward_empty++;
                break;
            } else {
                break;
            }
        }
    }
    
    // 活三：总共3个黑棋，且两端都有空位
    int total_count = forward_count + backward_count;
    return total_count == 2 && forward_empty > 0 && backward_empty > 0;
}

bool Game::checkLiveFour(int x, int y, int dx, int dy) const {
    // 活四：四个黑棋连成一线，且一端有空位
    int forward_count = 0;
    int backward_count = 0;
    
    for (int i = 1; i < 5; ++i) {
        int nx = x + dx * i;
        int ny = y + dy * i;
        if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == CellState::BLACK) {
            forward_count++;
        } else {
            break;
        }
    }
    
    for (int i = 1; i < 5; ++i) {
        int nx = x - dx * i;
        int ny = y - dy * i;
        if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == CellState::BLACK) {
            backward_count++;
        } else {
            break;
        }
    }
    
    return (forward_count + backward_count) == 3;
}

bool Game::checkRushFour(int x, int y, int dx, int dy) const {
    // 冲四：四个黑棋连成一线，但一端被白棋或边界阻挡
    int forward_count = 0;
    int backward_count = 0;
    bool forward_blocked = false;
    bool backward_blocked = false;
    
    for (int i = 1; i < 5; ++i) {
        int nx = x + dx * i;
        int ny = y + dy * i;
        if (board_.isValidPosition(nx, ny)) {
            if (board_.getCell(nx, ny) == CellState::BLACK) {
                forward_count++;
            } else if (board_.getCell(nx, ny) == CellState::WHITE) {
                forward_blocked = true;
                break;
            } else {
                break;
            }
        } else {
            forward_blocked = true;
            break;
        }
    }
    
    for (int i = 1; i < 5; ++i) {
        int nx = x - dx * i;
        int ny = y - dy * i;
        if (board_.isValidPosition(nx, ny)) {
            if (board_.getCell(nx, ny) == CellState::BLACK) {
                backward_count++;
            } else if (board_.getCell(nx, ny) == CellState::WHITE) {
                backward_blocked = true;
                break;
            } else {
                break;
            }
        } else {
            backward_blocked = true;
            break;
        }
    }
    
    return (forward_count + backward_count) == 3 && (forward_blocked || backward_blocked);
}

int Game::countLine(int x, int y, int dx, int dy, CellState player) const {
    int count = 1;  // 包括当前位置
    
    // 正向计数
    for (int i = 1; i < 5; ++i) {
        int nx = x + dx * i;
        int ny = y + dy * i;
        if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == player) {
            count++;
        } else {
            break;
        }
    }
    
    // 反向计数
    for (int i = 1; i < 5; ++i) {
        int nx = x - dx * i;
        int ny = y - dy * i;
        if (board_.isValidPosition(nx, ny) && board_.getCell(nx, ny) == player) {
            count++;
        } else {
            break;
        }
    }
    
    return count;
}
