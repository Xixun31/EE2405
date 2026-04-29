#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "lvgl/lvgl.h"

using namespace std::chrono;

DigitalOut led(LED1);
//---------------------------------------------------------------------------------------------
// lvgl Setup
EventQueue queue_lvgl(32 * EVENTS_EVENT_SIZE);
Thread t_lvgl;
void lv_ticker_func();
void lv_example_btnmatrix_1();
//---------------------------------------------------------------------------------------------

#define waveformLength (128)
#define lookUpTableDelay (10)
#define bufferLength (32)

AnalogOut Aout(PA_4);

float waveform[waveformLength] = { // 128 samples of a sine waveform
    0.500, 0.525, 0.549, 0.574, 0.598, 0.622, 0.646, 0.670, 0.693, 0.715, 0.737,
    0.759, 0.780, 0.800, 0.819, 0.838, 0.856, 0.873, 0.889, 0.904, 0.918, 0.931,
    0.943, 0.954, 0.964, 0.972, 0.980, 0.986, 0.991, 0.995, 0.998, 1.000, 1.000,
    0.999, 0.997, 0.994, 0.989, 0.983, 0.976, 0.968, 0.959, 0.949, 0.937, 0.925,
    0.911, 0.896, 0.881, 0.864, 0.847, 0.829, 0.810, 0.790, 0.769, 0.748, 0.726,
    0.704, 0.681, 0.658, 0.634, 0.610, 0.586, 0.562, 0.537, 0.512, 0.488, 0.463,
    0.438, 0.414, 0.390, 0.366, 0.342, 0.319, 0.296, 0.274, 0.252, 0.231, 0.210,
    0.190, 0.171, 0.153, 0.136, 0.119, 0.104, 0.089, 0.075, 0.063, 0.051, 0.041,
    0.032, 0.024, 0.017, 0.011, 0.006, 0.003, 0.001, 0.000, 0.000, 0.002, 0.005,
    0.009, 0.014, 0.020, 0.028, 0.036, 0.046, 0.057, 0.069, 0.082, 0.096, 0.111,
    0.127, 0.144, 0.162, 0.181, 0.200, 0.220, 0.241, 0.263, 0.285, 0.307, 0.330,
    0.354, 0.378, 0.402, 0.426, 0.451, 0.475, 0.500};

Mutex dac_mutex;


void playNote(int freq, float duration, float vol) {
  int j = waveformLength;
  int waitTime = 1000000 / waveformLength / freq;
  int i = duration*freq; //play i iterations of waveform samples
  //printf("Sample iteration=%d\n", i);
  printf("Play notes %d\n", freq);
  dac_mutex.lock(); //To prevent other calls to playNote to use DAC
  while (i--) {
    j = waveformLength;
    while (j--) {
      Aout = waveform[j]*vol; //scale with volume
      wait_us(waitTime);
    }
  }
  dac_mutex.unlock();
}

//Dummy version for test without DAC
// void playNote(int freq, float duration, float vol) {

//   printf("Play notes %d\n", freq);

// }

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
#define notes 8 //support 8 notes
int id[notes]; //create an thread ID for each note

//vector of string of note names
std::vector<string> NoteName={"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
//frequency table for C_4, D_4, E_4, F_4, G_4, A_4, B_4, C_5
const float frequency[]={261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

int main(void) {

	lv_init();
	tft_init();
	touchpad_init();
  t_lvgl.start(callback(&queue_lvgl, &EventQueue::dispatch_forever));
  queue_lvgl.call_every(5ms, lv_ticker_func); // 5ms should match lv_ticker_func(5)
  lv_example_btnmatrix_1();
  t.start(callback(&queue, &EventQueue::dispatch_forever));


	while(1) {
    lv_tick_inc(5);
    lv_task_handler();
    ThisThread::sleep_for(5ms);
    led=!led;
	}
}

void lv_ticker_func() {
  //printf("lv_ticker_func()\n");
  lv_tick_inc(5);
  // Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be
  // between 1 and 10). It is required for the internal timing of LittlevGL.
  lv_task_handler();
  // Call lv_task_handler() periodically every few milliseconds.
  // It will redraw the screen if required, handle input devices etc.
}

lv_obj_t *label1;
lv_obj_t *label2;
lv_obj_t *msg_label;

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
    uint32_t idx = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, idx);

    if(idx<notes){ //Ignore other non-note buttons
      if (code != LV_EVENT_PRESSING && code == LV_EVENT_PRESSED) { //Trigger call_every only at the first press
        string note="Playing "+string(txt);
        lv_label_set_text(msg_label, note.c_str());

        id[idx]=queue.call_every(10ms, playNote, frequency[idx], 0.01, 1); //Repeatively play a note with a duration of 0.01s or 10ms
        //id[idx]=queue.call(playNote, frequency[idx], 1, 1); //call playNote for 1s once
        //printf("Call playNote index=%d\n", idx);

      } else if (code == LV_EVENT_RELEASED) {
        string note="Stop playing "+string(txt);
        lv_label_set_text(msg_label, note.c_str());
        queue.cancel(id[idx]); //stop playing a note
        //printf("Cancel playNote index=%d\n", idx);
      }
    }
}

static void event_handler_btn1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED){ //Just print information of control buttons
        lv_label_set_text(msg_label, "Volume Up");
        printf("Volume Up.\n");
    }

}

static void event_handler_btn2(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED){ //Just print information of control buttons
        lv_label_set_text(msg_label, "Volume Down");
        printf("Volume Down.\n");
    }

}

static const char * btnm_map[] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5", ""};

void lv_example_btnmatrix_1(void)
{
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_24);
    lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(btnm1, btnm_map);
    lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(btnm1, &font_style, 0); 
    lv_obj_set_size(btnm1, 640, 360);
    lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t * btn1 = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, -160, 0);
    lv_obj_set_size(btn1, 160, 40); 
    label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Vol Up");
    lv_obj_center(label1);
    lv_obj_add_style(label1, &font_style, 0); 

    lv_obj_t * btn2 = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 160, 0);
    lv_obj_set_size(btn2, 160, 40); 
    label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Vol Down");
    lv_obj_center(label2);
    lv_obj_add_style(label2, &font_style, 0); 

    msg_label = lv_label_create(lv_screen_active());
    lv_label_set_text(msg_label, "No message.");
    lv_obj_add_style(msg_label, &font_style, 0); 
    lv_obj_set_width(msg_label, 200);
    lv_obj_align(msg_label, LV_ALIGN_TOP_MID, 0, 20);
}