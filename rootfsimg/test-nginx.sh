      
#!/bin/bash

# 设置HTML文件存放目录
HTML_DIR="/etc/nginx/html"

# 定义文件大小（字节）和对应的文件名前缀/后缀
FILE_SIZES=(262144 0 1024 4096 16384 65536)
FILE_SUFFIXES=("256k" "0k" "1k" "4k" "16k" "64k")

# ApacheBench (ab) 的基础URL和通用选项
AB_URL_BASE="https://127.0.0.1/old"
AB_COMMON_OPTS="-n 1000 -c 10 -H \"Accept-Encoding: gzip, deflate\""

# 定义 Nginx 测试版本及其启动命令
NGINX_VERSIONS=(
    "nginx-openssl-pcre-zlib"
    "nginx-openssl-pcre"
    "nginx-openssl"
    "nginx-no-dasics"
)

echo "=== 脚本开始 ==="

# --- 文件生成 ---
echo "=== 正在生成测试文件至 ${HTML_DIR} ==="

for i in "${!FILE_SIZES[@]}"; do
    size=${FILE_SIZES[$i]}
    suffix=${FILE_SUFFIXES[$i]}
    filename="${HTML_DIR}/file_${suffix}"
    echo "生成文件 ${filename} (${size} 字节)..."
    dd if=/dev/zero of="$filename" bs=1 count="$size" conv=notrunc 2>/dev/null
done
echo "=== 测试文件生成完毕 ==="
echo ""

# --- Nginx 测试循环 ---
for i in "${!NGINX_VERSIONS[@]}"; do
    nginx_cmd=${NGINX_VERSIONS[$i]}
    echo "=== 测试 ${nginx_cmd} ==="

    for suffix in "${FILE_SUFFIXES[@]}"; do
        # 启动 Nginx
        if [ $i -ne 3 ]; then
            $nginx_cmd -c /etc/nginx/nginx.conf -dasics &
        else
            $nginx_cmd -c /etc/nginx/nginx.conf &
        fi
        sleep 1

        url="${AB_URL_BASE}/file_${suffix}"
        echo "ab ${AB_COMMON_OPTS} ${url} > ${nginx_cmd}_${suffix}.log"
        ab "${AB_COMMON_OPTS}" "${url}" > "${nginx_cmd}_${suffix}.log"

        # 杀死当前 Nginx 进程
        pkill "$nginx_cmd" || true
        sleep 1
    done

done

echo "=== 脚本结束 ==="

    