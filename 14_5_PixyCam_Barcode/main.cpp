#include "mbed.h"
#include "Pixy2/Pixy2MbedSPI.h"
#include "bbcar.h"

Ticker servo_ticker;
Ticker servo_feedback_ticker;
PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

Pixy2MbedSPI pixy(PD_4, PD_3, PD_1, PD_5);

static const float kCruiseSpeed = 120.0f;
static const float kIntersectionSpeed = 95.0f;
static const float kHeadingKp = 3.0f;
static const float kHeadingKd = 1.1f;
static const float kHeadingDeadband = 3.0f;
static const float kHeadingCmdLimit = 72.0f;
static const float kHeadingFilterAlpha = 0.65f;
static const float kWheelSpeedLimit = 190.0f;
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
	STOPPED
};

Timer app_timer;

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
		printf("Queued next turn: LEFT\r\n");
	} else if (barcode_code == BARCODE_RIGHT) {
		pixy.line.setNextTurn(-90);
		printf("Queued next turn: RIGHT\r\n");
	}
}

int main()
{
	printf("\r\n=== Pixy2 Barcode Navigation ===\r\n");
	car.stop();
	app_timer.start();

	int8_t init_res = pixy.init();
	if (init_res < 0) {
		printf("Pixy2 init failed: %d\r\n", init_res);
		while (true) {
			thread_sleep_for(500);
		}
	}

	pixy.setLamp(1, 1);
	pixy.setServos(500, 1000);

	int8_t prog_res = pixy.changeProg("line");
	if (prog_res < 0) {
		printf("Failed to switch Pixy2 program to line: %d\r\n", prog_res);
		while (true) {
			thread_sleep_for(500);
		}
	}

	printf("Pixy2 ready. Frame: %u x %u\r\n", pixy.frameWidth, pixy.frameHeight);
	printf("Barcode 0=Left, 1=Stop, 2=Right\r\n");
	pixy.line.setDefaultTurn(0);
	printf("Default turn set to straight\r\n");

	DriveState state = FOLLOW_LINE;
	float prev_error = 0.0f;
	float prev_heading_cmd = 0.0f;
	bool has_prev_error = false;

	int last_barcode_code = BARCODE_NONE;
	uint32_t last_barcode_seen_ms = 0;
	bool barcode_armed = true;

	int pending_turn_code = BARCODE_NONE;
	uint32_t pending_turn_set_ms = 0;
	bool prev_intersection_seen = false;

	while (true) {
		int8_t feat_res = pixy.line.getMainFeatures(LINE_ALL_FEATURES);

		if (feat_res < 0) {
			printf("Pixy2 read error: %d\r\n", feat_res);
			thread_sleep_for(100);
			continue;
		}

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

		if (state == STOPPED) {
			car.stop();
			thread_sleep_for(50);
			continue;
		}

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
			float raw_heading_cmd = (kHeadingKp * error) + (kHeadingKd * (error - prev_error));
			heading_cmd = (kHeadingFilterAlpha * prev_heading_cmd) + ((1.0f - kHeadingFilterAlpha) * raw_heading_cmd);
			if (heading_cmd > 0.0f) heading_cmd += kHeadingDeadband;
			else if (heading_cmd < 0.0f) heading_cmd -= kHeadingDeadband;
			heading_cmd = car.clamp(heading_cmd, kHeadingCmdLimit, -kHeadingCmdLimit);
		}

		prev_error = error;
		prev_heading_cmd = heading_cmd;
		has_prev_error = true;

		float base_speed = kCruiseSpeed;
		if (v.m_flags & LINE_FLAG_INTERSECTION_PRESENT) {
			base_speed = kIntersectionSpeed;
		}

		float left = base_speed + heading_cmd;
		float right = base_speed - heading_cmd;
		left = car.clamp(left, kWheelSpeedLimit, -kWheelSpeedLimit);
		right = car.clamp(right, kWheelSpeedLimit, -kWheelSpeedLimit);
		car.driveLR(left, right);

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

		if (pending_turn_code != BARCODE_NONE &&
			(now - pending_turn_set_ms) > kPendingTurnTimeoutMs) {
			pending_turn_code = BARCODE_NONE;
		}

		prev_intersection_seen = intersection_seen;
		thread_sleep_for(10);
	}
}
