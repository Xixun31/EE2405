# EE2405 Final Project: STM32F769I-Discovery UI & Controller (lv_F769)

本專案是國立清華大學 EE2405 嵌入式系統實驗的期末專案子系統。本程式運行於 **STM32F769I-Discovery (F769)** 開發板上，主要功能為透過 **Ethernet** 連接 MQTT Broker，接收 BBCar 傳回的狀態數據並即時繪製軌跡（使用 LVGL 散佈圖），同時提供觸控螢幕 D-Pad 按鈕，以手動控制 BBCar 的移動或切換至自動尋線模式。

---

## 📌 功能特點

1. **LVGL 觸控圖形介面 (GUI)**：
   - **軌跡繪製 (Scatter Chart)**：左側顯示 2D 散佈圖，將 BBCar 左右輪編碼器（里程計 $dl, dr$）的數據，透過差動輪運動學 (Differential Drive Kinematics) 計算，即時繪製小車路徑。
   - **狀態顯示**：右上方即時顯示車速 (`Spd`)、總里程 (`Dist`) 以及小車加速度計數據 (`Acc: X, Y, Z`)。
   - **觸控 D-Pad 按鈕**：右下角提供直行、後退、左轉、右轉、停止及自動模式 (`Auto Mode`) 按鈕，可用於手動操控小車。
2. **MQTT 網路通訊 (透過 Ethernet)**：
   - 透過 STM32F769 內建的有線網路孔連接區域網路。
   - 訂閱 `bbcar/status`：接收來自小車（IoT 板）發送的 JSON 狀態封包。
   - 發布 `bbcar/control`：傳送觸控面板輸入的指令給小車。

---

## 🔌 硬體連接說明

1. **有線網路 (Ethernet)**：
   - 請插入 RJ-45 網路線至 STM32F769 的 Ethernet 接口，確保小車與 Broker 處於同一個區網段中。
2. **電源與偵錯 (ST-LINK USB)**：
   - 連接 `USB ST-LINK` 接口（Micro-USB）至電腦以進行程式燒錄與序列埠偵錯。
3. **LED 狀態指示燈**：
   - 運行中 onboard LED 會定時閃爍，代表主迴圈運作正常。

---

## ⚙️ 系統設定

在 [src/main.cpp](file:///home/xixun/EE2405/finalproject/finalproject_F769/src/main.cpp) 中，您可以自訂以下設定：

- **MQTT Broker IP**: 預設為 `192.168.1.88` (第 112 行)
- **MQTT Broker Port**: 預設為 `1883` (第 113 行)
- **小車輪距 (wheel_base)**：用於計算軌跡，預設為 `11.0f` cm (第 55 行)

---

## 🛠️ 編譯與燒錄步驟 (Target: DISCO_F769NI)

請在終端機中切換至本專案目錄下：

### 1. 初始化專案 (若尚未建立連結)
```bash
cd /home/xixun/EE2405/finalproject/finalproject_F769
ln -s ../mbed-os .
```

### 2. 建立編譯目錄並進行 CMake 設定
```bash
mkdir -p build && cd build
cmake .. -GNinja -DMBED_TARGET=DISCO_F769NI
```

### 3. 編譯與燒錄
* **Ubuntu** (使用 ST-Link 燒錄)：
  ```bash
  sudo ninja flash-lv_F769
  ```
* **macOS** (編譯後手動拷貝至磁碟)：
  ```bash
  ninja lv_F769
  cp lv_F769.bin /Volumes/DIS_F769NI/ && sync
  ```

---

## 💻 序列埠監控 (Serial Monitor)

- **鮑率 (Baud Rate)**：`115200`
- **Ubuntu 監控指令**：
  ```bash
  sudo microcom --port=/dev/ttyACM0 --speed=115200
  # 退出方式：Ctrl-\ 然後按 Ctrl-C
  ```
- **macOS 監控指令**：
  ```bash
  ls /dev/cu.usbmodem*  # 確認硬體名稱
  minicom -D /dev/cu.usbmodemXXXX -b 115200
  ```

---

## 🔄 差動輪軌跡計算原理

接收到 `bbcar/status` 主題的資料：
`{"x": ax, "y": ay, "z": az, "dl": dl, "dr": dr, "s": speed}`
程式會利用下述公式即時計算並累加小車座標 $(x, y)$ 與朝向 $\theta$：

$$d\_left = dl - last\_dl$$
$$d\_right = dr - last\_dr$$
$$d\_center = \frac{d\_left + d\_right}{2}$$
$$d\_\theta = \frac{d\_right - d\_left}{wheel\_base}$$
$$heading = heading + d\_\theta$$
$$x = x + d\_center \times \cos(heading)$$
$$y = y + d\_center \times \sin(heading)$$

計算完成後，程式鎖定 LVGL 互斥鎖 (`lvgl_mutex`) 並調用 `lv_chart_set_next_value2()` 更新螢幕左側軌跡圖。
