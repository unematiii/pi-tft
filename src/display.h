#include <node_api.h>
#include <stdbool.h>
#include <wiringPi.h>

#include "fb.h"

#ifndef PITFT_DISPLAY_H
#define PITFT_DISPLAY_H

napi_value GetDisplayInstanceJS(napi_env env, napi_callback_info info);

napi_value InitDisplayJS(napi_env env, napi_callback_info info);
void DestroyDisplay(void* data);

napi_value InitDisplayBacklightJS(napi_env env, napi_callback_info info);
napi_value InitDisplayFbJS(napi_env env, napi_callback_info info);

napi_value GetBacklightOnOffStateJS(napi_env env, napi_callback_info info);
napi_value TurnBacklightOnOffJS(napi_env env, napi_callback_info info);

napi_value DrawRectJS(napi_env env, napi_callback_info info);
napi_value GetPixelAtJS(napi_env env, napi_callback_info info);
napi_value SetPixelAtJS(napi_env env, napi_callback_info info);

bool ColorFromJSArray(napi_env env, napi_value value, RGB_Color* pixel);

#endif /* PITFT_DISPLAY_H */
