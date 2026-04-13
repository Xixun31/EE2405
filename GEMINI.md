# EE2405 嵌入式系統實驗室規範 (GEMINI.md)

## 核心準則
- **語言偏好：** 始終使用 **繁體中文** 回覆。
- **溝通風格：** 簡潔、隨性、專業。
- **技術決策：** 優先考慮性能與資源佔用，適時提供預測性或非常規的優化方案。

## 技術棧與標準
- **Mbed OS:** 
  - 目標開發板：`B_L4S5I_IOT01A`。
  - 編譯工具：`CMake` + `Ninja`。
- **程式碼風格：** 
  - 遵循 Mbed OS 的縮排與命名規範。
  - 註釋需清晰，特別是 Pin 腳位定義。
- **Python:** 用於訊號分析 (FFT, Plotting)，優先使用 `matplotlib` 和 `numpy`。

## 工作流程
### 1. 專案初始化
建立新實驗目錄時，必須包含以下步驟：
- 軟連結 mbed-os：`ln -s ../mbed-os .`
- 複製配置：從 `../Document/` 複製 `CMakeLists.txt` 與 `mbed_app.json`。

### 2. 編譯與燒錄 (Target: B_L4S5I_IOT01A)
- **Ubuntu:** `sudo ninja flash-MbedCEHelloWorld`
- **macOS:** `ninja MbedCEHelloWorld` 後手動執行 `cp *.bin /Volumes/DIS_L4S5VI/ && sync`。

### 3. 序列埠監控
- **速率：** `115200`。
- **工具：** Ubuntu 使用 `microcom`，macOS 使用 `minicom`。

### 4. 文件與測試
- 截圖建議：Ubuntu 使用 `gnome-screenshot -d 5`。
- 實驗截圖應存放在 `./photo/` 目錄。

## 編輯偏好 (Prettier)
- 尊重現有的專案格式，不隨意更改縮排風格。
