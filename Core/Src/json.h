#ifndef JSON_H
#define JSON_H

#include "misc.h"

extern const uint8_t json_bool_true;
extern const uint8_t json_bool_false;

#define JSON_VALUE_TRUE (&json_bool_true)
#define JSON_VALUE_FALSE (&json_bool_false)

typedef enum {
    JSON_NOTHING=0,
    JSON_ARRAY,
    JSON_OBJECTS,
    JSON_INT,
    JSON_INT_WIHT_DIV,
    JSON_FLOAT,
    JSON_STRING,
    JSON_BOOL
} json_valuetype_t;

typedef struct {
    json_valuetype_t type;
    uint8_t len;
    void *data;
} json_array_t; //now only BOOL INT FLOAT

typedef struct {
    uint32_t _int;
    uint8_t div;
} uint32_with_div_t;

typedef struct {
    json_valuetype_t type;
    void *data;
} json_value_t;

typedef struct {
    char           *key;
    json_value_t   *value;
} json_object_t;

typedef enum {
    JSON_OK,
    JSON_ERROR
} json_ret_t;

typedef struct {
    json_object_t* objects[24];
} json_t; //only global variables

json_array_t* json_new_array(json_valuetype_t type, uint8_t len, const void *data);
json_ret_t json_free_array(json_array_t *arr);

json_value_t* json_new_value(json_valuetype_t type, const void *data);
json_ret_t json_free_value(json_value_t *value);

json_object_t* json_new_object(const_string_t key, json_value_t *value);
json_ret_t json_free_object(json_object_t *obj);

json_t* new_json();
json_ret_t json_free(json_t* json);
json_ret_t json_add_object(json_t *json, json_object_t *obj);
json_ret_t json_delete_object(json_t *json, const_string_t key);

json_ret_t json_change_object(json_t *json, const_string_t key, json_value_t *value);

uint16_t json_print_object(json_object_t *obj, char* out);
uint16_t json_print_to_str(json_t *json, char* out);

json_value_t * json_get_value_from_str(const_string_t key, const_string_t src);

#define JSON_ADD_INT(j, k, i) do{tmp_int = i; json_add_object(j, json_new_object(k, json_new_value(JSON_INT, &tmp_int)));}while(0)
#define JSON_ADD_INT_WITH_DIV(j, k, i, d) do{tmp_int_wd._int = i; tmp_int_wd.div = d; json_add_object(j, json_new_object(k, json_new_value(JSON_INT_WIHT_DIV, &tmp_int_wd)));}while(0)
#define JSON_ADD_STRING(j, k, s) do{json_add_object(j, json_new_object(k, json_new_value(JSON_STRING, s)));}while(0)

#define JSON_GET_INT_FROM_STR(str, key, out) do{                \
    tmp_json_value = json_get_value_from_str(key, rx_str);      \
    if (tmp_json_value && (tmp_json_value->type == JSON_INT))   \
        out = *((uint32_t*)tmp_json_value->data);               \
    json_free_value(tmp_json_value);                            \
    if (tmp_json_value) free(tmp_json_value);                   \
    }while(0)

#define JSON_GET_STR_FROM_STR(str, key, out) do{                    \
    tmp_json_value = json_get_value_from_str(key, rx_str);          \
    if (tmp_json_value && (tmp_json_value->type == JSON_STRING))    \
        strcp((char*)tmp_json_value->data, out);                    \
    json_free_value(tmp_json_value);                                \
    if (tmp_json_value) free(tmp_json_value);                       \
    }while(0)


#endif // JSON_H
