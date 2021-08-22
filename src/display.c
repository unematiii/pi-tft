#include "display.h"

static napi_ref display_ref;
static int display_ref_count = 0;
static Fb_Data display_fb;

const int BACKLIGHT_GPIO_PIN = 3;

napi_value GetDisplayInstanceJS(napi_env env, napi_callback_info info) {
    napi_value display;

    // Get display obj from ref
    return display_ref_count > 0 &&
            napi_get_reference_value(env, display_ref, &display) == napi_ok ?
            display : NULL;
}

napi_value InitDisplayJS(napi_env env, napi_callback_info info) {
    napi_value display;
    napi_status status;

    // Get display obj from ref
    if (display_ref_count > 0) {
        return GetDisplayInstanceJS(env, info);
    }

    // Create display obj
    status = napi_create_object(env, &display);
    if (status != napi_ok) return NULL;

    // Create reference
    status = napi_create_reference(env, display, ++display_ref_count, &display_ref);
    if (status != napi_ok) return NULL;

    // Init backlight
    InitDisplayBacklightJS(env, info);

    // Init fb
    InitDisplayFbJS(env, info);

    return display;
}

void DestroyDisplay(void* data) {
    if (display_ref_count > 0) {
        DestroyFb(&display_fb);
        display_ref_count = 0;
    }
}

napi_value InitDisplayBacklightJS(napi_env env, napi_callback_info info) {
    // Get display obj
    napi_value display = GetDisplayInstanceJS(env, info);
    if (display == NULL) return NULL;

    // Init backlight GPIO
    pinMode(BACKLIGHT_GPIO_PIN, OUTPUT);

    // Define properties
    napi_status status;
    napi_property_descriptor descriptors[] = {
        { "getBacklightOnOffState", NULL, GetBacklightOnOffStateJS, NULL, NULL, NULL, napi_enumerable, NULL},
        { "turnBacklightOnOff", NULL, TurnBacklightOnOffJS, NULL, NULL, NULL, napi_enumerable, NULL}
    };
    status = napi_define_properties(env,
            display,
            sizeof (descriptors) / sizeof (descriptors[0]),
            descriptors);
    if (status != napi_ok) return NULL;

    return NULL;
}

napi_value InitDisplayFbJS(napi_env env, napi_callback_info info) {
    // Get display obj
    napi_value display = GetDisplayInstanceJS(env, info);
    if (display == NULL) return NULL;

    napi_status status;

    // Parse arguments
    size_t argc = 1;
    napi_value fb_device_path;
    status = napi_get_cb_info(env, info, &argc, &fb_device_path, NULL, NULL);
    if (status != napi_ok) return NULL;

    // Check for string type
    napi_valuetype argument_type;
    status = napi_typeof(env, fb_device_path, &argument_type);
    if (status != napi_ok || argument_type != napi_string) return NULL;

    // Convert to C string
    size_t device_path_len = 128;
    size_t device_path_final_len = 0;
    char device_path[device_path_len];

    status = napi_get_value_string_utf8(env, fb_device_path,
            device_path,
            device_path_len,
            &device_path_final_len);

    if (status != napi_ok || device_path_final_len == 0) return NULL;

    // Init fb
    if (InitFb(device_path, &display_fb) != 0) return NULL;

    // Define properties
    napi_value width;
    napi_value height;

    status = napi_create_uint32(env, display_fb.width, &width);
    if (status != napi_ok) return NULL;

    status = napi_create_uint32(env, display_fb.height, &height);
    if (status != napi_ok) return NULL;

    napi_property_descriptor descriptors[] = {
        { "width", NULL, NULL, NULL, NULL, width, napi_enumerable, NULL},
        { "height", NULL, NULL, NULL, NULL, height, napi_enumerable, NULL},
        { "drawRect", NULL, DrawRectJS, NULL, NULL, NULL, napi_enumerable, NULL},
        { "getPixelAt", NULL, GetPixelAtJS, NULL, NULL, NULL, napi_enumerable, NULL},
        { "setPixelAt", NULL, SetPixelAtJS, NULL, NULL, NULL, napi_enumerable, NULL}
    };
    status = napi_define_properties(env,
            display,
            sizeof (descriptors) / sizeof (descriptors[0]),
            descriptors);
    if (status != napi_ok) return NULL;

    return NULL;
}

napi_value GetBacklightOnOffStateJS(napi_env env, napi_callback_info info) {
    napi_value state;
    napi_status status;

    status = napi_create_int32(env, (int32_t) digitalRead(BACKLIGHT_GPIO_PIN), &state);
    if (status != napi_ok) return NULL;

    return state;
}

