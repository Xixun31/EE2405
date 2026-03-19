# 🚀 EE2405 Embedded System Laboratory

## 🐧 Ubuntu 開發流程 (實驗室機台)
### 更新與初始化專案
```cd ~/EE2405
git pull
```

### 建立新實驗資料夾 (以 labx 為例)
```mkdir labx
cd labx
```

### 建立 Mbed OS 連結與複製必要設定
```ln -s ../mbed-os .
cp ../CMakeLists.txt .
cp ../mbed_app.json .
code main.cpp
```

### 編譯與燒錄 (Compile & Flash)
```mkdir build && cd build
cmake .. -GNinja -DMBED_TARGET=B_L4S5I_IOT01A
```

### 使用 Ninja 進行燒錄
`sudo ninja flash-MbedCEHelloWorld`

### 序列埠監控與截圖
`Monitor: sudo microcom --port=/dev/ttyACM0 --speed=115200Exit: Ctrl-\ 然後按 Ctrl-CScreenshot: gnome-screenshot -d 5` (延遲 5 秒後截圖)

## 🍎 macOS 開發流程 (個人電腦)

### 更新與初始化專案
```cd ~/EE2405
git pull
```


### 建立新實驗資料夾 (以 labx 為例)
```mkdir labx
cd labx
```

### 建立 Mbed OS 連結與複製必要設定
```ln -s ../mbed-os .
cp ../CMakeLists.txt .
cp ../mbed_app.json .
code main.cpp
```

### 編譯與燒錄 (Compile & Flash)
```mkdir build && cd build
cmake .. -GNinja -DMBED_TARGET=B_L4S5I_IOT01A
```

### 在 build 資料夾內執行
`ninja MbedCEHelloWorld`

### 手動燒錄 (Flash)
`cp MbedCEHelloWorld.bin /Volumes/DIS_L4S5VI/
sync`

### 序列埠監控 (Serial Port) 確認裝置名稱
`ls /dev/cu.usbmodem*`

### 使用 minicom 連線 (請根據 ls 結果修改 modem 編號)
`minicom -D /dev/cu.usbmodem1103 -b 115200`
