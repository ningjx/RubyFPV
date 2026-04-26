# Renderer 渲染引擎

## 1. 模块概述

`code/renderer/` 目录包含 RubyFPV 的渲染引擎实现，负责用户界面的图形渲染。支持多种渲染后端以适配不同的硬件平台。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `render_engine.h/render_engine.cpp` | 渲染引擎基类，定义通用接口 |
| `render_engine_cairo.h/render_engine_cairo.cpp` | Cairo 渲染后端 (Radxa) |
| `render_engine_raw.h/render_engine_raw.cpp` | 原始帧缓冲渲染后端 (Pi) |
| `render_engine_ovg.h/render_engine_ovg.cpp` | OpenVG 渲染后端 (Pi) |
| `render_engine_ui.cpp` | UI渲染辅助函数 |
| `fbgraphics.h/fbgraphics.c` | 原始帧缓冲图形库 |
| `fbg_dispmanx.h/fbg_dispmanx.c` | Dispmanx 后端 (Pi) |
| `drm_core.h/drm_core.c` | DRM (Direct Rendering Manager) 核心 |
| `lodepng.h/lodepng.c` | PNG 图像解码库 |
| `nanojpeg.c` | JPEG 图像解码库 |
| `cairo_mpp.txt` | Cairo MPP 配置说明 |

---

## 2. 渲染引擎架构

### 2.1 架构概览

```
┌─────────────────────────────────────────────────────────────────────┐
│                        渲染引擎架构                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    RenderEngine (基类)                       │  │
│   │                                                             │  │
│   │  通用接口:                                                   │  │
│   │  - 初始化/清理                                               │  │
│   │  - 屏幕信息获取                                              │  │
│   │  - 颜色设置                                                  │  │
│   │  - 字体管理                                                  │  │
│   │  - 图像/图标管理                                             │  │
│   │  - 绘图操作                                                  │  │
│   │  - 文本渲染                                                  │  │
│   │                                                             │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              │ 继承                                 │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │RenderEngine│    │RenderEngine│    │RenderEngine│              │
│   │   Cairo    │    │    Raw     │    │    OVG     │              │
│   │  (Radxa)   │    │   (Pi)     │    │   (Pi)     │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │   Cairo    │    │  fbgraphics│    │  OpenVG    │              │
│   │   + DRM    │    │ + Dispmanx │    │            │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 平台适配

| 平台 | 渲染后端 | 底层库 | 特点 |
|------|----------|--------|------|
| **Raspberry Pi** | `RenderEngineRaw` | fbgraphics + Dispmanx | 直接帧缓冲操作，高效 |
| **Raspberry Pi** | `RenderEngineOVG` | OpenVG | 矢量图形，高质量 |
| **Radxa** | `RenderEngineCairo` | Cairo + DRM | 矢量图形，跨平台 |

---

## 3. RenderEngine 基类 (render_engine.h)

### 3.1 类定义

```cpp
class RenderEngine
{
   public:
     RenderEngine();
     virtual ~RenderEngine();

     // 初始化
     virtual bool initEngine();
     virtual bool uninitEngine();

     // 屏幕信息
     int getScreenWidth();
     int getScreenHeight();
     float getAspectRatio();
     float getPixelWidth();
     float getPixelHeight();

     // 绘图上下文
     virtual void* getDrawContext();

     // Alpha混合
     float setGlobalAlfa(float alfa);
     float getGlobalAlfa();
     bool isAlphaBlendingEnabled();
     void setAlphaBlendingEnabled(bool bEnable);
     void enableAlphaBlending();
     void disableAlphaBlending();

     // 颜色设置
     virtual void setColors(const double* color);
     virtual void setFill(const double* pColor);
     virtual void setFill(float r, float g, float b, float a);
     virtual void setStroke(const double* color);
     virtual void setStroke(float r, float g, float b, float a);
     virtual void setStrokeSize(float fStrokeSize);

     // 字体管理
     virtual int loadRawFont(int iFamilyId, const char* szFontFile, int iBold);
     virtual void freeRawFont(u32 idFont);
     virtual void setFontOutlineColor(u32 idFont, u8 r, u8 g, u8 b, u8 a);
     virtual float getRawFontHeight(u32 fontId);
     virtual float textHeight(u32 fontId);
     virtual float textWidth(u32 fontId, const char* szText);

     // 图像管理
     virtual u32 loadImage(const char* szFile);
     virtual void freeImage(u32 idImage);
     virtual u32 loadIcon(const char* szFile);
     virtual void freeIcon(u32 idIcon);
     virtual int getImageWidth(u32 uImageId);
     virtual int getImageHeight(u32 uImageId);

