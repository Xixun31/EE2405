#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "lvgl/lvgl.h"
//#include "lvgl/examples/lv_examples.h"
//#include "lvgl/demos/lv_demos.h"
void lv_example_buttonmatrix_1();
void lv_example_button_1();

DigitalOut led(LED1);

int main(void) {
	lv_init();
	tft_init();
	touchpad_init();

	lv_example_button_1();


	while(1) {
    lv_tick_inc(5);
    uint32_t time_till_next =5;
    lv_task_handler();
    ThisThread::sleep_for(5ms);
    led=!led;
	}
}

lv_obj_t * label1;
lv_obj_t * label2;
lv_obj_t * label3;
lv_obj_t * btn1;
lv_obj_t * btn2;

static void event_handler_btn1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        lv_label_set_text(label3, "Button 1 clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static void event_handler_btn2(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        lv_label_set_text(label3, "Button 2 clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void lv_example_button_1(void)
{
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_24);
    btn1 = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_size(btn1, 160, 80); 
    
    label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Button");
    lv_obj_center(label1);
    lv_obj_add_style(label1, &font_style, 0); 

    btn2 = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);
    lv_obj_set_size(btn2, 160, 80); 

    label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Toggle");
    lv_obj_center(label2);
    lv_obj_add_style(label2, &font_style, 0); 

    label3 = lv_label_create(lv_screen_active());
    lv_label_set_text(label3, "No message.");
    lv_obj_add_style(label3, &font_style, 0); 
    lv_obj_set_width(label3, 200);
    lv_obj_align(label3, LV_ALIGN_CENTER, 0, 200);
    lv_obj_set_style_text_align(label3 , LV_TEXT_ALIGN_CENTER, 0);

}