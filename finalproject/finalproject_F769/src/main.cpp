#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "lvgl/lvgl.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

void build_mems_gui(void);

DigitalOut led(LED1);

EthInterface *ethernet;
Thread mqtt_thread(osPriorityNormal);
EventQueue mqtt_queue;
MQTT::Client<MQTTNetwork, Countdown>* mqtt_client_ptr = nullptr;
Mutex lvgl_mutex;

// GUI objects
lv_obj_t * label_status;
lv_obj_t * chart;
lv_chart_series_t * ser_route;

// Route calculation
float current_x = 0.0f;
float current_y = 0.0f;
float heading = 0.0f;
float last_dl = 0.0f;
float last_dr = 0.0f;
bool is_first_msg = true;

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char payload[300];
    sprintf(payload, "%.*s", message.payloadlen, (char*)message.payload);
    
    // Parse JSON: {"x":0.00,"y":0.00,"z":0.00,"dl":0.00,"dr":0.00,"s":0.00}
    float ax, ay, az, dl, dr, speed;
    if (sscanf(payload, "{\"x\":%f,\"y\":%f,\"z\":%f,\"dl\":%f,\"dr\":%f,\"s\":%f}", &ax, &ay, &az, &dl, &dr, &speed) == 6) {
        
        // Differential drive kinematics
        if (is_first_msg) {
            last_dl = dl;
            last_dr = dr;
            is_first_msg = false;
        }
        float d_left = dl - last_dl;
        float d_right = dr - last_dr;
        last_dl = dl;
        last_dr = dr;
        
        float d_center = (d_left + d_right) / 2.0f;
        float wheel_base = 11.0f; // estimated cm
        float d_theta = (d_right - d_left) / wheel_base;
        
        heading += d_theta;
        current_x += d_center * cos(heading);
        current_y += d_center * sin(heading);

        lvgl_mutex.lock();
        char buf[128];
        sprintf(buf, "Spd: %.1f cm/s  Dist: %.1f cm\nAcc: %.2f, %.2f, %.2f", speed, (dl+dr)/2.0f, ax, ay, az);
        lv_label_set_text(label_status, buf);
        
        lv_chart_set_next_value2(chart, ser_route, (lv_coord_t)(current_x * 10), (lv_coord_t)(current_y * 10));
        lvgl_mutex.unlock();
    }
}

void publish_cmd(const char* cmd) {
    if (!mqtt_client_ptr) return;
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) cmd;
    message.payloadlen = strlen(cmd) + 1;
    mqtt_client_ptr->publish("bbcar/control", message);
}

static void btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        const char * cmd = (const char *)lv_event_get_user_data(e);
        printf("Sending cmd: %s\r\n", cmd);
        mqtt_queue.call(publish_cmd, cmd);
    }
}

int main(void) {
    lv_init();
    tft_init();
    touchpad_init();
    
    build_mems_gui();

    ethernet = EthInterface::get_default_instance();
    if (!ethernet) {
        printf("ERROR: No EthInterface found.\r\n");
    } else {
        printf("\r\nConnecting to Ethernet...\r\n");
        int ret = ethernet->connect();
        if (ret != 0) {
            printf("\r\nConnection error: %d\r\n", ret);
        } else {
            NetworkInterface* net = ethernet;
            MQTTNetwork* mqttNetwork = new MQTTNetwork(net);
            mqtt_client_ptr = new MQTT::Client<MQTTNetwork, Countdown>(*mqttNetwork);
            
            const char* host = "192.168.1.88";
            const int port = 1883;
            printf("Connecting to TCP network at %s...\r\n", host);
            int rc = mqttNetwork->connect(host, port);
            if (rc != 0) {
                printf("\nConnection error: %d\r\n", rc);
            } else {
                MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                data.MQTTVersion = 3;
                data.clientID.cstring = (char*)"F769_UI";
                if ((rc = mqtt_client_ptr->connect(data)) != 0){
                    printf("Fail to connect MQTT\r\n");
                }
                if (mqtt_client_ptr->subscribe("bbcar/status", MQTT::QOS0, messageArrived) != 0){
                    printf("Fail to subscribe\r\n");
                }
                
                mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
                
                Thread* yield_thread = new Thread(osPriorityNormal);
                yield_thread->start([&]() {
                    while(true) {
                        mqtt_client_ptr->yield(100);
                        ThisThread::sleep_for(100ms);
                    }
                });
            }
        }
    }

    while(1) {
        lv_tick_inc(5);
        lvgl_mutex.lock();
        lv_task_handler();
        lvgl_mutex.unlock();
        ThisThread::sleep_for(5ms);
        led = !led;
    }
}

void create_btn(lv_obj_t* parent, const char* text, const char* cmd, lv_coord_t x, lv_coord_t y) {
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, (void*)cmd);
    lv_obj_set_size(btn, 100, 50); 
    lv_obj_align(btn, LV_ALIGN_CENTER, x, y); 
    
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}

void build_mems_gui(void) {
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_20);

    // Chart on the left
    chart = lv_chart_create(lv_screen_active());
    lv_obj_set_size(chart, 400, 400);
    lv_obj_align(chart, LV_ALIGN_LEFT_MID, 20, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, -2000, 2000);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -2000, 2000);
    lv_chart_set_point_count(chart, 5000); // 增加點數上限，避免舊的軌跡消失
    ser_route = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    // Status label on the top right
    label_status = lv_label_create(lv_screen_active());
    lv_label_set_text(label_status, "Waiting for data...");
    lv_obj_add_style(label_status, &font_style, 0); 
    lv_obj_align(label_status, LV_ALIGN_TOP_RIGHT, -20, 20);

    // D-Pad and Auto buttons on the bottom right
    create_btn(lv_screen_active(), "Forward", "cmd:forward", 200, 50);
    create_btn(lv_screen_active(), "Left", "cmd:left", 100, 110);
    create_btn(lv_screen_active(), "Right", "cmd:right", 300, 110);
    create_btn(lv_screen_active(), "Backward", "cmd:backward", 200, 170);
    create_btn(lv_screen_active(), "Stop", "cmd:stop", 200, 110);
    
    create_btn(lv_screen_active(), "Auto Mode", "cmd:auto", 200, -20);
}