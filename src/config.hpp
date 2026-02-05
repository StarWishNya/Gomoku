#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

enum class GameMode {
    STANDARD,    // 无规则模式
    COMPETITIVE  // 竞技模式（有禁手）
};

enum class PlayerOrder {
    FIRST,       // 先手
    SECOND,      // 后手
    RANDOM       // 随机
};

struct GameConfig {
    int board_size = 15;           // 棋盘大小（默认15x15）
    GameMode game_mode = GameMode::STANDARD;  // 游戏模式
    PlayerOrder player_order = PlayerOrder::RANDOM;  // 先后手选择
    int port = 8888;               // 默认端口
    std::string server_ip = "127.0.0.1";  // 默认服务器IP
    
    // 验证配置有效性
    bool isValid() const {
        return board_size >= 10 && board_size <= 25 && port > 1024 && port < 65536;
    }
    
    // 获取模式字符串
    std::string getModeString() const {
        return game_mode == GameMode::STANDARD ? "无规则" : "竞技";
    }
    
    // 获取先后手字符串
    std::string getOrderString() const {
        switch (player_order) {
            case PlayerOrder::FIRST: return "先手";
            case PlayerOrder::SECOND: return "后手";
            case PlayerOrder::RANDOM: return "随机";
            default: return "未知";
        }
    }
};

#endif // CONFIG_HPP
