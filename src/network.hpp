#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET_VALUE -1
    #define CLOSE_SOCKET close
#endif

// 消息类型枚举
enum class MessageType {
    CONFIG = 0,
    MOVE = 1,
    ERROR = 2
};

// 消息结构
struct Message {
    MessageType type;
    std::string data;
    
    Message() : type(MessageType::ERROR), data("") {}
    Message(MessageType t, const std::string& d) : type(t), data(d) {}
};

// 网络管理器类
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // 启动服务器（等待客户端连接）
    bool startServer(int port);
    
    // 连接到服务器
    bool connectToServer(const std::string& ip, int port);
    
    // 发送消息
    bool sendMessage(const Message& msg);
    
    // 接收消息
    bool receiveMessage(Message& msg);
    
    // 关闭连接
    void closeConnection();
    
    // 获取最后的错误信息
    const std::string& getLastError() const {
        return last_error_;
    }
    
    // 检查是否已连接
    bool isConnected() const {
        return connected_;
    }

private:
    // 初始化socket
    bool initializeSocket();
    
    // 清理资源
    void cleanup();
    
    // 序列化消息
    std::string serializeMessage(const Message& msg);
    
    // 反序列化消息
    bool deserializeMessage(const std::string& json_str, Message& msg);
    
    SocketType server_socket_;
    SocketType client_socket_;
    bool is_server_;
    bool connected_;
    std::string last_error_;
};

#endif // NETWORK_HPP
