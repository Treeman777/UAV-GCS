#include "r_map.h"
#include <QWebEnginePage>

RMap::RMap(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);//0边缘留空

    // 2. 实例化 Web 引擎并加入布局
    webView = new QWebEngineView(this);
    layout->addWidget(webView);

    // 3. 构建内嵌的 HTML 与 高德地图 JS 代码
    QString html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8">
            <title>GCS Map</title>
            <style>
                html, body, #container { width: 100%; height: 100%; margin: 0; padding: 0; background-color: #282828;}
            </style>
            <script src="https://webapi.amap.com/maps?v=2.0&key=d58f8e91b49c8328e5e108de7967f589"></script>
        </head>
        <body>
            <div id="container"></div>
            <script>
                // 初始化地图
                var map = new AMap.Map('container', {
                    zoom: 16,
                    center: [116.397428, 39.90923], // 默认初始坐标，收到真实GPS后会自动跳转
                    mapStyle: 'amap://styles/dark'  // 战术地面站推荐暗色主题
                });

                var planeMarker = null; // 飞机图标
                var trackPath = [];     // 飞行轨迹数组
                var polyline = new AMap.Polyline({
                    map: map,
                    strokeColor: "#FF0000", // 红色轨迹
                    strokeWeight: 4,        // 轨迹粗细
                    lineJoin: 'round'
                });

                // 【核心函数】：留给 C++ 随时调用的接口
                function updateGPS(lon, lat) {
                    var currentPos = new AMap.LngLat(lon, lat);
                    
                    if (!planeMarker) {
                        // 第一次收到数据，创建标记
                        planeMarker = new AMap.Marker({
                            position: currentPos,
                            map: map
                        });
                        map.setCenter(currentPos); // 视角切过去
                    } else {
                        // 后续收到数据，平滑移动标记
                        planeMarker.setPosition(currentPos);
                        map.setCenter(currentPos); // 视角跟随
                    }
                    
                    // 将新坐标点加入轨迹并重绘线条
                    trackPath.push(currentPos);
                    polyline.setPath(trackPath);
                }
            </script>
        </body>
        </html>
    )";

    // 4. 将 HTML 代码直接加载进浏览器控件
    webView->setHtml(html);
}

RMap::~RMap() {
    // 资源清理交由 Qt 对象树自动管理
}

// 供主程序调用的刷新接口
void RMap::updatePosition(float lat, float lon) {
    // 高德 API 要求的参数顺序是：经度 (lon) 在前，纬度 (lat) 在后
    // 拼接成我们要执行的 JS 代码语句，例如: "updateGPS(116.39, 39.90);"
    QString jsCode = QString("updateGPS(%1, %2);").arg(lon).arg(lat);
    
    // 让内嵌的网页去执行这段脚本，实现无刷新、丝滑的地图更新
    if (webView && webView->page()) {
        webView->page()->runJavaScript(jsCode);
    }
}