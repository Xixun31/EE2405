#include "mbed.h"
#include "bbcar.h"
#include "Pixy2/Pixy2MbedSPI.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "accelerometer.h"

Ticker servo_ticker;
Ticker servo_feedback_ticker;

PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

// MOSI, MISO, SCLK, CS
Pixy2MbedSPI pixy(PD_4, PD_3, PD_1, PD_5);
DigitalInOut pin8(D8);

static const uint32_t kBarcodeDebounceMs = 350;
static const uint32_t kPendingTurnTimeoutMs = 2500;

enum BarcodeCase {
	BARCODE_NONE = -1,
	BARCODE_LEFT = 0,
	BARCODE_STOP = 1,
	BARCODE_RIGHT = 2,
};

enum DriveState {
	FOLLOW_LINE,
	STOPPED,
    REMOTE_CONTROL
};

Timer app_timer;

WiFiInterface *wifi;
Accelerometer* accelerometer_ptr = nullptr;
Thread mqtt_thread(osPriorityNormal);
EventQueue mqtt_queue;
MQTTNetwork* mqtt_network_ptr = nullptr;
MQTT::Client<MQTTNetwork, Countdown>* mqtt_client_ptr = nullptr;

volatile bool is_remote_control = false;
volatile float rc_left = 0.0f;
volatile float rc_right = 0.0f;

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char payload[300];
    sprintf(payload, "%.*s", message.payloadlen, (char*)message.payload);
    printf("Received control: %s\r\n", payload);
    
    if (strncmp(payload, "cmd:forward", 11) == 0) {
        is_remote_control = true;
        rc_left = 45.0f;   // speed=50, trim=-4 => left = -(50 - (-4)) = -54
        rc_right = 56.0f;  // speed=50, trim=-4 => right = -50 - (-4) = -46
    } else if (strncmp(payload, "cmd:backward", 12) == 0) {
        is_remote_control = true;
        rc_left = -54.0f;    // backward left = 50 + trim (-4) = 54
        rc_right = -46.0f;   // backward right = 50 - trim (-4) = 46
    } else if (strncmp(payload, "cmd:left", 8) == 0) {
        is_remote_control = true;
        rc_left = 40.0f;    // 原地左轉：左輪後退，右輪前進
        rc_right = -40.0f;
    } else if (strncmp(payload, "cmd:right", 9) == 0) {
        is_remote_control = true;
        rc_left = -40.0f;   // 原地右轉：左輪前進，右輪後退
        rc_right = 40.0f;
    } else if (strncmp(payload, "cmd:stop", 8) == 0) {
        is_remote_control = true;
        rc_left = 0.0f;
        rc_right = 0.0f;
    } else if (strncmp(payload, "cmd:auto", 8) == 0) {
        is_remote_control = false;
    }
}

void publish_telemetry() {
    if (!mqtt_client_ptr || !accelerometer_ptr) return;
    
    double rawAccelerationData[3] = {0};
    double calibratedAccelerationData[3] = {0};
    accelerometer_ptr->GetAcceleromterSensor(rawAccelerationData);
    accelerometer_ptr->GetAcceleromterCalibratedData(calibratedAccelerationData);
    
    // distance = angle * 0.05672
    float dist_left = car.servo0.angle * 0.05672f;
    float dist_right = -car.servo1.angle * 0.05672f;
    float speed = (abs(car.servo0.current_pwm_value) + abs(car.servo1.current_pwm_value)) / 2.0f;
    
    char buff[150];
    sprintf(buff, "{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"dl\":%.2f,\"dr\":%.2f,\"s\":%.2f}",
            calibratedAccelerationData[0],
            calibratedAccelerationData[1],
            calibratedAccelerationData[2],
            dist_left, dist_right, speed);
            
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    
    int rc = mqtt_client_ptr->publish("bbcar/status", message);
    if (rc != 0) {
        printf("Publish failed: %d\r\n", rc);
    }
}

static uint32_t now_ms()
{
	return chrono::duration_cast<chrono::milliseconds>(app_timer.elapsed_time()).count();
}

static const char *barcode_to_text(int code)
{
	switch (code) {
		case BARCODE_LEFT: return "Barcode 0: Turn Left";
		case BARCODE_STOP: return "Barcode 1: Stop";
		case BARCODE_RIGHT: return "Barcode 2: Turn Right";
		default: return "No Barcode";
	}
}

