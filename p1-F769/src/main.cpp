#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "lvgl/lvgl.h"

// --- 加入 eRPC 相關標頭檔 ---
#include "erpc_client_manager.h"
#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "acc_gyro.h" // 這是 erpcgen 幫你產生的 Client 標頭檔

void build_mems_gui(void);

DigitalOut led(LED1);

// --- eRPC Client 基礎設施配置 ---
// 注意：Baud rate (9600) 和腳位 (D1, D0) 必須與 Server 端完全對應
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::ClientManager client_manager;
erpc::ClientManager *g_client;

// --- 建立 EventQueue 與 Thread 處理背景任務 ---
Thread event_thread;
EventQueue queue(32 * EVENTS_EVENT_SIZE);
int timer_id = 0; // 用來記錄定時任務的 ID，以便 Stop 時取消
Mutex lvgl_mutex;

// --- GUI 全域物件 ---
lv_obj_t * label_status;
lv_obj_t * btn_start;
lv_obj_t * btn_stop;
lv_obj_t * chart;
lv_chart_series_t * ser_x;
lv_chart_series_t * ser_y;
lv_chart_series_t * ser_z;

// =================================================================
// 背景任務：呼叫 eRPC 並更新 GUI
// =================================================================
void fetch_and_update_data() {
    double x = 0.0, y = 0.0, z = 0.0;
    
    // 1. 透過 eRPC 呼叫 IoT 開發板 (由於是 Blocking，會等待 Server 回傳)
    GetAccelerometer(&x, &y, &z);

    lvgl_mutex.lock();  // <--- 【上鎖】
    
    // 2. 更新狀態訊息標籤 (Label)
    char buf[64];
    sprintf(buf, "X: %.1f  Y: %.1f  Z: %.1f", x, y, z);
    lv_label_set_text(label_status, buf);
    
    // 3. 更新圖表 (Chart)
    // 註：lv_chart_set_next_value 吃的是整數，若 x,y,z 是小數且數值很小(例如 +/- 1.0g)，
    // 你可以視情況乘上 1000 變成 mg，例如 (lv_coord_t)(x * 1000.0)
    lv_chart_set_next_value(chart, ser_x, (lv_coord_t)(x * 1000.0));
    lv_chart_set_next_value(chart, ser_y, (lv_coord_t)(y * 1000.0));
    lv_chart_set_next_value(chart, ser_z, (lv_coord_t)(z * 1000.0));

    lvgl_mutex.unlock(); // <--- 【解鎖】
}

// =================================================================
// 按鈕事件處理
// =================================================================
static void event_handler_start(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Start Clicked");
        printf("Start button clicked!\n"); // <--- 加這行
        // 確保不會重複啟動
        if(timer_id == 0) {
            // 設定每 100 毫秒 (0.1 秒) 將任務推入 Queue
            timer_id = queue.call_every(100ms, fetch_and_update_data);
            lv_label_set_text(label_status, "Status: Running...");
        }
    }
}

static void event_handler_stop(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Stop Clicked");
        if(timer_id != 0) {
            // 取消 Queue 中的定時任務
            queue.cancel(timer_id);
            timer_id = 0;
            lv_label_set_text(label_status, "Status: Stopped");
        }
    }
}

// =================================================================
// 主程式
// =================================================================
int main(void) {
    // 1. 初始化 LVGL 與螢幕
    lv_init();
    tft_init();
    touchpad_init();
    
    // 依據 erpcgen 產生的名稱，通常會有一個 global 初始化函數
    // 2. 初始化 eRPC Client
    uart_transport.setCrc16(&crc16);
    
    // 設定 Client Manager
    client_manager.setMessageBufferFactory(&dynamic_mbf);
    client_manager.setTransport(&uart_transport);
    client_manager.setCodecFactory(&basic_cf);
    
    // 【重要】將設定好的 client_manager 綁定給 eRPC 底層的全域指標
    g_client = &client_manager;

    // 3. 繪製 GUI
    build_mems_gui();

    // 4. 啟動背景執行緒 (專門處理 EventQueue)
    event_thread.start(callback(&queue, &EventQueue::dispatch_forever));

    // 5. LVGL 主迴圈
    while(1) {
        lv_tick_inc(5);
        lvgl_mutex.lock();    // <--- 【上鎖】
        lv_task_handler();
        lvgl_mutex.unlock();  // <--- 【解鎖】
        ThisThread::sleep_for(5ms);
        led = !led;
    }
}

// =================================================================
// GUI 介面配置 (沿用並微調)
// =================================================================
void build_mems_gui(void) {
    // 建立字體樣式
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_24);

    // 1. 建立圖表物件
    chart = lv_chart_create(lv_screen_active());
    lv_obj_set_size(chart, 400, 240);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, -40);

    // 2. 【核心設定】設定為折線圖並開啟循環模式
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR); // 開啟帶缺口的循環功能

    // 3. 設定數值範圍與點數
    // 加速度計 1.0g 乘上 1000 後為 1000mg，範圍設為 -2000 ~ 2000 足夠觀察傾斜
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -2000, 2000); 
    lv_chart_set_point_count(chart, 50); // 設定 X 軸總共顯示 50 個點

    // 4. 設定外觀樣式 (選配)
    // 隱藏點，只顯示線條，讓循環效果更平滑
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR); 

    // 5. 加入 X, Y, Z 三條資料系列
    ser_x = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    ser_y = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    ser_z = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    btn_start = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn_start, event_handler_start, LV_EVENT_ALL, NULL);
    lv_obj_set_size(btn_start, 140, 60); 
    lv_obj_align(btn_start, LV_ALIGN_BOTTOM_LEFT, 40, -40); 
    
    lv_obj_t * label_start = lv_label_create(btn_start);
    lv_label_set_text(label_start, "Start");
    lv_obj_center(label_start);
    lv_obj_add_style(label_start, &font_style, 0); 

    btn_stop = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn_stop, event_handler_stop, LV_EVENT_ALL, NULL);
    lv_obj_set_size(btn_stop, 140, 60); 
    lv_obj_align(btn_stop, LV_ALIGN_BOTTOM_RIGHT, -40, -40); 
    
    lv_obj_t * label_stop = lv_label_create(btn_stop);
    lv_label_set_text(label_stop, "Stop");
    lv_obj_center(label_stop);
    lv_obj_add_style(label_stop, &font_style, 0); 

    label_status = lv_label_create(lv_screen_active());
    lv_label_set_text(label_status, "Status: Idle");
    lv_obj_add_style(label_status, &font_style, 0); 
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -55);
}