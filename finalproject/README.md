# EE2405 Final Project: BBCar Firmware (MbedCEHelloWorld)

本專案是國立清華大學 EE2405 嵌入式系統實驗的期末專案子系統。此韌體程式運行於 **B_L4S5I_IOT01A (IoT 開發板)** 上，安裝於 BBCar 車體，負責小車的自動尋線、條碼偵測、紅外線/雷射避障，以及透過 **WiFi** 聯網與 MQTT 伺服器進行雙向通訊。

---

## 📌 功能特點

1. **自動尋線模式 (Auto Line-Following Mode)**：
   - 藉由 **Pixy2 影像感測器** 偵測地面軌跡，利用 PD 演算法控制雙輪伺服馬達進行尋線，並加入了直行偏向修正（`trim = -4.0f`）。
   - 在十字路口或轉彎處時小車會自動減速 (`slow_down = 30.0f`) 以利穩定過彎。
2. **條碼特徵識別 (Barcode Recognition)**：
   - 使用 Pixy2 讀取地面條碼：
     - **左轉條碼 (`BARCODE_LEFT` - 2)**：執行原地旋轉左轉，並在完成旋轉後自動尋回軌跡線。
     - **右轉條碼 (`BARCODE_RIGHT` - 1)**：執行原地旋轉右轉，並在完成旋轉後自動尋回軌跡線。
     - **停止條碼 (`BARCODE_STOP` - 3)**：直接煞車停下小車。
3. **雷射避障保護 (LaserPING Obstacle Avoidance)**：
   - 使用 **LaserPING 雷射感測器** 量測前方障礙物距離，當距離小於 `20.0` cm 時小車會強制停下，直到前方障礙物移除才繼續尋線。
4. **MQTT 遙控與狀態回報**：
   - **狀態回報**：每 500ms 將左右輪里程計數據 ($dl, dr$)、車速 (`s`) 與內建加速度計 LSM6DSL 測量到的三軸加速度數據 ($x, y, z$) 包裝成 JSON 格式，發送至 `bbcar/status` 主題。
   - **手動遙控**：訂閱 `bbcar/control` 主題。接收到遙控指令時小車會切換至遙控模式，由 F769 觸控螢幕發送的手手動 D-Pad 控制小車移動 (`forward`, `backward`, `left`, `right`, `stop`)；當收到 `auto` 指令時，會自動切換回自動尋線模式。

---

## 🔌 硬體接線與腳位定義

| 感測器/元件 | 腳位名稱 (Mbed Pin) | 說明 |
| :--- | :--- | :--- |
| **左輪伺服馬達 (Left Servo)** | **D9** (PwmIn)<br>**D11** (PwmOut) | 雙輪馬達回授線與控制線 |
| **右輪伺服馬達 (Right Servo)** | **D10** (PwmIn)<br>**D12** (PwmOut) | 雙輪馬達回授線與控制線 |
| **LaserPING 感測器** | **D8** (DigitalInOut) | 雷射測距避障 |
| **Pixy2 鏡頭 (SPI)** | **PD_4** (MOSI)<br>**PD_3** (MISO)<br>**PD_1** (SCLK)<br>**PD_5** (CS) | SPI 影像傳輸線 |
| **LSM6DSL 加速度計** | **板載 I2C** | 晶片內建三軸加速度量測 |

---

## ⚙️ 網路與 MQTT 設定

請在 [mbed_app.json](file:///home/xixun/EE2405/finalproject/mbed_app.json) 中修改您的區域網路 AP 資訊：
- **SSID**: `"ASUS56"` (請根據您的網路環境修改)
- **Password**: `"123456789"`

在 [main.cpp](file:///home/xixun/EE2405/finalproject/main.cpp) 中修改：
- **MQTT Broker IP**: 預設為 `192.168.1.88` (第 177 行)
- **MQTT Broker Port**: 預設為 `1883` (第 178 行)

---

## 🛠️ 編譯與燒錄步驟 (Target: B_L4S5I_IOT01A)

請在終端機中切換至本專案目錄下：

### 1. 初始化專案 (若尚未建立連結)
```bash
cd /home/xixun/EE2405/finalproject
ln -s ../mbed-os .
```

### 2. 建立編譯目錄與 CMake 設定
```bash
mkdir -p build && cd build
cmake .. -GNinja -DMBED_TARGET=B_L4S5I_IOT01A
```

### 3. 編譯並燒錄至小車
* **Ubuntu** (使用 ST-Link 燒錄)：
  ```bash
  sudo ninja flash-MbedCEHelloWorld
  ```
* **macOS** (編譯後手動拷貝至磁碟)：
  ```bash
  ninja MbedCEHelloWorld
  cp MbedCEHelloWorld.bin /Volumes/DIS_L4S5VI/ && sync
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
