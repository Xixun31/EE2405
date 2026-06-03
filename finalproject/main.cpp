#include "mbed.h"
#include "bbcar.h"
#include "Pixy2/Pixy2MbedSPI.h"

Ticker servo_ticker;
Ticker servo_feedback_ticker;

PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);
// MOSI, MISO, SCLK, CS
Pixy2MbedSPI pixy(PD_4, PD_3, PD_1, PD_5);
DigitalInOut pin8(D8);

int main()
{
	parallax_laserping ping1(pin8);
	const float speed_up = 80.0f;
	const float slow_down = 50.0f;
	const float heading_kp = 1.7f;
	const float heading_kd = 0.6f;
	const float heading_deadband = 10.5f;
	const float wheel_speed_limit = 135.0f;
	float prev_error = 0.0f;
	bool has_prev_error = false;

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

	int ping_count = 0;
	float dist = 999.0f;

	while (true) {
		ping_count++;
		if (ping_count >= 5) {
			dist = (float)ping1;
			printf("LaserPING: %.2f cm\r\n", dist);
			ping_count = 0;
		}

		if (dist < 20.0f) {
			car.stop();
			has_prev_error = false;
			thread_sleep_for(10);
			continue;
		}

		int8_t feat_res = pixy.line.getMainFeatures();

		if (feat_res <= 0 || !(feat_res & LINE_VECTOR) || pixy.line.numVectors == 0) {
			car.stop();
			has_prev_error = false;
			thread_sleep_for(10);
			continue;
		}

		if (feat_res & LINE_VECTOR) {
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

			float left = base_speed + heading_cmd;
			float right = base_speed - heading_cmd;
			left = car.clamp(left, wheel_speed_limit, -wheel_speed_limit);
			right = car.clamp(right, wheel_speed_limit, -wheel_speed_limit);
			car.driveLR(left, right);
		}
		thread_sleep_for(10);
	}
}