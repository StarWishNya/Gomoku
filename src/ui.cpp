#include "ui.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <csignal>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

// 全局标志，用于信号处理
static volatile sig_atomic_t g_interrupted = 0;

static void signalHandler(int signal) {
    g_interrupted = 1;
    // 设置默认处理，以便下次信号时强制退出
    std::signal(signal, SIG_DFL);
}

UI::UI() {
    // 注册信号处理器
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
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
            CellState cell = board.getCell(x, y);
            std::cout << " " << getStoneChar(cell) << " ";
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

bool UI::getMoveInput(int& x, int& y, const Board& board) {
    std::cout << "请输入坐标 (格式: x y 或 q 退出): ";
    
    // 检查是否被信号中断
    if (g_interrupted) {
        std::cout << "\n程序被中断，正在退出..." << std::endl;
        return false;
    }
    
    // 检查 stdin 是否关闭或到达 EOF
    if (std::cin.eof() || std::cin.bad()) {
        std::cout << "\n输入流已关闭，正在退出..." << std::endl;
        return false;
    }
    
    std::string input;
    std::getline(std::cin, input);
    
    // 如果 getline 因为 EOF 失败
    if (std::cin.eof()) {
        std::cout << "\n输入流已关闭，正在退出..." << std::endl;
        return false;
    }
    
    // 检查退出
    if (input == "q" || input == "Q" || input == "quit") {
        return false;
    }
    
    // 解析坐标
    std::istringstream iss(input);
    int input_x, input_y;
    
    if (iss >> input_x >> input_y) {
        // 转换为0-based索引
        x = input_x - 1;
        y = input_y - 1;
        
        // 验证坐标
        if (board.isValidPosition(x, y)) {
            return true;
        } else {
            displayError("坐标超出范围！");
            return false;
        }
    } else {
        displayError("无效的输入格式！请使用: x y");
        return false;
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
