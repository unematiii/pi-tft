#include <node_api.h>
#include <stdio.h>
#include <wiringPi.h>

#include "buttons.h"
#include "display.h"

napi_status DefineProperties(napi_env env, napi_value module_obj) {
    // Define properties
    napi_property_descriptor descriptors[] = {
        { "initButtons", NULL, InitButtonsJS, NULL, NULL, NULL, napi_enumerable, NULL},
        { "initDisplay", NULL, InitDisplayJS, NULL, NULL, NULL, napi_enumerable, NULL}
    };
    return napi_define_properties(env,
            module_obj,
            sizeof (descriptors) / sizeof (descriptors[0]),
            descriptors);
}

void DestroyModules(void *data) {
    DestroyButtons(data);
    DestroyDisplay(data);
}

napi_value Init(napi_env env, napi_value exports) {
    wiringPiSetup();

    static napi_value module_obj;
    napi_status status;

    // Create exported object
    status = napi_create_object(env, &module_obj);
    if (status != napi_ok) return NULL;

    // Define properties
    status = DefineProperties(env, module_obj);
    if (status != napi_ok) return NULL;

    // Export PiTFT
    status = napi_set_named_property(env, exports, "PiTFT", module_obj);
    if (status != napi_ok) return NULL;

    // Add clean up hook
    status = napi_add_env_cleanup_hook(env, &DestroyModules, NULL);
    if (status != napi_ok) return NULL;

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