     // 帧控制
     virtual void startFrame();
     virtual void endFrame();
     virtual bool isFrameStarted();

     // 绘图操作
     virtual void drawLine(float x1, float y1, float x2, float y2);
     virtual void drawRect(float xPos, float yPos, float fWidth, float fHeight);
     virtual void drawRoundRect(float xPos, float yPos, float fWidth, float fHeight, float fCornerRadius);
     virtual void fillCircle(float x, float y, float r);
     virtual void drawCircle(float x, float y, float r);
     virtual void drawArc(float x, float y, float r, float a1, float a2);

     // 文本渲染
     virtual void drawText(float xPos, float yPos, u32 uFontId, const char* szText);
     virtual void drawTextLeft(float xPos, float yPos, u32 fontId, const char* szText);
     virtual void drawTextScaled(float xPos, float yPos, u32 fontId, float fScale, const char* szText);

     // 图像绘制
     virtual void drawImage(float xPos, float yPos, float fWidth, float fHeight, u32 imageId);
     virtual void drawIcon(float xPos, float yPos, float fWidth, float fHeight, u32 iconId);
};
```

### 3.2 字体结构

```cpp
#define MAX_FONT_CHARS 256
#define MAX_FONT_KERINGS 1024
#define MAX_RAW_FONTS 100
#define MAX_RAW_IMAGES 100
#define MAX_RAW_ICONS 100

typedef struct
{
   int charId;
   int imgXOffset;    // 字符在字体图像中的X偏移
   int imgYOffset;    // 字符在字体图像中的Y偏移
   int width;         // 字符宽度
   int height;        // 字符高度
   int xOffset;       // 渲染X偏移
   int yOffset;       // 渲染Y偏移
   int xAdvance;      // 渲染后X前进距离
} RenderEngineRawFontChar;

typedef struct
{
   char szName[128];          // 字体名称
   int iFamilyId;             // 字体家族ID
   bool bBold;                // 是否粗体
   void* pImageObject;        // 字体图像对象
   int lineHeight;            // 行高
   int baseLine;              // 基线
   int charCount;             // 字符数量
   int charIdFirst;           // 首字符ID
   int charIdLast;            // 末字符ID
   RenderEngineRawFontChar chars[MAX_FONT_CHARS];  // 字符数据

   int keringsCount;          // 字距调整数量
   int kerings[MAX_FONT_KERINGS][3];  // 字距调整数据

   float dxLetters;           // 字符间距调整
} RenderEngineRawFont;
```

---

## 4. Cairo 渲染后端 (render_engine_cairo.h)

### 4.1 Cairo 后端特点

Cairo 是一个跨平台的矢量图形库，支持多种输出后端：
- DRM (Direct Rendering Manager)
- X11
- PNG
- PDF

### 4.2 RenderEngineCairo 类

```cpp
class RenderEngineCairo: public RenderEngine
{
   public:
     RenderEngineCairo();
     virtual ~RenderEngineCairo();

     virtual void* getDrawContext();
     virtual void setStroke(const double* color, float fStrokeSize); 
     virtual void setStrokeSize(float fStrokeSize);

     // 帧控制
     virtual void startFrame();
     virtual void endFrame();
     virtual void rotate180();

     // 绘图操作
     virtual void drawLine(float x1, float y1, float x2, float y2);
     virtual void drawRect(float xPos, float yPos, float fWidth, float fHeight);
     virtual void drawRoundRect(float xPos, float yPos, float fWidth, float fHeight, float fCornerRadius);
     virtual void fillCircle(float x, float y, float r);
     virtual void drawCircle(float x, float y, float r);
     virtual void drawArc(float x, float y, float r, float a1, float a2);

   protected:
      cairo_t* _createTempDrawContext();
      cairo_t* _getActiveCairoContext();

      bool m_bUseDoubleBuffering;           // 双缓冲
      u32 m_uRenderDrawSurfacesIds[2];      // 渲染表面ID
      cairo_surface_t *m_pMainCairoSurface[2];  // Cairo表面
      cairo_t* m_pCairoCtx;                 // Cairo上下文
      cairo_t* m_pCairoTempCtx;             // 临时Cairo上下文

