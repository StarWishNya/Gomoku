#include "network.hpp"
#include <nlohmann/json.hpp>
#include <cstring>
#include <iostream>
#include <cerrno>

using json = nlohmann::json;

NetworkManager::NetworkManager() 
    : server_socket_(INVALID_SOCKET_VALUE), 
      client_socket_(INVALID_SOCKET_VALUE),
      is_server_(false), 
      connected_(false) {
    initializeSocket();
}

NetworkManager::~NetworkManager() {
    cleanup();
}

bool NetworkManager::initializeSocket() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        last_error_ = "WSAStartup failed";
        return false;
    }
#endif
    return true;
}

void NetworkManager::cleanup() {
    closeConnection();
    
    if (server_socket_ != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkManager::startServer(int port) {
    cleanup();
    is_server_ = true;
    
    // 创建socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == INVALID_SOCKET_VALUE) {
        last_error_ = "Failed to create socket";
        return false;
    }
    
    // 设置socket选项（允许地址重用）
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // 绑定地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        last_error_ = "Failed to bind socket (port may be in use or access denied)";
#else
        last_error_ = std::string("Failed to bind socket: ") + std::strerror(errno) +
            " (port " + std::to_string(port) + ")";
#endif
        CLOSE_SOCKET(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    // 监听
    if (listen(server_socket_, 1) < 0) {
        last_error_ = "Failed to listen";
        CLOSE_SOCKET(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    std::cout << "服务器已启动，等待客户端连接... (端口: " << port << ")" << std::endl;
    
    // 接受连接
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    client_socket_ = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    
    if (client_socket_ == INVALID_SOCKET_VALUE) {
        last_error_ = "Failed to accept connection";
        CLOSE_SOCKET(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    connected_ = true;
    std::cout << "客户端已连接: " << inet_ntoa(client_addr.sin_addr) << std::endl;
    
    // 关闭监听socket
    CLOSE_SOCKET(server_socket_);
    server_socket_ = INVALID_SOCKET_VALUE;
    
    return true;
}

bool NetworkManager::connectToServer(const std::string& ip, int port) {
    cleanup();
    is_server_ = false;
    
    // 创建socket
    client_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_ == INVALID_SOCKET_VALUE) {
        last_error_ = "Failed to create socket";
        return false;
    }
    
    // 连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        last_error_ = "Invalid IP address";
        CLOSE_SOCKET(client_socket_);
        client_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    std::cout << "正在连接到服务器 " << ip << ":" << port << "..." << std::endl;
    
    if (connect(client_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        last_error_ = "Failed to connect to server (check IP/port and firewall)";
#else
        last_error_ = std::string("连接失败: ") + std::strerror(errno);
        if (errno == ECONNREFUSED && (ip == "127.0.0.1" || ip == "localhost")) {
            last_error_ += "。若服务器在另一台机器或 Docker 容器中，请使用其 IP 而非 127.0.0.1";
        }
#endif
        CLOSE_SOCKET(client_socket_);
        client_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    connected_ = true;
    std::cout << "已连接到服务器" << std::endl;
    
    return true;
}

bool NetworkManager::sendMessage(const Message& msg) {
    if (!connected_ || client_socket_ == INVALID_SOCKET_VALUE) {
        last_error_ = "Not connected";
        return false;
    }
    
    std::string json_str = serializeMessage(msg);
    if (json_str.empty()) {
        return false;
    }
    
    // 先发送消息长度（4字节）
    uint32_t length = htonl(static_cast<uint32_t>(json_str.length()));
    if (send(client_socket_, (char*)&length, sizeof(length), 0) != sizeof(length)) {
        last_error_ = "Failed to send message length";
        return false;
    }
    
    // 发送消息内容
    int sent = 0;
    int total = json_str.length();
    while (sent < total) {
        int n = send(client_socket_, json_str.c_str() + sent, total - sent, 0);
        if (n < 0) {
            last_error_ = "Failed to send message";
            return false;
        }
        sent += n;
    }
    
    return true;
}

bool NetworkManager::receiveMessage(Message& msg) {
    if (!connected_ || client_socket_ == INVALID_SOCKET_VALUE) {
        last_error_ = "Not connected";
        return false;
    }
    
    // 接收消息长度
    uint32_t length;
    int received = recv(client_socket_, (char*)&length, sizeof(length), 0);
    if (received != sizeof(length)) {
        if (received == 0) {
            last_error_ = "Connection closed by peer";
            connected_ = false;
        } else {
            last_error_ = "Failed to receive message length";
        }
        return false;
    }
    
    length = ntohl(length);
    if (length > 1024 * 1024) {  // 限制最大消息大小为1MB
        last_error_ = "Message too large";
        return false;
    }
    
    // 接收消息内容
    std::string json_str(length, '\0');
    received = 0;
    while (received < length) {
        int n = recv(client_socket_, &json_str[received], length - received, 0);
        if (n <= 0) {
            if (n == 0) {
                last_error_ = "Connection closed by peer";
                connected_ = false;
            } else {
                last_error_ = "Failed to receive message";
            }
            return false;
        }
        received += n;
    }
    
    return deserializeMessage(json_str, msg);
}

void NetworkManager::closeConnection() {
    if (client_socket_ != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(client_socket_);
        client_socket_ = INVALID_SOCKET_VALUE;
    }
    connected_ = false;
}

std::string NetworkManager::serializeMessage(const Message& msg) {
    try {
        json j;
        j["type"] = static_cast<int>(msg.type);
        j["data"] = msg.data;
        return j.dump();
    } catch (const std::exception& e) {
        last_error_ = "Failed to serialize message: " + std::string(e.what());
        return "";
    }
}

bool NetworkManager::deserializeMessage(const std::string& json_str, Message& msg) {
    try {
        json j = json::parse(json_str);
        msg.type = static_cast<MessageType>(j["type"].get<int>());
        msg.data = j["data"].get<std::string>();
        return true;
    } catch (const std::exception& e) {
        last_error_ = "Failed to deserialize message: " + std::string(e.what());
        return false;
    }
}
