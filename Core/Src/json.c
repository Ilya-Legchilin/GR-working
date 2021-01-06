#include "json.h"
#include <stdlib.h>

const uint8_t  json_bool_true = 1;
const uint8_t  json_bool_false = 0;

#define safe_free(x) do{if (x) {((uint8_t*)(x))[0] = 'z'; free(x);}}while(0)


json_array_t* json_new_array(json_valuetype_t type, uint8_t len, const void  *data)
{
    json_array_t* arr = malloc(sizeof(json_array_t));
    arr->type = type;
    arr->len = len;

    switch (type)
    {
    case JSON_BOOL:
        arr->data = malloc(len);
        uint8_t i;
        for(i = 0; i < len; ++i)
            ((uint8_t*)arr->data)[i] = ((uint8_t*)data)[i];
        break;
    case JSON_INT:
        arr->data = malloc(4 * len);
        for(i = 0; i < len; ++i)
            ((uint32_t*)arr->data)[i] = ((uint32_t*)data)[i];
        break;
    case JSON_FLOAT:
        arr->data = malloc(4 * len);
        for(i = 0; i < len; ++i)
            ((float*)arr->data)[i] = ((float*)data)[i];
        break;
    default:
        safe_free(arr);
        return 0;
        break;
    }
    return arr;
}

json_ret_t json_free_array(json_array_t *arr)
{
    if (!arr) return JSON_ERROR;
    arr->type = JSON_NOTHING;
    arr->len = 0;
    safe_free((void*)arr->data);
    arr->data = 0;
    return JSON_OK;
}

json_value_t* json_new_value(json_valuetype_t type, const void  *data)
{
    json_value_t *value = malloc(sizeof(json_value_t));

    if (!value) return 0;

    value->type = type;

    switch (type)
    {
    case JSON_ARRAY:
        if (!data) break;
        value->data = malloc(sizeof(json_array_t*));
        *((json_array_t**)value->data) = json_new_array(((json_array_t*)data)->type, ((json_array_t*)data)->len, ((json_array_t*)data)->data);
        break;
    case JSON_OBJECTS:
        value->data = malloc(sizeof(json_t*));
        *((json_t**)value->data) = (json_t*)data;
        break;
    case JSON_INT:
        value->data = malloc(sizeof(uint32_t));
        *((uint32_t*)(value->data)) = *((uint32_t*)data);
        break;
    case JSON_INT_WIHT_DIV:
        value->data = malloc(sizeof(uint32_with_div_t));
        ((uint32_with_div_t*)value->data)->div = ((uint32_with_div_t*)data)->div;
        ((uint32_with_div_t*)value->data)->_int = ((uint32_with_div_t*)data)->_int;
        break;
    case JSON_FLOAT:
        value->data = malloc(sizeof(float));
        *((float*)(value->data)) = *((float*)data); ///TODO
        break;
    case JSON_STRING:
        uint8_t data_len = spec_strlen( (const_string_t)data ) + 1;
        value->data = malloc(data_len * sizeof(char));
        if (value->data) {
            spec_strcp((const_string_t)data, (char*)value->data);
            break;
        } else {
            safe_free(value);
        }
        break;
    case JSON_BOOL:
        value->data = malloc(sizeof(char));
        *((uint8_t*)value->data) = *((const uint8_t  *)data);
        break;
    default:
        safe_free(value);
        return 0;
        break;
    }

    return value;
}

json_ret_t json_free_value(json_value_t *value)
{
    if (!value) return JSON_ERROR;

    if (value->type == JSON_ARRAY)
        json_free_array((json_array_t*)(value->data));

    if (value->type != JSON_OBJECTS)
        safe_free(value->data);
}

json_object_t* json_new_object(const_string_t key, json_value_t *value)
{
    if (!value) return 0;

    json_object_t *obj = malloc(sizeof(json_object_t));

    uint8_t key_len = strlen(key) + 1;

    if (key_len < 2)
    {
        safe_free(obj);
        return 0;
    }

    obj->key = malloc(key_len * sizeof(char));

    if (!(obj->key))
    {
        safe_free(obj);
        return 0;
    }

    strcp(key, obj->key);

    obj->value = value;

    return obj;
}