      cairo_surface_t* m_pImages[MAX_RAW_IMAGES];  // 图像表面
      cairo_surface_t* m_pIcons[MAX_RAW_ICONS];    // 图标表面
};
```

### 4.3 Cairo 渲染流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Cairo 渲染流程                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 初始化                                                          │
│     │                                                               │
│     ├─► DRM设备打开                                                 │
│     ├─► 创建Cairo DRM表面                                           │
│     ├─► 创建Cairo上下文                                             │
│     └─► 加载字体和图像                                              │
│                                                                     │
│  2. 帧渲染                                                          │
│     │                                                               │
│     ├─► startFrame()                                                │
│     │     ├─► 清除缓冲区                                            │
│     │     └─► 准备绘图上下文                                        │
│     │                                                               │
│     ├─► 绘图操作                                                    │
│     │     ├─► setFill/setStroke 设置颜色                            │
│     │     ├─► drawRect/drawLine 绘制形状                            │
│     │     ├─► drawText 绘制文本                                     │
│     │     ├─► drawImage 绘制图像                                    │
│     │     └─► ...                                                   │
│     │                                                               │
│     ├─► endFrame()                                                  │
│     │     ├─► 完成Cairo绘图                                         │
│     │     ├─► 刷新DRM表面                                           │
│     │     └─► 交换缓冲区                                            │
│     │                                                               │
│  3. 清理                                                            │
│     │                                                               │
│     ├─► 销毁Cairo上下文                                             │
│     ├─► 销毁Cairo表面                                               │
│     ├─► 关闭DRM设备                                                 │
│     └─► 释放字体和图像                                              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 5. 原始帧缓冲渲染后端 (render_engine_raw.h)

### 5.1 Raw 后端特点

原始帧缓冲渲染直接操作 Linux 帧缓冲设备 (`/dev/fb0`)：
- 高效的像素操作
- 低延迟
- 适合嵌入式系统

### 5.2 RenderEngineRaw 类

```cpp
class RenderEngineRaw: public RenderEngine
{
   public:
     RenderEngineRaw();
     virtual ~RenderEngineRaw();

     // 帧控制
     virtual void startFrame();
     virtual void endFrame();
     virtual void rotate180();

     // 绘图操作
     virtual void drawLine(float x1, float y1, float x2, float y2);
     virtual void drawRect(float xPos, float yPos, float fWidth, float fHeight);
     virtual void drawRoundRect(float xPos, float yPos, float fWidth, float fHeight, float fCornerRadius);
     virtual void fillCircle(float x, float y, float r);
     virtual void drawCircle(float x, float y, float r);

   protected:
      struct _fbg* m_pFBG;                   // fbgraphics 上下文

      struct _fbg_img* m_pImages[MAX_RAW_IMAGES];  // 图像对象
      struct _fbg_img* m_pIcons[MAX_RAW_ICONS];    // 图标对象
};
```

### 5.3 fbgraphics 库

fbgraphics 是一个轻量级的帧缓冲图形库：
- 直接操作帧缓冲设备
- 支持基本绘图操作
- 支持图像加载和渲染
- 支持字体渲染

---

## 6. Dispmanx 后端 (fbg_dispmanx.h)

### 6.1 Dispmanx 特点

Dispmanx 是 Raspberry Pi 的显示管理 API：
- 使用 Broadcom GPU
- 支持硬件加速
- 支持多层叠加
- 高效的图像合成

### 6.2 Dispmanx 架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Dispmanx 架构                                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    应用层                                    │   │
│  │  - RenderEngineRaw                                          │   │
│  │  - fbgraphics                                               │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    fbg_dispmanx                              │   │
│  │  - Dispmanx元素创建                                          │   │
│  │  - 资源管理                                                  │   │
│  │  - 层叠加                                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    Broadcom GPU                              │   │
│  │  - VC (VideoCore)                                            │   │
│  │  - 硬件加速                                                  │   │
│  │  - 显示合成                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    HDMI/DSI 输出                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 7. DRM 核心 (drm_core.h)

### 7.1 DRM 特点

DRM (Direct Rendering Manager) 是 Linux 的图形子系统：
- 直接 GPU 访问
- 支持硬件加速
- 支持多显示输出
- 现代图形架构

### 7.2 DRM 架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DRM 架构                                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    应用层                                    │   │
│  │  - RenderEngineCairo                                        │   │
│  │  - Cairo                                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    drm_core                                  │   │
│  │  - DRM设备管理                                               │   │
│  │  - 缓冲区管理                                                │   │
│  │  - 显示输出                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    DRM/KMS                                   │   │
│  │  - Kernel Mode Setting                                       │   │
│  │  - GEM (Graphics Execution Manager)                          │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    GPU驱动                                   │   │
│  │  - Panfrost (开源)                                           │   │
│  │  - Arm Mali (闭源)                                           │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    HDMI/DSI 输出                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 8. 图像解码库

### 8.1 lodepng

lodepng 是一个纯 C/C++ 实现的 PNG 解码库：
- 无外部依赖
- 支持 PNG 所有特性
- 内存效率高

### 8.2 nanojpeg

nanojpeg 是一个极简的 JPEG 解码器：
- 代码量小
- 解码速度快
- 适合嵌入式系统

---

## 9. 字体系统

### 9.1 字体格式

RubyFPV 使用自定义的字体描述文件格式 (`.dsc`)：
- 字体图像包含所有字符
- 描述文件包含字符位置和属性
- 支持字距调整 (kerning)

### 9.2 字体文件结构

```
字体描述文件 (.dsc):
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  字体名称: font_ariobold_24                                        │
│  字体家族ID: 0                                                      │
│  是否粗体: true                                                     │
│  行高: 24                                                           │
│  基线: 20                                                           │
│  首字符ID: 32 (空格)                                                │
│  末字符ID: 255                                                      │
│                                                                     │
│  字符数据:                                                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ 字符 'A' (ID: 65):                                           │   │
│  │   - 图像位置: (x=100, y=0)                                    │   │
│  │   - 字符大小: (width=12, height=24)                           │   │
│  │   - 渲染偏移: (xOffset=0, yOffset=-2)                         │   │
│  │   - X前进: 12                                                 │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  字距调整:                                                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ 'A' + 'V' → 调整 -2                                          │   │
│  │ 'T' + 'o' → 调整 -1                                          │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 9.3 字体资源文件

