#include "buttons.h"

static napi_threadsafe_function buttons_state_threadsafe_fn;
static bool handlers_initialized = false;

const int BUTTON_UP_GPIO_PIN = 4;
const int BUTTON_DOWN_GPIO_PIN = 5;

static t_button_state buttown_down_state = {
    .button = BUTTON_DOWN_GPIO_PIN,
    .state = HIGH
};

static t_button_state buttown_up_state = {
    .button = BUTTON_UP_GPIO_PIN,
    .state = HIGH
};

// TODO Error handling

napi_value InitButtonsJS(napi_env env, napi_callback_info info) {
    if (handlers_initialized) return NULL;

    napi_status status;

    size_t argc = 1;
    napi_value callback_fn;
    status = napi_get_cb_info(env, info, &argc, &callback_fn, NULL, NULL);
    if (status != napi_ok) return NULL;

    napi_valuetype argument_type;

    status = napi_typeof(env, callback_fn, &argument_type);
    if (status != napi_ok || argument_type != napi_function) return NULL;

    pinMode(BUTTON_DOWN_GPIO_PIN, INPUT);
    pinMode(BUTTON_UP_GPIO_PIN, INPUT);

    buttown_down_state.state = digitalRead(BUTTON_DOWN_GPIO_PIN);
    buttown_up_state.state = digitalRead(BUTTON_UP_GPIO_PIN);

    napi_value work_name;
    status = napi_create_string_utf8(
            env,
            "Buttons interrupt handler",
            NAPI_AUTO_LENGTH,
            &work_name
            );
    if (status != napi_ok) return NULL;

    status = napi_create_threadsafe_function(
            env,
            callback_fn,
            NULL,
            work_name,
            0,
            1,
            NULL,
            &DestroyButtonsHandler,
            NULL,
            &ButtonsStateCallback,
            &buttons_state_threadsafe_fn
            );
    if (status != napi_ok) return NULL;

    wiringPiISR(BUTTON_DOWN_GPIO_PIN, INT_EDGE_BOTH, &HandleDownButtonInterrupt);
    wiringPiISR(BUTTON_UP_GPIO_PIN, INT_EDGE_BOTH, &HandleUpButtonInterrupt);

    return NULL;
}

void DestroyButtons(void* data) {
    if (napi_release_threadsafe_function(buttons_state_threadsafe_fn, napi_tsfn_abort) == napi_ok) {
        handlers_initialized = false;
    }
}

void HandleDownButtonInterrupt() {
    int state = digitalRead(BUTTON_DOWN_GPIO_PIN);

    if (state != buttown_down_state.state) {
        buttown_down_state.state = state;

        napi_call_threadsafe_function(
                buttons_state_threadsafe_fn,
                (void *) &buttown_down_state,
                napi_tsfn_nonblocking
                );
    }
}

void HandleUpButtonInterrupt() {
    int state = digitalRead(BUTTON_UP_GPIO_PIN);

    if (state != buttown_up_state.state) {
        buttown_up_state.state = state;

        napi_call_threadsafe_function(
                buttons_state_threadsafe_fn,
                (void *) &buttown_up_state,
                napi_tsfn_nonblocking
                );
    }
}

void ButtonsStateCallback(napi_env env, napi_value js_callback, void* context, void* data) {
    napi_value global;
    napi_status status;

    status = napi_get_global(env, &global);
    if (status != napi_ok) return;

    int argc = 2;
    napi_value argv[2];
    t_button_state *button_state = (t_button_state *) data;

    status = napi_create_uint32(env, (uint32_t) button_state->button, argv);
    if (status != napi_ok) return;

    status = napi_create_uint32(env, (uint32_t) button_state->state, argv + 1);
    if (status != napi_ok) return;

    napi_value return_val;
    napi_call_function(env, global, js_callback, argc, argv, &return_val);
}

void DestroyButtonsHandler(napi_env env, void* finalize_data, void* finalize_hint) {
    handlers_initialized = false;
}