json_ret_t json_free_object(json_object_t *obj)
{
    json_free_value(obj->value);

    safe_free(obj->value);
    safe_free(obj->key);

    return JSON_OK;
}

json_ret_t json_add_object(json_t *json, json_object_t *obj)
{
    uint8_t i = 0;
    for (i = 0; i < sizeof(json_t)/sizeof(json_object_t*); ++i) {
        if (json->objects[i] == 0) {
            json->objects[i] = obj;
            return JSON_OK;
        }
    }

    return JSON_ERROR;
}

json_ret_t json_delete_object(json_t *json, const_string_t key)
{
    uint8_t i = 0;
    while (json->objects[i] != 0)
    {
        if (strstr(json->objects[i]->key, key))
            break;
        i += 1;
    }

    if (json->objects[i] == 0) return JSON_ERROR;

    json_free_object(json->objects[i]);

    i += 1;

    while (json->objects[i] != 0)
    {
        json->objects[i - 1] = json->objects[i];
        i += 1;
    }
    json->objects[i - 1] = 0;

    return JSON_OK;
}


json_t* new_json()
{
    json_t* json = malloc(sizeof(json_t));

    uint8_t i = 0;
    for (i = 0; i < sizeof(json_t)/sizeof(json_object_t*); ++i) {
        if (json->objects[i] != 0)
            json->objects[i] = 0;
    }

    return json;
}

json_ret_t json_free(json_t* json)
{
    uint8_t i = 0;

    for (i = 0; i < sizeof(json_t)/sizeof(json_object_t*); ++i) {
        if (json->objects[i] != 0) {
            json_free_object(json->objects[i]);
            safe_free(json->objects[i]);
            json->objects[i] = 0;
        }
    }

    safe_free(json);
}

json_ret_t json_change_object(json_t *json, const_string_t key, json_value_t *value)
{
    uint8_t i = 0;
    while (json->objects[i] != 0)
    {
        if (strstr(key, json->objects[i]->key))
            break;
        i++;
    }

    if (json->objects[i] == 0)
    {
        json_free_value(value);
        return JSON_ERROR;
    }

    json_free_object(json->objects[i]);

    json->objects[i] = json_new_object(key, value);

    return JSON_OK;
}

uint16_t json_print_object(json_object_t *obj, char* out)
{
    uint16_t p = 0;

    out[p++] = '\"';
    while(obj->key[p - 1] != '\0')
        out[p] = obj->key[p++ - 1];
    out[p++] = '\"';
    out[p++] = ':';

    switch (obj->value->type)
    {
    case JSON_OBJECTS:
        p += json_print_to_str(*((json_t**)(obj->value->data)), out + p);
        break;
    case JSON_BOOL:
        if ( *((uint8_t*)obj->value->data) ) {
            out[p++] = 't';
            out[p++] = 'r';
            out[p++] = 'u';
            out[p++] = 'e';
        } else {
            out[p++] = 'f';
            out[p++] = 'a';
            out[p++] = 'l';
            out[p++] = 's';
            out[p++] = 'e';
        }
        break;
    case JSON_STRING:
        uint16_t k = 0;
        out[p++] = '\"';
        while (*((char*)obj->value->data + k) != '\0') {
            out[p++] = *((char*)obj->value->data + k);
            k++;
        }
        out[p++] = '\"';
        break;
    case JSON_INT:
        p += uint32_to_str( *((uint32_t*)obj->value->data), out + p );
        break;
    case JSON_FLOAT:
        p += float_to_str( *((float*)obj->value->data), out + p);
        break;
    case JSON_INT_WIHT_DIV:
        p += uint32withoffset_to_str(((uint32_with_div_t*)obj->value->data)->_int, ((uint32_with_div_t*)obj->value->data)->div, out + p);
        break;
    case JSON_ARRAY:
        out[p++] = '[';
        out[p++] = ']';
        break;
    default:
        out[p++] = '\"';
        out[p++] = '\"';
        break;
    }
    return p;
}

uint16_t json_print_to_str(json_t *json, char* out)
{
    uint8_t i = 0;
    uint16_t p = 0;
    out[p++] = '{';
    while (json->objects[i] != 0)
    {
        if (i)
            out[p++] = ',';

        p += json_print_object(json->objects[i], out + p);

        i += 1;
    }
    out[p++] = '}';
    out[p++] = '\0';

    return p;
}