`res/` 目录包含多种字体：
- `font_ariobold_*.dsc` - Arial Bold 系列 (14-56号)
- `font_bt_bold_*.dsc` - BT Bold 系列
- `font_raw_bold_*.dsc` - Raw Bold 系列

---

## 10. 渲染操作详解

### 10.1 基本绘图操作

| 操作 | 函数 | 说明 |
|------|------|------|
| **线条** | `drawLine(x1, y1, x2, y2)` | 绘制直线 |
| **矩形** | `drawRect(x, y, w, h)` | 绘制矩形边框 |
| **填充矩形** | `fillRect(x, y, w, h)` | 填充矩形 |
| **圆角矩形** | `drawRoundRect(x, y, w, h, r)` | 绘制圆角矩形 |
| **圆形** | `drawCircle(x, y, r)` | 绘制圆形边框 |
| **填充圆形** | `fillCircle(x, y, r)` | 填充圆形 |
| **弧形** | `drawArc(x, y, r, a1, a2)` | 绘制弧形 |
| **三角形** | `drawTriangle(...)` | 绘制三角形 |
| **多边形** | `fillPolygon(x[], y[], n)` | 填充多边形 |

### 10.2 文本渲染操作

| 操作 | 函数 | 说明 |
|------|------|------|
| **文本宽度** | `textWidth(fontId, text)` | 计算文本宽度 |
| **文本高度** | `textHeight(fontId)` | 获取文本高度 |
| **绘制文本** | `drawText(x, y, fontId, text)` | 绘制文本 |
| **左对齐文本** | `drawTextLeft(x, y, fontId, text)` | 左对齐绘制 |
| **缩放文本** | `drawTextScaled(x, y, fontId, scale, text)` | 缩放绘制 |
| **多行文本** | `drawMessageLines(...)` | 绘制多行文本 |

### 10.3 图像渲染操作

| 操作 | 函数 | 说明 |
|------|------|------|
| **绘制图像** | `drawImage(x, y, w, h, imageId)` | 绘制图像 |
| **透明图像** | `drawImageAlpha(x, y, w, h, imageId, alpha)` | 带透明度绘制 |
| **图像块传输** | `bltImage(...)` | 图像块传输 |
| **绘制图标** | `drawIcon(x, y, w, h, iconId)` | 绘制图标 |

---

## 11. 颜色系统

### 11.1 颜色格式

颜色使用 RGBA 格式：
- `double` 数组: `[r, g, b, a]` (范围 0.0-1.0)
- 分量参数: `float r, g, b, a` (范围 0.0-1.0)

### 11.2 颜色设置

