#include <stdbool.h>
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
#include <lib/toolbox/path.h>

#include "../helpers/mem.h"
#include "../helpers/i2c.h"
#include "../helpers/file_io.h"

struct BoilerplateScene2 {
    View* view;
    BoilerplateScene2Callback callback;
    void* context;
};

typedef struct {
    char* memory_type;
    char* status_string;
    char* file_name;
} BoilerplateScene2Model;

void boilerplate_scene_2_set_callback(
    BoilerplateScene2* instance,
    BoilerplateScene2Callback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void boilerplate_scene_2_draw(Canvas* canvas, BoilerplateScene2Model* model) {
    // UNUSED(model);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, "Flashing");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, model->memory_type);
    canvas_draw_str_aligned(canvas, 0, 32, AlignLeft, AlignTop, model->status_string);
    canvas_draw_str_aligned(canvas, 0, 42, AlignLeft, AlignTop, model->file_name);
}

static void boilerplate_scene_2_model_init(BoilerplateScene2Model* const model) {
    // UNUSED(model);
    model->status_string = "Press OK to write";
    model->file_name = "Press Down to select file";
}

bool boilerplate_scene_2_input(InputEvent* event, void* context) {
    furi_assert(context);
    BoilerplateScene2* instance = context;
    Boilerplate* app = instance->context;
    FuriString* initial_path;
    if(event->type == InputTypeRelease) {
        switch(event->key) {
        case InputKeyBack:
            furi_string_set_str(app->file_path, "");
            with_view_model(
                instance->view,
                BoilerplateScene2Model * model,
                {
                    UNUSED(model);
                    instance->callback(BoilerplateCustomEventScene2Back, instance->context);
                },
                true);
            break;
        case InputKeyLeft:
            break;
        case InputKeyRight:
            break;
        case InputKeyUp:
            break;
        case InputKeyDown:
            initial_path = furi_string_alloc();
            furi_string_set(initial_path, EXT_PATH(DEFAULT_DIR));
            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options, DEFAULT_EXTENSION, &I_hex_10px);
            browser_options.hide_ext = false;
            bool success = dialog_file_browser_show(
                app->dialogs, app->file_path, initial_path, &browser_options);
            furi_string_free(initial_path);
            if(success) {
                boilerplate_play_happy_bump(instance->context);
                boilerplate_play_input_sound(instance->context);
                FuriString* filename = furi_string_alloc();
                path_extract_filename(app->file_path, filename, false);
                with_view_model(
                    instance->view,
                    BoilerplateScene2Model * model,
                    { model->file_name = (char*)furi_string_get_cstr(filename); },
                    true);
            } else {
                boilerplate_play_bad_bump(instance->context);
                boilerplate_play_input_sound(instance->context);
            }
            // furi_string_free(filename);
            furi_delay_ms(300);
            boilerplate_stop_all_sound(instance->context);
            break;
        case InputKeyOk:
            with_view_model(
                instance->view,
                BoilerplateScene2Model * model,
                {
                    // UNUSED(model);
                    char* final_message = "Completed. Press OK to retry.";
                    if(!furi_string_equal_str(app->file_path, "")) {
                        bool two_bytes_address = false;
                        if(app->mem_size > 8192) {
                            two_bytes_address = true;
                        }
                        for(int i = 0; i < (int)(app->mem_size / I2C_BUF_SIZE); i++) {
                            boilerplate_led_set_rgb(instance->context, 255, 0, 0);
                            FURI_LOG_I(TAG, "Pass №: %d", i);
                            fio_read_file_to_buf(
                                furi_string_get_cstr(app->file_path), i2c_buf, (i * I2C_BUF_SIZE));
                            boilerplate_led_set_rgb(instance->context, 0, 255, 0);
                            i2c_write_buf((i * I2C_BUF_SIZE), two_bytes_address);
                            boilerplate_led_set_rgb(instance->context, 0, 0, 255);
                            if(!i2c_verify_buf((i * I2C_BUF_SIZE), two_bytes_address)) {
                                FURI_LOG_I(TAG, "Verify: error");
                                final_message = "Verification error!";
                                break;
                            } else {
                                FURI_LOG_I(TAG, "Verify: ok");
                            }
                        }
                    } else {
                        final_message = "File not selected!";
                    }
                    model->status_string = final_message;
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

void boilerplate_scene_2_exit(void* context) {
    furi_assert(context);
}

void boilerplate_scene_2_enter(void* context) {
    furi_assert(context);
    BoilerplateScene2* instance = (BoilerplateScene2*)context;
    Boilerplate* app = instance->context;
    BoilerplateScene2Model* model;
    uint8_t soml = memory_get_list_size();
    uint8_t index = value_index_uint32(app->mem_size, memory_capacity, soml);
    char* mm = (char*)memory_model[index];
    with_view_model(
        instance->view,
        model,
        {
            model->memory_type = mm;
            boilerplate_scene_2_model_init(model);
        },
        true);
}

BoilerplateScene2* boilerplate_scene_2_alloc() {
    BoilerplateScene2* instance = malloc(sizeof(BoilerplateScene2));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(BoilerplateScene2Model));
    view_set_context(instance->view, instance); // furi_assert crashes in events without this
    view_set_draw_callback(instance->view, (ViewDrawCallback)boilerplate_scene_2_draw);
    view_set_input_callback(instance->view, boilerplate_scene_2_input);
    view_set_enter_callback(instance->view, boilerplate_scene_2_enter);
    view_set_exit_callback(instance->view, boilerplate_scene_2_exit);

    with_view_model(
        instance->view,
        BoilerplateScene2Model * model,
        { boilerplate_scene_2_model_init(model); },
        true);

    return instance;
}

void boilerplate_scene_2_free(BoilerplateScene2* instance) {
    furi_assert(instance);

    with_view_model(instance->view, BoilerplateScene2Model * model, { UNUSED(model); }, true);
    view_free(instance->view);
    free(instance);
}

View* boilerplate_scene_2_get_view(BoilerplateScene2* instance) {
    furi_assert(instance);
    return instance->view;
}