napi_value TurnBacklightOnOffJS(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return NULL;

    napi_value state;

    status = napi_coerce_to_bool(env, argv[0], &state);
    if (status != napi_ok) return NULL;

    bool backlight_state;

    status = napi_get_value_bool(env, state, &backlight_state);
    if (status != napi_ok) return NULL;

    digitalWrite(BACKLIGHT_GPIO_PIN, backlight_state ? HIGH : LOW);

    return NULL;
}

napi_value DrawRectJS(napi_env env, napi_callback_info info) {
    // Parse arguments
    size_t argc = 6;
    napi_value argv[argc];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return NULL;

    // Get coordinates
    uint32_t x = 0;
    uint32_t y = 0;
    if (napi_get_value_uint32(env, argv[0], &x) != napi_ok ||
            napi_get_value_uint32(env, argv[1], &y) != napi_ok)
        return NULL;

    // Check bounds
    if (x > (uint32_t) display_fb.width - 1 || y > (uint32_t) display_fb.height - 1)
        return NULL;

    // Get width & height
    uint32_t width = 0;
    uint32_t height = 0;
    if (napi_get_value_uint32(env, argv[2], &width) != napi_ok ||
            napi_get_value_uint32(env, argv[3], &height) != napi_ok)
        return NULL;

    // Get color
    RGB_Color pixel = {0};
    if (!ColorFromJSArray(env, argv[4], &pixel))
        return NULL;

    // Get fill mode
    napi_value should_fill_js;
    bool should_fill;
    status = napi_coerce_to_bool(env, argv[5], &should_fill_js);
    if (status != napi_ok) return NULL;

    status = napi_get_value_bool(env, should_fill_js, &should_fill);
    if (status != napi_ok) return NULL;

    // Set pixel
    DrawRect(x, y, width, height, should_fill, RGBToRGB565(pixel), &display_fb);

    return NULL;
}

napi_value GetPixelAtJS(napi_env env, napi_callback_info info) {
    // Parse arguments
    size_t argc = 2;
    napi_value argv[argc];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return NULL;

    // Get coordinates
    uint32_t x = 0;
    uint32_t y = 0;
    if (napi_get_value_uint32(env, argv[0], &x) != napi_ok ||
            napi_get_value_uint32(env, argv[1], &y) != napi_ok)
        return NULL;

    // Check bounds
    if (x > (uint32_t) display_fb.width - 1 || y > (uint32_t) display_fb.height - 1) {
        return NULL;
    }

    // Get value
    RGB_Color pixel = RGB565ToRGB(GetPixelAt(x, y, &display_fb));

    // Convert to JS array
    napi_value color_array;
    status = napi_create_array_with_length(env, 3, &color_array);
    if (status != napi_ok) return NULL;

    int colors[3] = {pixel.red, pixel.green, pixel.blue};
    for (int i = 0; i < 3; i++) {
        napi_value color;

        if (napi_create_uint32(env, colors[i], &color) != napi_ok)
            return NULL;

        if (napi_set_element(env, color_array, i, color) != napi_ok)
            return NULL;
    }

    return color_array;
}

napi_value SetPixelAtJS(napi_env env, napi_callback_info info) {
    // Parse arguments
    size_t argc = 3;
    napi_value argv[argc];
    napi_status status;

    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (status != napi_ok) return NULL;

    // Get coordinates
    uint32_t x = 0;
    uint32_t y = 0;
    if (napi_get_value_uint32(env, argv[0], &x) != napi_ok ||
            napi_get_value_uint32(env, argv[1], &y) != napi_ok)
        return NULL;

    // Check bounds
    if (x > (uint32_t) display_fb.width - 1 || y > (uint32_t) display_fb.height - 1)
        return NULL;

    // Get color
    RGB_Color pixel = {0};
    if (!ColorFromJSArray(env, argv[2], &pixel))
        return NULL;

    // Set pixel
    SetPixelAt(x, y, RGBToRGB565(pixel), &display_fb);

    return NULL;
}

bool ColorFromJSArray(napi_env env, napi_value value, RGB_Color* pixel) {
    bool is_array;
    if (napi_is_array(env, value, &is_array) != napi_ok || !is_array)
        return 0;

    uint32_t color_array_len = 0;
    if (napi_get_array_length(env, value, &color_array_len) != napi_ok || color_array_len != 3)
        return 0;


    uint32_t color[3];
    for (int i = 0; i < 3; i++) {
        napi_value color_js;

        if (napi_get_element(env, value, i, &color_js) != napi_ok)
            return 0;

        if (napi_get_value_uint32(env, color_js, (color + i)) != napi_ok)
            return 0;
    }

    pixel->red = color[0];
    pixel->green = color[1];
    pixel->blue = color[1];

    return 1;
}