```cpp
// 设置填充颜色
void setFill(const double* pColor);
void setFill(float r, float g, float b, float a);

// 设置描边颜色
void setStroke(const double* color);
void setStroke(float r, float g, float b, float a);

// 设置描边宽度
void setStrokeSize(float fStrokeSize);
```

---

## 12. Alpha 混合

### 12.1 Alpha 混合控制

```cpp
// 设置全局Alpha值
float setGlobalAlfa(float alfa);
float getGlobalAlfa();

// Alpha混合开关
bool isAlphaBlendingEnabled();
void setAlphaBlendingEnabled(bool bEnable);
void enableAlphaBlending();
void disableAlphaBlending();
```

### 12.2 Alpha 混合用途

- 半透明菜单叠加
- 渐变效果
- 图像淡入淡出
- 告警闪烁效果

---

## 13. 双缓冲机制

### 13.1 双缓冲原理

双缓冲用于避免画面撕裂：
- 前缓冲: 当前显示的帧
- 后缓冲: 正在渲染的帧
- 帧完成后交换缓冲

### 13.2 双缓冲流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                    双缓冲流程                                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  前缓冲 (显示)          后缓冲 (渲染)                               │
│  ┌─────────────┐       ┌─────────────┐                             │
│  │  Frame N    │       │  Frame N+1  │                             │
│  │  (显示中)   │       │  (渲染中)   │                             │
│  └─────────────┘       └─────────────┘                             │
│                                                                     │
│  渲染流程:                                                          │
│    │                                                                │
│    ├─► startFrame()                                                 │
│    │     └─► 清除后缓冲                                             │
│    │                                                                │
│    ├─► 绘图操作                                                     │
│    │     └─► 在后缓冲上绘制                                         │
│    │                                                                │
│    ├─► endFrame()                                                   │
│    │     ├─► 完成后缓冲渲染                                         │
│    │     ├─► 交换前后缓冲                                           │
│    │     └─► 后缓冲变为前缓冲显示                                   │
│    │                                                                │
│    └─────────────────────────────────────────────────────────────┐ │
│    │  前缓冲 (显示)          后缓冲 (渲染)                         │ │
│    │  ┌─────────────┐       ┌─────────────┐                       │ │
│    │  │  Frame N+1  │       │  Frame N+2  │                       │ │
│    │  │  (显示中)   │       │  (渲染中)   │                       │ │
│    │  └─────────────┘       └─────────────┘                       │ │
│    └─────────────────────────────────────────────────────────────┘ │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 14. 渲染性能优化

### 14.1 优化策略

| 策略 | 说明 |
|------|------|
| **双缓冲** | 避免画面撕裂 |
| **硬件加速** | 使用 GPU 加速渲染 |
| **图像缓存** | 预加载图像和图标 |
| **字体缓存** | 预加载字体 |
| **脏区域渲染** | 只渲染变化区域 |
| **Mipmap** | 缩放图像使用预生成的小图 |

### 14.2 图像 Mipmap

```cpp
// 图像 Mipmap 结构
struct _fbg_img* m_pIcons[MAX_RAW_ICONS];        // 原始图标
struct _fbg_img* m_pIconsMip[MAX_RAW_ICONS][2];  // Mipmap 图标 (缩小版本)
```

Mipmap 用于：
- 快速渲染缩小的图像
- 避免实时缩放的性能开销
- 提高渲染质量

---

## 15. 渲染流程示例

