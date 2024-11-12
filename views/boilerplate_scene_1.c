#include "../boilerplate.h"
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <dolphin/dolphin.h>
#include "../helpers/boilerplate_haptic.h"
#include "../helpers/boilerplate_speaker.h"
#include "../helpers/boilerplate_led.h"
#include <lib/toolbox/value_index.h>
#include <datetime/datetime.h>

#include "../helpers/mem.h"
#include "../helpers/i2c.h"
#include "../helpers/file_io.h"

struct BoilerplateScene1 {
    View* view;
    BoilerplateScene1Callback callback;
    void* context;
};

typedef struct {
    char* memory_type;
    char* status_string;
    char* file_name;
    DateTime date_time;
} BoilerplateScene1Model;

void boilerplate_scene_1_set_callback(
    BoilerplateScene1* instance,
    BoilerplateScene1Callback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void boilerplate_scene_1_draw(Canvas* canvas, BoilerplateScene1Model* model) {
    // UNUSED(model);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, "Reading");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, model->memory_type);
    canvas_draw_str_aligned(canvas, 0, 32, AlignLeft, AlignTop, model->status_string);
    canvas_draw_str_aligned(canvas, 0, 42, AlignLeft, AlignTop, model->file_name);
}

static void boilerplate_scene_1_model_init(BoilerplateScene1Model* const model) {
    // UNUSED(model);
    model->status_string = "Press OK to read";
    model->file_name = "";
}

bool boilerplate_scene_1_input(InputEvent* event, void* context) {
    furi_assert(context);
    BoilerplateScene1* instance = context;
    Boilerplate* app = instance->context;
    if(event->type == InputTypeRelease) {
        switch(event->key) {
        case InputKeyBack:
            with_view_model(
                instance->view,
                BoilerplateScene1Model * model,
                {
                    UNUSED(model);
                    instance->callback(BoilerplateCustomEventScene1Back, instance->context);
                },
                true);
            break;
        case InputKeyLeft:
        case InputKeyRight:
        case InputKeyUp:
        case InputKeyDown:
            break;
        case InputKeyOk:
            with_view_model(
                instance->view,
                BoilerplateScene1Model * model,
                {
                    // UNUSED(model);
                    char filename[30] = "";
                    char fullpath[100] = "";
                    furi_hal_rtc_get_datetime(&model->date_time);
                    snprintf(
                        filename,
                        30,
                        "%s-%.4d%.2d%.2d_%.2d%.2d%.2d.bin",
                        model->memory_type,
                        model->date_time.year,
                        model->date_time.month,
                        model->date_time.day,
                        model->date_time.hour,
                        model->date_time.minute,
                        model->date_time.second);
                    snprintf(fullpath, 100, "%s/%s", EXT_PATH(DEFAULT_DIR), filename);
                    bool two_bytes_address = false;
                    if(app->mem_size > 8192) {
                        two_bytes_address = true;
                    }
                    for(int i = 0; i < (int)(app->mem_size / I2C_BUF_SIZE); i++) {
                        boilerplate_led_set_rgb(instance->context, 0, 255, 0);
                        i2c_read_buf((i * I2C_BUF_SIZE), two_bytes_address);
                        boilerplate_led_set_rgb(instance->context, 255, 0, 0);
                        fio_write_buf_to_file(fullpath, i2c_buf);
                    }
                    model->status_string = "Completed. Press OK to retry.";
                    model->file_name = filename;
                    FURI_LOG_I(TAG, "Writed to file: %s", fullpath);
                    boilerplate_play_happy_bump(instance->context);
                    boilerplate_play_input_sound(instance->context);
                    furi_delay_ms(300);
                    boilerplate_stop_all_sound(instance->context);
                },
                true);
            break;
        case InputKeyMAX:
            break;
        }
    }
    return true;
}

void boilerplate_scene_1_exit(void* context) {
    furi_assert(context);
}

void boilerplate_scene_1_enter(void* context) {
    furi_assert(context);
    BoilerplateScene1* instance = (BoilerplateScene1*)context;
    Boilerplate* app = instance->context;
    BoilerplateScene1Model* model;
    uint8_t soml = memory_get_list_size();
    uint8_t index = value_index_uint32(app->mem_size, memory_capacity, soml);
    char* mm = (char*)memory_model[index];
    with_view_model(
        instance->view,
        model,
        {
            model->memory_type = mm;
            boilerplate_scene_1_model_init(model);
        },
        true);
}

BoilerplateScene1* boilerplate_scene_1_alloc() {
    BoilerplateScene1* instance = malloc(sizeof(BoilerplateScene1));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(BoilerplateScene1Model));
    view_set_context(instance->view, instance); // furi_assert crashes in events without this
    view_set_draw_callback(instance->view, (ViewDrawCallback)boilerplate_scene_1_draw);
    view_set_input_callback(instance->view, boilerplate_scene_1_input);
    view_set_enter_callback(instance->view, boilerplate_scene_1_enter);
    view_set_exit_callback(instance->view, boilerplate_scene_1_exit);

    with_view_model(
        instance->view,
        BoilerplateScene1Model * model,
        { boilerplate_scene_1_model_init(model); },
        true);

    return instance;
}

void boilerplate_scene_1_free(BoilerplateScene1* instance) {
    furi_assert(instance);

    with_view_model(instance->view, BoilerplateScene1Model * model, { UNUSED(model); }, true);
    view_free(instance->view);
    free(instance);
}

View* boilerplate_scene_1_get_view(BoilerplateScene1* instance) {
    furi_assert(instance);
    return instance->view;
}