static inline uint8_t compare_key(const_string_t key, const_string_t src, uint16_t p)
{
    //find start of key
    while(src[p - 1] != '\"') p -= 1;

    uint8_t i = 0;
    while( (src[p] != '\"') && (key[i] != '\"') )
    {
        if (src[p] != key[i]) return 0;
        p += 1;
        i += 1;
    }

    if ((src[p] == '\"') && (key[i] == '\0'))
        return 1;

    return 0;
}

static json_value_t * json_prepair_value(const_string_t src, uint16_t p)
{
    //skip spaces
    while(src[p] == ' ') p += 1;

    // if true
    if (src[p] == 't')
    {
        if (src[p + 1] == 'r' && \
            src[p + 2] == 'u' && \
            src[p + 3] == 'e')
        {

            uint8_t a = 1;
            return json_new_value(JSON_BOOL, &a);
        }
        return 0;
    }

    // if false
    if (src[p] == 'f')
    {
        if (src[p + 1] == 'a' && \
            src[p + 2] == 'l' && \
            src[p + 3] == 's' && \
            src[p + 4] == 'e'
            )
        {
            uint8_t tmp = 0;
            return json_new_value(JSON_BOOL, &tmp);
        }
        return 0;
    }

    // if uint ot float
    if ( (src[p] == '-') || (('0' <= src[p]) && (src[p] <= '9')) )
    {
        uint8_t digits = 0;
        uint8_t is_float = 0;
        uint8_t is_plus = 1;

        if (src[p] == '-') {
            is_plus = 0;
            p += 1;
        }

        uint16_t p1 = p;
        while( (src[p1] != ',') && (src[p1] != '}'))
        {
            digits += 1;
            if (src[p1] == '.') {
                is_float += 1;
                digits -= 1;
            } else {
                if ( (src[p1] < '0') || ('9' < src[p1]) )
                    return 0;
            }
            p1 += 1;
        }

        if (digits > 10) return 0;

        if (is_float)
        {
            float f = 0;
            while(src[p] != '.')
            {
                f = f * 10 + (src[p] - '0');
                p += 1;
            }
            //skip '.'
            p += 1;
            float pow = 0.100000;
            while( ('0' <= src[p]) && (src[p] <= '9') )
            {
                f += pow * (src[p] - '0');
                pow /= 10;
                p += 1;
            }
            if (!is_plus) f = -f;
            return json_new_value(JSON_FLOAT, &f);
        }
        else // if int
        {
            uint32_t i = 0;
            while( ('0' <= src[p]) && (src[p] <= '9') )
            {
                i = i * 10 + (src[p] - '0');
                p += 1;
            }
            return json_new_value(JSON_INT, &i);
        }
    }

    // if string
    if (src[p] == '\"')
    {
        // skip "
        p += 1;
        return json_new_value(JSON_STRING, src + p);
    }

    // if object
    if (src[p] == '{')
    {
        return 0;
    }

    // if array
    if (src[p] == '[')
    {
        return 0;
    }

    return 0;
}

json_value_t* json_get_value_from_str(const_string_t key, const_string_t src)
{
    json_value_t* res = 0;

    uint16_t p = 0;

    uint8_t quotes = 0;
    uint16_t last_quote_p = 0;
    uint8_t number_curly_string_o = 0;
    uint8_t number_curly_string_c = 0;

    while(src[p] != '\0')
    {
        switch (src[p])
        {
        case '{':
            number_curly_string_o += 1;
            break;
        case '}':
            number_curly_string_c += 1;
            if (number_curly_string_c > number_curly_string_o) return res;
            number_curly_string_o -= 1;
            number_curly_string_c -= 1;
            break;
        case '\"':
            // finding only on first level
            if ( !(number_curly_string_o == 1 && number_curly_string_c == 0) ) break;

            quotes += 1;

            // find end of keys
            // {"asas":"hgjhv", "asdasda": 123}
            //       ^                  ^
            if ( !(quotes & 1) && (src[p + 1] == ':')) {
                if (compare_key(key, src, p))
                    return json_prepair_value(src, p + 2);
            }
            break;
        default:
            break;
        }
        p += 1;
    }

    return res;
}