### 15.1 菜单渲染流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                    菜单渲染流程                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 开始帧                                                          │
│     render_engine->startFrame();                                    │
│                                                                     │
│  2. 渲染视频背景                                                    │
│     // 视频解码器直接渲染到帧缓冲                                   │
│                                                                     │
│  3. 渲染OSD叠加                                                     │
│     ├─► 设置颜色                                                    │
│     │     render_engine->setFill(osd_color);                        │
│     │                                                               │
│     ├─► 渲染人工地平仪                                              │
│     │     render_engine->drawLine(...);                             │
│     │     render_engine->fillPolygon(...);                          │
│     │                                                               │
│     ├─► 渲染文本                                                    │
│     │     render_engine->drawText(x, y, font_id, "Alt: 100m");      │
│     │                                                               │
│     ├─► 渲染图标                                                    │
│     │     render_engine->drawIcon(x, y, w, h, icon_id);             │
│     │                                                               │
│     └─► 渲染仪表盘                                                  │
│           render_engine->fillCircle(...);                           │
│           render_engine->drawArc(...);                              │
│                                                                     │
│  4. 渲染菜单                                                        │
│     ├─► 渲染菜单背景                                                │
│     │     render_engine->setFill(menu_bg_color);                    │
│     │     render_engine->setGlobalAlfa(0.8);                        │
│     │     render_engine->drawRoundRect(x, y, w, h, radius);         │
│     │                                                               │
│     ├─► 渲染菜单项                                                  │
│     │     render_engine->drawText(x, y, font_id, "Settings");       │
│     │                                                               │
│     └─► 渲染选中指示                                                │
│           render_engine->setFill(highlight_color);                  │
│           render_engine->drawRect(x, y, w, h);                      │
│                                                                     │
│  5. 结束帧                                                          │
│     render_engine->endFrame();                                      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 16. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                    renderer 模块依赖关系                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    base (基础库)                             │  │
│   │  - base.h (类型定义)                                         │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   图像解码库                                  │  │
│   │  - lodepng.h (PNG解码)                                       │  │
│   │  - nanojpeg.c (JPEG解码)                                     │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   fbgraphics (帧缓冲图形库)                   │  │
│   │  - 基本绘图操作                                              │  │
│   │  - 图像渲染                                                  │  │
│   │  - 字体渲染                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │fbg_dispmanx│    │ drm_core   │    │   Cairo    │              │
│   │ (Pi GPU)   │    │  (DRM)     │    │  (矢量)    │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │RenderEngine│    │RenderEngine│    │RenderEngine│              │
│   │    Raw     │    │   Cairo    │    │    OVG     │              │
│   │   (Pi)     │    │  (Radxa)   │    │   (Pi)     │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   RenderEngine (基类)                        │  │
│   │  - 通用渲染接口                                              │  │
│   │  - 字体管理                                                  │  │
│   │  - 图像管理                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   r_central (使用方)                         │  │
│   │  - menu (菜单渲染)                                           │  │
│   │  - osd (OSD渲染)                                             │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 17. 平台选择逻辑

### 17.1 Makefile 中的渲染选择

```makefile
# Raspberry Pi
ifeq ($(RUBY_BUILD_ENV),)
CENTRAL_RENDER_CODE := \
    $(FOLDER_CENTRAL_RENDERER)/lodepng.o \
    $(FOLDER_CENTRAL_RENDERER)/nanojpeg.o \
    $(FOLDER_CENTRAL_RENDERER)/fbgraphics.o \
    $(FOLDER_CENTRAL_RENDERER)/render_engine.o \
    $(FOLDER_CENTRAL_RENDERER)/render_engine_raw.o \
    $(FOLDER_CENTRAL_RENDERER)/render_engine_ui.o \
    $(FOLDER_CENTRAL_RENDERER)/fbg_dispmanx.o
endif

# Radxa
ifeq ($(RUBY_BUILD_ENV),radxa)
CENTRAL_RENDER_CODE := \
    $(FOLDER_CENTRAL_RENDERER)/render_engine.o \
    $(FOLDER_CENTRAL_RENDERER)/render_engine_cairo.o \
    $(FOLDER_CENTRAL_RENDERER)/render_engine_ui.o \
    $(FOLDER_CENTRAL_RENDERER)/drm_core.o
endif
```

---

## 18. 渲染引擎初始化

### 18.1 初始化流程

```
初始化流程:
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  1. 创建渲染引擎实例                                                │
│     │                                                               │
│     ├─► Raspberry Pi:                                               │
│     │     RenderEngineRaw* engine = new RenderEngineRaw();          │
│     │                                                               │
│     ├─► Radxa:                                                      │
│     │     RenderEngineCairo* engine = new RenderEngineCairo();      │
│     │                                                               │
│  2. 初始化引擎                                                      │
│     │                                                               │
│     ├─► engine->initEngine()                                        │
│     │     ├─► 打开显示设备                                          │
│     │     ├─► 创建渲染表面                                          │
│     │     ├─► 获取屏幕尺寸                                          │
│     │     └─► 初始化绘图上下文                                      │
│     │                                                               │
│  3. 加载资源                                                        │
│     │                                                               │
│     ├─► 加载字体                                                    │
│     │     engine->loadRawFont(0, "res/font_ariobold_24.dsc", 0);    │
│     │                                                               │
│     ├─► 加载图像                                                    │
│     │     engine->loadImage("res/background.png");                  │
│     │                                                               │
│     ├─► 加载图标                                                    │
│     │     engine->loadIcon("res/icon_settings.png");                │
│     │                                                               │
│  4. 开始渲染循环                                                    │
│     │                                                               │
│     └─► 进入主渲染循环                                              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*