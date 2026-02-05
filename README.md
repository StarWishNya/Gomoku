# 联机五子棋游戏

一个基于C++的联机五子棋Shell游戏，支持点对点TCP连接、可配置棋盘大小、多种游戏模式，并使用最小化Docker镜像打包。

## 功能特性

- ✅ **点对点连接**：支持TCP服务器/客户端模式
- ✅ **灵活配置**：
  - 标准/拓展棋盘大小（10x10 到 25x25）
  - 自由/随机选择先后手
  - 竞技/无规则模式
- ✅ **游戏规则**：
  - 标准模式：无禁手规则
  - 竞技模式：支持禁手规则（双三、双四、长连）
- ✅ **最小化Docker镜像**：使用多阶段构建，基于Alpine Linux

## 系统要求

- C++17 编译器（GCC 7+ 或 Clang 5+）
- CMake 3.12+
- Linux/macOS/Windows（需要支持POSIX socket或Winsock）

## 本地构建

### 1. 克隆项目

```bash
git clone <repository-url>
cd Gomoku
```

### 2. 编译项目

```bash
mkdir build
cd build
cmake ..
make
```

### 3. 运行游戏

**服务器模式（等待连接）：**
```bash
./gomoku
# 选择 1（启动服务器）
# 配置游戏参数
```

**客户端模式（连接到服务器）：**
```bash
./gomoku
# 选择 2（连接到服务器）
# 输入服务器IP地址（如：127.0.0.1）
# 配置游戏参数
```

## Docker构建和运行

### 构建Docker镜像

```bash
docker build -t gomoku:latest .
```

### 运行服务器容器

```bash
docker run -it --rm -p 8888:8888 gomoku:latest
# 选择 1（启动服务器）
```

### 运行客户端容器

**方式一：客户端使用主机网络（推荐，与宿主机/其他容器互通）**
```bash
docker run -it --rm --network host gomoku:latest
# 选择 2（连接到服务器）
# 输入 127.0.0.1（服务器在本机或本机端口已映射时）
```

**方式二：客户端与服务器在同一 Docker 网络**
若服务器和客户端都在容器内，且未使用 `--network host`，则客户端不能填 `127.0.0.1`（会连到本容器）。应：
- 使用 `docker network create gomoku && docker run --network gomoku ...` 让两容器同网段，客户端填写**服务器容器的 IP 或容器名**；或
- 服务器映射端口后，客户端用 `--network host` 并连接 `127.0.0.1:端口`。

### 或者使用docker-compose

创建 `docker-compose.yml`：

```yaml
version: '3.8'

services:
  server:
    build: .
    ports:
      - "8888:8888"
    stdin_open: true
    tty: true
    
  client:
    build: .
    depends_on:
      - server
    stdin_open: true
    tty: true
    network_mode: "host"
```

运行：
```bash
docker-compose up
```

## 游戏使用说明

### 主菜单

1. **启动服务器（等待连接）**：作为服务器等待客户端连接
2. **连接到服务器**：作为客户端连接到指定IP的服务器
3. **退出**：退出游戏

### 配置菜单

- **棋盘大小**：设置棋盘大小（10-25）
- **游戏模式**：
  - 无规则模式：标准五子棋规则
  - 竞技模式：包含禁手规则（仅对黑棋生效）
- **先后手选择**：
  - 先手（黑棋）
  - 后手（白棋）
  - 随机

### 游戏操作

- **输入坐标**：格式为 `x y`（例如：`8 8` 表示第8行第8列）
- **退出游戏**：输入 `q` 或 `quit`

### 游戏规则

#### 标准模式
- 黑棋和白棋轮流落子
- 先形成五连的一方获胜

#### 竞技模式（禁手规则）
- 黑棋有禁手限制：
  - **双三禁手**：不能同时形成两个活三
  - **双四禁手**：不能同时形成两个四（活四或冲四）
  - **长连禁手**：不能形成超过5子的连子
- 白棋无禁手限制
- 如果黑棋下出禁手，需要重新落子

## 项目结构

```
Gomoku/
├── src/
│   ├── main.cpp           # 主程序入口
│   ├── config.hpp/cpp     # 配置管理
│   ├── game.hpp/cpp       # 游戏逻辑（棋盘、规则判断）
│   ├── network.hpp/cpp    # 网络通信（TCP）
│   └── ui.hpp/cpp         # 用户界面
├── CMakeLists.txt         # CMake构建配置
├── Dockerfile             # Docker多阶段构建
├── .dockerignore          # Docker忽略文件
└── README.md              # 项目文档
```

## 技术实现

- **网络通信**：TCP Socket，JSON消息协议
- **游戏逻辑**：C++17，面向对象设计
- **构建系统**：CMake
- **依赖库**：
  - nlohmann/json（header-only，自动下载）
  - 标准库（thread, socket等）

## 网络协议

消息格式（JSON）：
- `CONFIG`: 游戏配置同步
- `MOVE`: 落子信息 `{"x": int, "y": int, "player": int}`
- `STATE`: 游戏状态
- `RESULT`: 游戏结果
- `ERROR`: 错误消息

## 故障排除

### 连接失败
- 检查防火墙设置
- 确认服务器IP地址和端口正确
- 确保服务器已启动并监听

### 编译错误
- 确保C++17编译器已安装
- 检查CMake版本（需要3.12+）
- 确保网络连接正常（需要下载nlohmann/json）

### Docker构建失败
- 检查Docker版本（需要支持多阶段构建）
- 确保有足够的磁盘空间
- 检查网络连接（需要下载Alpine和GCC镜像）

## 许可证

本项目采用MIT许可证。

## 贡献

欢迎提交Issue和Pull Request！
