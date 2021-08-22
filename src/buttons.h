#include <node_api.h>
#include <wiringPi.h>

#ifndef PITFT_BUTTONS_H
#define PITFT_BUTTONS_H

typedef struct button_state {
    int button;
    int state;
} t_button_state;

napi_value InitButtonsJS(napi_env env, napi_callback_info info);
void DestroyButtons(void* data);

void ButtonsStateCallback(napi_env env, napi_value js_callback, void* context, void* data);
void HandleUpButtonInterrupt();
void HandleDownButtonInterrupt();
void DestroyButtonsHandler(napi_env env, void* finalize_data, void* finalize_hint);

#endif /* PITFT_BUTTONS_H */
