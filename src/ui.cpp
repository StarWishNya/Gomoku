#include "ui.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

// ANSI转义码常量
const std::string ANSI_BOLD = "\033[1m";
const std::string ANSI_REVERSE = "\033[7m";
const std::string ANSI_RESET = "\033[0m";

// 全局标志，用于信号处理
static volatile sig_atomic_t g_interrupted = 0;

static void signalHandler(int signal) {
    g_interrupted = 1;
    // 设置默认处理，以便下次信号时强制退出
    std::signal(signal, SIG_DFL);
}

UI::UI() : cursor_x_(0), cursor_y_(0), terminal_raw_mode_(false) {
    // 注册信号处理器
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

UI::~UI() {
    // 确保退出时恢复终端模式
    if (terminal_raw_mode_) {
        setTerminalRawMode(false);
    }
}

void UI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UI::showMainMenu() {
    clearScreen();
    std::cout << "========================================" << std::endl;
    std::cout << "        联机五子棋游戏" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. 启动服务器（等待连接）" << std::endl;
    std::cout << "2. 连接到服务器" << std::endl;
    std::cout << "3. 退出" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "请选择: ";
}

void UI::showConfigMenu(GameConfig& config) {
    clearScreen();
    std::cout << "========================================" << std::endl;
    std::cout << "        游戏配置" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "当前配置:" << std::endl;
    std::cout << "  棋盘大小: " << config.board_size << "x" << config.board_size << std::endl;
    std::cout << "  游戏模式: " << config.getModeString() << std::endl;
    std::cout << "  先后手: " << config.getOrderString() << std::endl;
    std::cout << "  端口: " << config.port << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. 设置棋盘大小 (当前: " << config.board_size << ")" << std::endl;
    std::cout << "2. 设置游戏模式 (当前: " << config.getModeString() << ")" << std::endl;
    std::cout << "3. 设置先后手 (当前: " << config.getOrderString() << ")" << std::endl;
    std::cout << "4. 设置端口 (当前: " << config.port << ")" << std::endl;
    std::cout << "5. 确认并开始游戏" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "请选择: ";
    
    int choice = getMenuChoice(1, 5);
    
    switch (choice) {
        case 1: {
            int size = getIntInput("请输入棋盘大小 (10-25): ", 10, 25);
            config.board_size = size;
            break;
        }
        case 2: {
            std::cout << "1. 无规则模式" << std::endl;
            std::cout << "2. 竞技模式（有禁手）" << std::endl;
            int mode = getMenuChoice(1, 2);
            config.game_mode = (mode == 1) ? GameMode::STANDARD : GameMode::COMPETITIVE;
            break;
        }
        case 3: {
            std::cout << "1. 先手（黑棋）" << std::endl;
            std::cout << "2. 后手（白棋）" << std::endl;
            std::cout << "3. 随机" << std::endl;
            int order = getMenuChoice(1, 3);
            config.player_order = (order == 1) ? PlayerOrder::FIRST : 
                                 (order == 2) ? PlayerOrder::SECOND : PlayerOrder::RANDOM;
            break;
        }
        case 4: {
            int port = getIntInput("请输入端口 (1025-65535): ", 1025, 65535);
            config.port = port;
            break;
        }
        case 5:
            return;
    }
    
    // 递归调用以继续配置
    showConfigMenu(config);
}

void UI::displayBoard(const Board& board) {
    displayBoardWithCursor(board, -1, -1, false);
}

void UI::displayBoardWithCursor(const Board& board, int cx, int cy, bool highlight) {
    int size = board.getSize();
    
    // 打印列号
    std::cout << "\n   ";
    for (int i = 0; i < size; ++i) {
        std::cout << std::setw(3) << (i + 1);
    }
    std::cout << "\n";
    
    // 打印棋盘
    for (int y = 0; y < size; ++y) {
        std::cout << std::setw(2) << (y + 1) << " ";
        for (int x = 0; x < size; ++x) {
            bool is_cursor = (x == cx && y == cy);
            CellState cell = board.getCell(x, y);
            char stone = getStoneChar(cell);
            
            if (is_cursor && highlight) {
                // 高亮显示：使用加粗和反色
                std::cout << ANSI_BOLD << ANSI_REVERSE;
            }
            
            // 如果是光标位置且为空，显示特殊标记
            if (is_cursor && cell == CellState::EMPTY) {
                std::cout << "[" << stone << "]";
            } else {
                std::cout << " " << stone << " ";
            }
            
            if (is_cursor && highlight) {
                std::cout << ANSI_RESET;
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

char UI::getStoneChar(CellState state) const {
    switch (state) {
        case CellState::EMPTY: return '.';
        case CellState::BLACK: return 'X';  // 黑棋用X表示
        case CellState::WHITE: return 'O';  // 白棋用O表示
        default: return '?';
    }
}

void UI::displayCurrentPlayer(bool is_black) {
    std::cout << "当前玩家: " << (is_black ? "黑棋 (X)" : "白棋 (O)") << std::endl;
}

void UI::displayOpponentMove(const Board& board, int x, int y, bool opponent_is_black) {
    // 显示对方落子位置，带闪烁效果
    const int flash_count = 3;  // 闪烁3次
    const auto flash_interval = std::chrono::milliseconds(300);
    
    int size = board.getSize();
    char opponent_stone = opponent_is_black ? 'X' : 'O';
    
    for (int i = 0; i < flash_count; ++i) {
        clearScreen();
        
        // 打印列号
        std::cout << "\n   ";
        for (int j = 0; j < size; ++j) {
            std::cout << std::setw(3) << (j + 1);
        }
        std::cout << "\n";
        
        // 打印棋盘，高亮显示对方落子位置
        for (int py = 0; py < size; ++py) {
            std::cout << std::setw(2) << (py + 1) << " ";
            for (int px = 0; px < size; ++px) {
                bool is_opponent_move = (px == x && py == y);
                CellState cell = board.getCell(px, py);
                char stone = getStoneChar(cell);
                
                if (is_opponent_move) {
                    // 高亮显示对方落子位置
                    std::cout << ANSI_BOLD << ANSI_REVERSE;
                    // 如果位置为空，显示对方即将落子的棋子
                    if (cell == CellState::EMPTY) {
                        std::cout << "[" << opponent_stone << "]";
                    } else {
                        std::cout << " " << stone << " ";
                    }
                    std::cout << ANSI_RESET;
                } else {
                    std::cout << " " << stone << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        
        std::cout << "对手落子: " << (opponent_is_black ? "黑棋 (X)" : "白棋 (O)") 
                  << " 位置: (" << (x + 1) << ", " << (y + 1) << ")" << std::endl;
        std::this_thread::sleep_for(flash_interval);
        
        clearScreen();
        
        // 打印列号
        std::cout << "\n   ";
        for (int j = 0; j < size; ++j) {
            std::cout << std::setw(3) << (j + 1);
        }
        std::cout << "\n";
        
        // 打印棋盘，正常显示
        for (int py = 0; py < size; ++py) {
            std::cout << std::setw(2) << (py + 1) << " ";
            for (int px = 0; px < size; ++px) {
                bool is_opponent_move = (px == x && py == y);
                CellState cell = board.getCell(px, py);
                char stone = getStoneChar(cell);
                
                if (is_opponent_move && cell == CellState::EMPTY) {
                    // 显示对方即将落子的棋子（不高亮）
                    std::cout << "[" << opponent_stone << "]";
                } else {
                    std::cout << " " << stone << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        
        std::cout << "对手落子: " << (opponent_is_black ? "黑棋 (X)" : "白棋 (O)") 
                  << " 位置: (" << (x + 1) << ", " << (y + 1) << ")" << std::endl;
        std::this_thread::sleep_for(flash_interval);
    }
}

void UI::displayResult(GameResult result) {
    std::cout << "\n========================================" << std::endl;
    switch (result) {
        case GameResult::BLACK_WIN:
            std::cout << "游戏结束！黑棋 (X) 获胜！" << std::endl;
            break;
        case GameResult::WHITE_WIN:
            std::cout << "游戏结束！白棋 (O) 获胜！" << std::endl;
            break;
        case GameResult::DRAW:
            std::cout << "游戏结束！平局！" << std::endl;
            break;
        case GameResult::FORBIDDEN_MOVE:
            std::cout << "禁手！请重新落子。" << std::endl;
            break;
        default:
            break;
    }
    std::cout << "========================================" << std::endl;
}

void UI::displayError(const std::string& message) {
    std::cout << "\n[错误] " << message << std::endl;
}

void UI::displayInfo(const std::string& message) {
    std::cout << "\n[信息] " << message << std::endl;
}

void UI::setTerminalRawMode(bool enable) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (enable) {
        GetConsoleMode(hStdin, &original_console_mode_);
        DWORD mode = original_console_mode_;
        mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
        SetConsoleMode(hStdin, mode);
    } else {
        SetConsoleMode(hStdin, original_console_mode_);
    }
#else
    if (enable) {
        tcgetattr(STDIN_FILENO, &original_termios_);
        struct termios raw = original_termios_;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_);
    }
#endif
    terminal_raw_mode_ = enable;
}

KeyCode UI::readKey() {
#ifdef _WIN32
    if (!_kbhit()) {
        return KeyCode::NONE;
    }
    
    int ch = _getch();
    if (ch == 0xE0 || ch == 0x00) {
        // 扩展键
        ch = _getch();
        switch (ch) {
            case 72: return KeyCode::UP;
            case 80: return KeyCode::DOWN;
            case 75: return KeyCode::LEFT;
            case 77: return KeyCode::RIGHT;
            default: return KeyCode::NONE;
        }
    } else if (ch == 13 || ch == 10) {
        return KeyCode::ENTER;
    } else if (ch == 27) {
        return KeyCode::ESC;
    } else if (ch == 'q' || ch == 'Q') {
        return KeyCode::QUIT;
    }
    return KeyCode::NONE;
#else
    // 使用select实现非阻塞读取
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
        return KeyCode::NONE;
    }
    
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return KeyCode::NONE;
    }
    
    // 检查是否是转义序列（方向键）
    if (c == '\033') {
        char seq[2];
        // 设置超时，避免阻塞
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms超时
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
            return KeyCode::ESC;
        }
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KeyCode::ESC;
        if (seq[0] != '[') {
            return KeyCode::ESC;
        }
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
            return KeyCode::NONE;
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KeyCode::NONE;
        
        // 处理可能的修饰符序列（如 [1;2A）
        if (seq[1] >= '0' && seq[1] <= '9') {
            // 跳过修饰符部分
            char ch;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            timeout.tv_usec = 50000; // 50ms超时
            while (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0) {
                if (read(STDIN_FILENO, &ch, 1) == 1) {
                    if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D') {
                        seq[1] = ch;
                        break;
                    }
                }
                FD_ZERO(&readfds);
                FD_SET(STDIN_FILENO, &readfds);
            }
        }
        
        switch (seq[1]) {
            case 'A': return KeyCode::UP;
            case 'B': return KeyCode::DOWN;
            case 'C': return KeyCode::RIGHT;
            case 'D': return KeyCode::LEFT;
            default: return KeyCode::NONE;
        }
    } else if (c == '\n' || c == '\r') {
        return KeyCode::ENTER;
    } else if (c == 'q' || c == 'Q') {
        return KeyCode::QUIT;
    }
    
    return KeyCode::NONE;
#endif
}

bool UI::getMoveInput(int& x, int& y, const Board& board, bool current_player_is_black) {
    int size = board.getSize();
    
    // 初始化光标位置到棋盘中心
    cursor_x_ = size / 2;
    cursor_y_ = size / 2;
    
    // 设置终端原始模式
    setTerminalRawMode(true);
    
    bool highlight = true;
    auto last_flash = std::chrono::steady_clock::now();
    const auto flash_interval = std::chrono::milliseconds(400);
    
    std::cout << "使用方向键移动光标，回车确认落子，Q退出\n";
    
    while (true) {
        // 检查是否被信号中断
        if (g_interrupted) {
            setTerminalRawMode(false);
            std::cout << "\n程序被中断，正在退出..." << std::endl;
            return false;
        }
        
        // 检查闪烁时间
        auto now = std::chrono::steady_clock::now();
        bool should_refresh = false;
        if (now - last_flash >= flash_interval) {
            highlight = !highlight;
            last_flash = now;
            should_refresh = true;
        }
        
        // 读取按键（非阻塞）
        KeyCode key = readKey();
        
        if (key != KeyCode::NONE) {
            should_refresh = true;
            
            switch (key) {
                case KeyCode::UP:
                    if (cursor_y_ > 0) {
                        cursor_y_--;
                    }
                    break;
                case KeyCode::DOWN:
                    if (cursor_y_ < size - 1) {
                        cursor_y_++;
                    }
                    break;
                case KeyCode::LEFT:
                    if (cursor_x_ > 0) {
                        cursor_x_--;
                    }
                    break;
                case KeyCode::RIGHT:
                    if (cursor_x_ < size - 1) {
                        cursor_x_++;
                    }
                    break;
                case KeyCode::ENTER:
                    // 检查位置是否为空
                    if (board.isEmpty(cursor_x_, cursor_y_)) {
                        x = cursor_x_;
                        y = cursor_y_;
                        setTerminalRawMode(false);
                        return true;
                    } else {
                        // 位置已有棋子，显示错误但不退出
                        // 错误信息会在刷新时显示
                    }
                    break;
                case KeyCode::QUIT:
                case KeyCode::ESC:
                    setTerminalRawMode(false);
                    return false;
                default:
                    break;
            }
        }
        
        // 刷新显示
        if (should_refresh) {
            clearScreen();
            displayBoardWithCursor(board, cursor_x_, cursor_y_, highlight);
            displayCurrentPlayer(current_player_is_black);
            std::cout << "使用方向键移动光标，回车确认落子，Q退出\n";
            
            // 如果位置已有棋子，显示提示
            if (!board.isEmpty(cursor_x_, cursor_y_)) {
                std::cout << "[提示] 该位置已有棋子，请选择其他位置\n";
            }
        }
        
        // 短暂休眠以避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int UI::getMenuChoice(int min, int max) {
    int choice;
    while (true) {
        // 检查是否被信号中断
        if (g_interrupted) {
            std::cout << "\n程序被中断，正在退出..." << std::endl;
            std::exit(0);
        }
        
        // 检查 stdin 是否关闭或到达 EOF
        if (std::cin.eof() || std::cin.bad()) {
            std::cout << "\n输入流已关闭，正在退出..." << std::endl;
            std::exit(0);
        }
        
        std::cin >> choice;
        if (std::cin.fail() || choice < min || choice > max) {
            // 如果是因为 EOF 失败，退出
            if (std::cin.eof()) {
                std::cout << "\n输入流已关闭，正在退出..." << std::endl;
                std::exit(0);
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "无效选择，请输入 " << min << "-" << max << ": ";
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
    }
}

int UI::getIntInput(const std::string& prompt, int min, int max) {
    int value;
    std::cout << prompt;
    while (true) {
        // 检查是否被信号中断
        if (g_interrupted) {
            std::cout << "\n程序被中断，正在退出..." << std::endl;
            std::exit(0);
        }
        
        // 检查 stdin 是否关闭或到达 EOF
        if (std::cin.eof() || std::cin.bad()) {
            std::cout << "\n输入流已关闭，正在退出..." << std::endl;
            std::exit(0);
        }
        
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            // 如果是因为 EOF 失败，退出
            if (std::cin.eof()) {
                std::cout << "\n输入流已关闭，正在退出..." << std::endl;
                std::exit(0);
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "无效输入，请输入 " << min << "-" << max << ": ";
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

std::string UI::getStringInput(const std::string& prompt) {
    std::cout << prompt;
    
    // 检查是否被信号中断
    if (g_interrupted) {
        std::cout << "\n程序被中断，正在退出..." << std::endl;
        return "";
    }
    
    // 检查 stdin 是否关闭或到达 EOF
    if (std::cin.eof() || std::cin.bad()) {
        std::cout << "\n输入流已关闭，正在退出..." << std::endl;
        return "";
    }
    
    std::string input;
    std::getline(std::cin, input);
    
    // 如果 getline 因为 EOF 失败
    if (std::cin.eof()) {
        std::cout << "\n输入流已关闭，正在退出..." << std::endl;
        return "";
    }
    
    return input;
}

void UI::waitForEnter() {
    std::cout << "\n按回车键继续...";
    
    // 检查是否被信号中断
    if (g_interrupted) {
        return;
    }
    
    // 检查 stdin 是否关闭或到达 EOF
    if (std::cin.eof() || std::cin.bad()) {
        return;
    }
    
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