static int read_barcode_code(int8_t feat_res)
{
	if ((feat_res & LINE_BARCODE) && pixy.line.numBarcodes > 0 && pixy.line.barcodes != nullptr) {
		return static_cast<int>(pixy.line.barcodes[0].m_code);
	}
	return BARCODE_NONE;
}

static void apply_turn_command(int barcode_code)
{
	if (barcode_code == BARCODE_LEFT) {
		pixy.line.setNextTurn(90);
		printf("Queued next turn: LEFT (90 deg)\r\n");
	} else if (barcode_code == BARCODE_RIGHT) {
		pixy.line.setNextTurn(-90);
		printf("Queued next turn: RIGHT (-90 deg)\r\n");
	}
}

int main()
{
	accelerometer_ptr = new Accelerometer();
	parallax_laserping ping1(pin8);
	app_timer.start();

	// 保留 go_v1 尋線參數，並加入 trim=-4.0f 直行修正
	const float speed_up = 80.0f;
	const float slow_down = 50.0f;
	const float trim = -4.0f;
	const float heading_kp = 1.7f;
	const float heading_kd = 0.6f;
	const float heading_deadband = 10.5f;
	const float wheel_speed_limit = 135.0f;
	float prev_error = 0.0f;
	bool has_prev_error = false;

    printf("Connecting to WiFi...\r\n");
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\r\n");
    } else {
        int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
        if (ret != 0) {
            printf("\r\nConnection error: %d\r\n", ret);
        } else {
            NetworkInterface* net = wifi;
            mqtt_network_ptr = new MQTTNetwork(net);
            mqtt_client_ptr = new MQTT::Client<MQTTNetwork, Countdown>(*mqtt_network_ptr);
            
            const char* host = "192.168.1.88";
            const int port = 1883;
            printf("Connecting to TCP network at %s...\r\n", host);
            int rc = mqtt_network_ptr->connect(host, port);
            if (rc != 0) {
                printf("\nConnection error: %d\r\n", rc);
            } else {
                MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                data.MQTTVersion = 3;
                data.clientID.cstring = (char*)"BBCar";
                if ((rc = mqtt_client_ptr->connect(data)) != 0){
                    printf("Fail to connect MQTT\r\n");
                }
                if (mqtt_client_ptr->subscribe("bbcar/control", MQTT::QOS0, messageArrived) != 0){
                    printf("Fail to subscribe\r\n");
                }
                
                mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
                mqtt_queue.call_every(500ms, publish_telemetry);
                
                Thread* yield_thread = new Thread(osPriorityNormal);
                yield_thread->start([&]() {
                    while(true) {
                        if (mqtt_client_ptr) {
                            mqtt_client_ptr->yield(100);
                        }
                        ThisThread::sleep_for(100ms);
                    }
                });
            }
        }
    }

	printf("Pixy2 init...\r\n");
	int8_t init_res = pixy.init();
	if (init_res < 0) {
		printf("Pixy2 init failed: %d\r\n", init_res);
		car.stop();
		while (true) {
			thread_sleep_for(500);
		}
	}
	pixy.setLamp(1, 1);  // upper, lower
	pixy.setServos(500, 1000);

	int8_t prog_res = pixy.changeProg("line");
	if (prog_res < 0) {
		printf("Change program to line failed: %d\r\n", prog_res);
		car.stop();
		while (true) {
			thread_sleep_for(500);
		}
	}

	printf("Pixy2 line tracking started. Frame: %u x %u\r\n",
		   pixy.frameWidth, pixy.frameHeight);
	pixy.line.setDefaultTurn(0); // 預設直行

	DriveState state = FOLLOW_LINE;
	int last_barcode_code = BARCODE_NONE;
	uint32_t last_barcode_seen_ms = 0;
	bool barcode_armed = true;

	int pending_turn_code = BARCODE_NONE;
	uint32_t pending_turn_set_ms = 0;
	bool prev_intersection_seen = false;

	int ping_count = 0;
	float dist = 999.0f;

	while (true) {
        if (is_remote_control) {
            car.driveLR(rc_left, rc_right);
            thread_sleep_for(100);
            continue;
        }

		// 1. LaserPING 避障處理（最高優先）
		ping_count++;
		if (ping_count >= 5) {
			dist = (float)ping1;
			// printf("LaserPING: %.2f cm\r\n", dist);
			ping_count = 0;
		}

		if (dist < 20.0f) {
			car.stop();
			has_prev_error = false;
			thread_sleep_for(10);
			continue;
		}

		// 2. 獲取 Pixy2 特徵（必須是 LINE_ALL_FEATURES 以包含 Barcode 偵測）
		int8_t feat_res = pixy.line.getMainFeatures(LINE_ALL_FEATURES);
		if (feat_res < 0) {
			printf("Pixy2 read error: %d\r\n", feat_res);
			thread_sleep_for(100);
			continue;
		}

		// 3. 處理 Barcode 偵測
		int barcode_code = read_barcode_code(feat_res);
		uint32_t now = now_ms();

		if (barcode_code != BARCODE_NONE) {
			if (barcode_code != last_barcode_code || (now - last_barcode_seen_ms) > kBarcodeDebounceMs) {
				printf("Seen %s\r\n", barcode_to_text(barcode_code));

				if (barcode_code == BARCODE_STOP) {
					state = STOPPED;
					car.stop();
					printf("STOPPED by barcode\r\n");
				} else if (barcode_armed && (barcode_code == BARCODE_LEFT || barcode_code == BARCODE_RIGHT)) {
					apply_turn_command(barcode_code);
					pending_turn_code = barcode_code;
					pending_turn_set_ms = now;
					barcode_armed = false;
				}

				last_barcode_code = barcode_code;
				last_barcode_seen_ms = now;
			}
		} else {
			barcode_armed = true;
		}

		// 4. 若狀態為 STOPPED，維持靜止
		if (state == STOPPED) {
			car.stop();
			thread_sleep_for(50);
			continue;
		}

		// 5. 尋線向量檢測與控制
		if (!(feat_res & LINE_VECTOR) || pixy.line.numVectors == 0) {
			car.stop();
			has_prev_error = false;
			thread_sleep_for(10);
			continue;
		}

		const Vector &v = pixy.line.vectors[0];
		const float center_x = (pixy.frameWidth > 0) ? (pixy.frameWidth * 0.5f) : 39.5f;
		float error = center_x - static_cast<float>(v.m_x1);

		float heading_cmd = 0.0f;
		if (has_prev_error) {
			heading_cmd = (heading_kp * error) + (heading_kd * (error - prev_error));
		} else {
			heading_cmd = heading_kp * error;
		}

		if (heading_cmd > 0.0f) {
			heading_cmd += heading_deadband;
		} else if (heading_cmd < 0.0f) {
			heading_cmd -= heading_deadband;
		}
		prev_error = error;
		has_prev_error = true;

		float base_speed = speed_up;
		if (v.m_flags & LINE_FLAG_INTERSECTION_PRESENT) {
			base_speed = slow_down;
		}

		if (v.m_y0 <= v.m_y1) {
			base_speed = -slow_down;
		}

		float direction = (base_speed >= 0) ? 1.0f : -1.0f;
		float left = base_speed - (trim * direction) + heading_cmd;
		float right = base_speed + (trim * direction) - heading_cmd;
		left = car.clamp(left, wheel_speed_limit, -wheel_speed_limit);
		right = car.clamp(right, wheel_speed_limit, -wheel_speed_limit);
		car.driveLR(left, right);

		// 6. 處理交叉路口的轉向加強
		bool intersection_seen = ((v.m_flags & LINE_FLAG_INTERSECTION_PRESENT) != 0) ||
			((feat_res & LINE_INTERSECTION) && pixy.line.numIntersections > 0);

		if (intersection_seen && !prev_intersection_seen) {
			printf("Intersection detected\r\n");
			if (pending_turn_code == BARCODE_LEFT) {
				pixy.line.setNextTurn(90);
				printf("Reinforce turn at intersection: LEFT\r\n");
			} else if (pending_turn_code == BARCODE_RIGHT) {
				pixy.line.setNextTurn(-90);
				printf("Reinforce turn at intersection: RIGHT\r\n");
			}
			pending_turn_code = BARCODE_NONE;
		}

		// 7. 轉向指令超時清除
		if (pending_turn_code != BARCODE_NONE &&
			(now - pending_turn_set_ms) > kPendingTurnTimeoutMs) {
			pending_turn_code = BARCODE_NONE;
		}

		prev_intersection_seen = intersection_seen;
		thread_sleep_for(10);
	}
}