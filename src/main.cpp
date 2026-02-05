#include "config.hpp"
#include "game.hpp"
#include "network.hpp"
#include "ui.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 游戏主循环
void runGame(GameConfig& config, NetworkManager& network, UI& ui, bool is_server) {
    // 创建游戏实例
    Game game(config);
    
    // 同步配置和确定先后手
    bool is_black = false;
    
    if (is_server) {
        // 服务器端：决定配置并发送给客户端
        if (config.player_order == PlayerOrder::FIRST) {
            is_black = true;
        } else if (config.player_order == PlayerOrder::SECOND) {
            is_black = false;
        } else {  // RANDOM
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 1);
            is_black = (dis(gen) == 0);
        }
        
        // 发送配置给客户端（server_is_black 明确表示“服务器”是否执黑）
        json config_msg;
        config_msg["board_size"] = config.board_size;
        config_msg["game_mode"] = static_cast<int>(config.game_mode);
        config_msg["server_is_black"] = is_black;
        
        Message msg(MessageType::CONFIG, config_msg.dump());
        if (!network.sendMessage(msg)) {
            ui.displayError("发送配置失败: " + network.getLastError());
            return;
        }
    } else {
        // 客户端：接收服务器配置
        Message msg;
        if (!network.receiveMessage(msg) || msg.type != MessageType::CONFIG) {
            ui.displayError("接收配置失败: " + network.getLastError());
            return;
        }
        
        json config_json = json::parse(msg.data);
        config.board_size = config_json["board_size"];
        config.game_mode = static_cast<GameMode>(config_json["game_mode"]);
        // 协议：server_is_black 表示服务器是否执黑，客户端执另一方
        bool server_is_black = config_json.contains("server_is_black")
            ? config_json["server_is_black"].get<bool>()
            : config_json["is_black"].get<bool>();
        is_black = !server_is_black;
        
        // 更新游戏配置
        game = Game(config);
    }
    
    // 开局始终为黑方先手（只用 is_black 表示“本方是否执黑”，不改变回合）
    game.setCurrentPlayer(true);
    
    ui.clearScreen();
    ui.displayInfo("游戏开始！");
    ui.displayInfo(std::string("你是 ") + (is_black ? "黑棋 (X)" : "白棋 (O)"));
    ui.waitForEnter();
    
    // 游戏主循环
    while (!game.isGameOver()) {
        bool my_turn = (game.getCurrentPlayer() == is_black);
        
        if (my_turn) {
            // 我的回合 - getMoveInput内部会处理显示
            int x, y;
            if (!ui.getMoveInput(x, y, game.getBoard(), game.getCurrentPlayer())) {
                // 用户选择退出
                ui.displayInfo("游戏已退出");
                return;
            }
            
            GameResult result = game.makeMove(x, y);
            
            if (result == GameResult::FORBIDDEN_MOVE) {
                ui.displayResult(result);
                ui.waitForEnter();
                continue;
            }
            
            // 发送落子信息给对手
            json move_msg;
            move_msg["x"] = x;
            move_msg["y"] = y;
            move_msg["player"] = is_black ? 1 : 2;
            
            Message msg(MessageType::MOVE, move_msg.dump());
            if (!network.sendMessage(msg)) {
                ui.displayError("发送消息失败: " + network.getLastError());
                return;
            }
            
            if (result != GameResult::NONE) {
                ui.displayResult(result);
                break;
            }
        } else {
            // 对手的回合
            ui.displayInfo("等待对手落子...");
            
            Message msg;
            if (!network.receiveMessage(msg)) {
                ui.displayError("接收消息失败: " + network.getLastError());
                return;
            }
            
            if (msg.type == MessageType::MOVE) {
                json move_json = json::parse(msg.data);
                int x = move_json["x"];
                int y = move_json["y"];
                
                GameResult result = game.makeMove(x, y);
                
                if (result != GameResult::NONE) {
                    ui.clearScreen();
                    ui.displayBoard(game.getBoard());
                    ui.displayResult(result);
                    break;
                }
            } else if (msg.type == MessageType::ERROR) {
                ui.displayError("对手发送错误消息: " + msg.data);
                return;
            }
        }
    }
    
    ui.waitForEnter();
}

int main(int argc, char* argv[]) {
    std::srand(std::time(nullptr));
    
    UI ui;
    GameConfig config;
    
    // 显示主菜单
    while (true) {
        ui.showMainMenu();
        int choice = ui.getMenuChoice(1, 3);
        
        if (choice == 3) {
            std::cout << "再见！" << std::endl;
            return 0;
        }
        
        NetworkManager network;
        bool is_server = (choice == 1);
        
        if (is_server) {
            // 服务器模式：先显示配置菜单（可在菜单中设置端口），再启动
            ui.showConfigMenu(config);
            int port = config.port;
            if (argc > 2) {
                port = std::atoi(argv[2]);
            }
            if (!network.startServer(port)) {
                ui.displayError("启动服务器失败: " + network.getLastError());
                ui.waitForEnter();
                continue;
            }
        } else {
            // 客户端模式
            std::string ip = config.server_ip;
            int port = config.port;
            
            if (argc > 2) {
                ip = argv[2];
            }
            if (argc > 3) {
                port = std::atoi(argv[3]);
            } else {
                ip = ui.getStringInput("请输入服务器IP地址 (默认: 127.0.0.1): ");
                if (ip.empty()) {
                    ip = "127.0.0.1";
                }
            }
            
            if (!network.connectToServer(ip, port)) {
                ui.displayError("连接服务器失败: " + network.getLastError());
                ui.waitForEnter();
                continue;
            }
            
            // 客户端接收配置（如果服务器发送了配置）
            // 这里简化处理，客户端也显示配置菜单
            ui.showConfigMenu(config);
        }
        
        // 验证配置
        if (!config.isValid()) {
            ui.displayError("配置无效！");
            ui.waitForEnter();
            continue;
        }
        
        // 开始游戏
        runGame(config, network, ui, is_server);
        
        network.closeConnection();
    }
    
    return 0;
}
