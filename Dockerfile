# 多阶段构建 - 编译阶段
FROM gcc:12 AS builder

# 安装构建依赖
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /build

# 复制源代码
COPY . .

# 创建构建目录并编译
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC_BUILD=ON && \
    make -j$(nproc)

# 运行时阶段 - 使用最小化的Alpine镜像
FROM alpine:latest

# 安装运行时依赖（如果需要）
# 对于静态链接的可执行文件，可能不需要任何依赖

# 创建非root用户
RUN addgroup -g 1000 gomoku && \
    adduser -D -u 1000 -G gomoku gomoku

# 设置工作目录
WORKDIR /app

# 从构建阶段复制可执行文件
COPY --from=builder /build/build/gomoku /app/gomoku

# 更改所有权
RUN chown -R gomoku:gomoku /app

# 切换到非root用户
USER gomoku

# 暴露端口（默认8888）
EXPOSE 8888

# 设置入口点
ENTRYPOINT ["./gomoku"]
