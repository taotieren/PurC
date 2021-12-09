/**
 * @file purc-variant.h
 * @author 
 * @date 2021/07/02
 * @brief The API for variant.
 *
 * Copyright (C) 2021 FMSoft <https://www.fmsoft.cn>
 *
 * This file is a part of PurC (short for Purring Cat), an HVML interpreter.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PURC_PURC_VARIANT_H
#define PURC_PURC_VARIANT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "purc-macros.h"
#include "purc-rwstream.h"
#include "purc-utils.h"

struct purc_variant;
typedef struct purc_variant purc_variant;
typedef struct purc_variant* purc_variant_t;

#define PURC_VARIANT_INVALID            ((purc_variant_t)(0))

#define PURC_VARIANT_BADSIZE            ((size_t)(-1))

PCA_EXTERN_C_BEGIN

typedef bool (*pcvar_msg_handler) (purc_variant_t source,
        purc_atom_t msg_type, void *ctxt,
        size_t nr_args, purc_variant_t *argv);

/**
 * Adds ref for a variant value
 *
 * @param value: variant value to be operated
 *
 * Returns: A purc_variant_t on success, NULL on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT unsigned int purc_variant_ref(purc_variant_t value);

/**
 * substract ref for a variant value. When ref is zero, releases the resource
 * occupied by the data
 *
 * @param value: variant value to be operated
 *
 * Note: When the reference count reaches zero, the system will release
 *      all memory used by value.
 *
 * Since: 0.0.1
 */
PCA_EXPORT unsigned int purc_variant_unref(purc_variant_t value);

/**
 * Creates a variant value of undefined type.
 *
 * Returns: A purc_variant_t with undefined type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_undefined(void);


/**
 * Creates a variant value of null type.
 *
 * Returns: A purc_variant_t with null type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_null(void);


/**
 * Creates a variant value of boolean type.
 *
 * @param b: the initial value of created data
 *
 * Returns: A purc_variant_t with boolean type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_boolean(bool b);


/**
 * Creates a variant value of number type.
 *
 * @param d: the initial value of created data
 *
 * Returns: A purc_variant_t with number type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_number(double d);


/**
 * Creates a variant value of long int type.
 *
 * @param u64: the initial value of unsigned long int type
 *
 * Returns: A purc_variant_t with long integer type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_ulongint(uint64_t u64);


/**
 * Creates a variant value of long int type.
 *
 * @param u64: the initial value of signed long int type
 *
 * Returns: A purc_variant_t with long int type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_longint(int64_t u64);


/**
 * Creates a variant value of long double type.
 *
 * @param d: the initial value of created data
 *
 * Returns: A purc_variant_t with long double type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_longdouble(long double lf);


/**
 * Creates a variant value of string type.
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_string(const char* str_utf8, bool check_encoding);

/**
 * Creates a variant value of string type from a static C string.
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.2
 */
PCA_EXPORT purc_variant_t
purc_variant_make_string_static(const char* str_utf8, bool check_encoding);

/**
 * Creates a variant value of string type by reusing the string buffer.
 * The buffer will be released by calling free() when the variant is destroyed.
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding.
 * @param sz_buff: the size of the buffer.
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding.
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.2
 */
PCA_EXPORT purc_variant_t
purc_variant_make_string_reuse_buff(char* str_utf8, size_t sz_buff,
        bool check_encoding);

/**
 * Creates a variant value of string type by using non-null-terminated buffer
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding
 * @param len: the length of string to be used at most
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.5
 */
PCA_EXPORT purc_variant_t
purc_variant_make_string_ex(const char* str_utf8, size_t len,
        bool check_encoding);

/**
 * Gets the pointer of string which is encapsulated in string type.
 *
 * @param value: the data of string type
 *
 * Returns: The pointer of char string, or NULL if value is not string type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT const char* purc_variant_get_string_const(purc_variant_t value);

/**
 * Get the length in bytes of a string variant value.
 *
 * @param value: the variant value of string type
 * @param length: the buffer to receive the length (including the terminating
 *  null byte) of the string.
 *
 * Returns: True on success, otherwise False (the variant is not a string).
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_string_bytes(purc_variant_t value, size_t *length);

/**
 * Get the length in bytes of a string variant value.
 *
 * @param value: the variant value of string type
 *
 * Returns: The length in bytes of the string variant;
 *  \PURC_VARIANT_BADSIZE (-1) if the variant is not a string.
 *
 * Note: This function is deprecated, use \purc_variant_string_bytes
 *  instead.
 *
 * Since: 0.0.1
 */
static inline size_t purc_variant_string_length(purc_variant_t value)
{
    size_t len;
    if (!purc_variant_string_bytes(value, &len))
        return PURC_VARIANT_BADSIZE;
    return len;
}

/**
 * Creates a variant value of atom string type.
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding
 *
 * Returns: A purc_variant_t with atom string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t purc_variant_make_atom_string(const char* str_utf8,
        bool check_encoding);


/**
 * Creates a variant value of atom string type.
 *
 * @param str_utf8: the pointer of a string which is in UTF-8 encoding
 * @param check_encoding: whether check str_utf8 in UTF-8 encoding
 *
 * Returns: A purc_variant_t with atom string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_atom_string_static(const char* str_utf8,
        bool check_encoding);


/**
 * Gets the pointer of string which is encapsulated in atom string type.
 *
 * @param value: the data of atom string type
 *
 * Returns: The pointer of const char string,
 *      or NULL if value is not string type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT const char*
purc_variant_get_atom_string_const(purc_variant_t value);

/**
 * Creates a variant value of byte sequence type by coping the content.
 *
 * @param bytes: the pointer of a byte sequence.
 * @param nr_bytes: the number of bytes in sequence.
 *
 * Returns: A purc_variant_t with byte sequence type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_byte_sequence(const void* bytes, size_t nr_bytes);

/**
 * Creates a variant value of byte sequence type from a static C byte array.
 *
 * @param bytes: the pointer of a byte sequence.
 * @param nr_bytes: the number of bytes in sequence.
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.2
 */
PCA_EXPORT purc_variant_t
purc_variant_make_byte_sequence_static(const void* bytes, size_t nr_bytes);

/**
 * Creates a variant value of byte sequence type by reusing the bytes buffer.
 * The buffer will be released by calling free() when the variant is destroyed.
 *
 * @param bytes: the pointer of a byte sequence.
 * @param nr_bytes: the number of bytes in sequence.
 * @param sz_buff: the size of the bytes buffer.
 *
 * Returns: A purc_variant_t with string type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.2
 */
PCA_EXPORT purc_variant_t
purc_variant_make_byte_sequence_reuse_buff(void* bytes, size_t nr_bytes,
        size_t sz_buff);

/**
 * Gets the pointer of byte array which is encapsulated in byte sequence type.
 *
 * @param value: the data of byte sequence type
 * @param nr_bytes: the buffer to receive the size of byte sequence
 *
 * Returns: the pointer of byte array on success, or NULL on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT const unsigned char*
purc_variant_get_bytes_const(purc_variant_t value, size_t* nr_bytes);

/**
 * Get the number of bytes in a byte sequence variant value.
 *
 * @param sequence: the variant value of sequence type
 * @param length: the buffer receiving the length in bytes of the byte
 *  sequence.
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_sequence_bytes(purc_variant_t sequence, size_t *length);

/**
 * Get the number of bytes in a byte sequence variant value.
 *
 * @param sequence: the variant value of sequence type
 *
 * Returns: The number of bytes in an sequence variant value;
 *  \PURC_VARIANT_BADSIZE (-1) if the variant is not a byte sequence.
 *
 * Note: This function is deprecated, use \purc_variant_sequence_bytes
 *  instead.
 *
 * Since: 0.0.1
 */
static inline size_t purc_variant_sequence_length(purc_variant_t sequence)
{
    size_t len;
    if (!purc_variant_sequence_bytes(sequence, &len))
        return PURC_VARIANT_BADSIZE;
    return len;
}

typedef purc_variant_t (*purc_dvariant_method) (purc_variant_t root,
        size_t nr_args, purc_variant_t * argv);

/**
 * Creates dynamic value by setter and getter functions
 *
 * @param getter: the getter funciton pointer
 * @param setter: the setter function pointer
 *
 * Returns: A purc_variant_t with dynamic value,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_dynamic(purc_dvariant_method getter,
        purc_dvariant_method setter);


/**
 * Get the getter function from a dynamic value
 *
 * @param dynamic: the variant value of dynamic type
 *
 * Returns: A purc_dvariant_method funciton pointer
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_dvariant_method
purc_variant_dynamic_get_getter(purc_variant_t dynamic);


/**
 * Get the setter function from a dynamic value
 *
 * @param dynamic: the variant value of dynamic type
 *
 * Returns: A purc_dvariant_method funciton pointer
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_dvariant_method
purc_variant_dynamic_get_setter(purc_variant_t dynamic);


typedef purc_variant_t (*purc_nvariant_method) (void* native_entity,
            size_t nr_args, purc_variant_t* argv);

struct purc_native_ops {
    // query the getter for a specific property.
    purc_nvariant_method (*property_getter) (const char* key_name);

    // query the setter for a specific property.
    purc_nvariant_method (*property_setter) (const char* key_name);

    // query the eraser for a specific property.
    purc_nvariant_method (*property_eraser) (const char* key_name);

    // query the cleaner for a specific property.
    purc_nvariant_method (*property_cleaner) (const char* key_name);

    // the cleaner to clear the content of the native entity.
    bool (*cleaner) (void* native_entity);

    // the eraser to erase the native entity.
    bool (*eraser) (void* native_entity);

    // the callback when the variant was observed (nullable).
    bool (*observe) (void* native_entity, ...);
};

/**
 * Creates a variant value of native type.
 *
 * @param entity: the pointer to the native entity.
 * @param ops: the pointer to the ops for the native entity.
 *
 * Returns: A purc_variant_t with native value,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.2
 */
purc_variant_t purc_variant_make_native (void *native_entity,
    const struct purc_native_ops *ops);



/**
 * Get the native pointer of native variant value
 *
 * @param native: the variant value of native type
 *
 * Returns: the native pointer
 *
 * Since: 0.0.1
 */
PCA_EXPORT void *
purc_variant_native_get_entity(purc_variant_t native);


/**
 * Creates a variant value of array type.
 *
 * @param sz: the size of array
 * @param value0 ..... valuen: enumerates every elements in array
 *
 * Returns: A purc_variant_t with array type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_array(size_t sz, purc_variant_t value0, ...);


/**
 * Appends a variant value to the tail of an array.
 *
 * @param array: the variant value of array type
 * @param value: the element to be appended
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_append(purc_variant_t array, purc_variant_t value);


/**
 * Insert a variant value to the head of an array.
 *
 * @param array: the variant value of array type
 * @param value: the element to be insert
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_prepend(purc_variant_t array, purc_variant_t value);


/**
 * Gets an element from an array by index.
 *
 * @param array: the variant value of array type
 * @param idx: the index of wanted element
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_array_get(purc_variant_t array, int idx);


/**
 * Sets an element value in an array by index.
 *
 * @param array: the variant value of array type
 * @param idx: the index of replaced element
 * @param value: the element to replace
 *
 * Returns: True on success, otherwise False.
 *
 * Note: If idx is greater than max index of array, return -1.
 *       Whether free the replaced element, depends on its ref.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_set(purc_variant_t array, int idx, purc_variant_t value);


/**
 * Remove an element from an array by index.
 *
 * @param array: the variant value of array type
 * @param idx: the index of element to be removed
 *
 * Returns: True on success, otherwise False.
 *
 * Note: If idx is greater than max index of array, return -1.
 *       Whether free the removed element, depends on its ref.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool purc_variant_array_remove(purc_variant_t array, int idx);


/**
 * Inserts an element to an array, places it before an indicated element.
 *
 * @param array: the variant value of array type
 * @param idx: the index of element before which the new element will be placed
 *
 * @param value: the inserted element
 *
 * Returns: True on success, otherwise False.
 *
 * Note: If idx is greater than max index of array, return -1.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_insert_before(purc_variant_t array,
        int idx, purc_variant_t value);


/**
 * Inserts an element to an array, places it after an indicated element.
 *
 * @param array: the variant value of array type
 * @param idx: the index of element after which the new element will be placed
 * @param value: the inserted element
 *
 * Returns: True on success, otherwise False.
 *
 * Note: If idx is greater than sum of one plus max index of array, return -1.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_insert_after(purc_variant_t array,
        int idx, purc_variant_t value);

/**
 * Get the number of elements in an array variant value.
 *
 * @param array: the variant value of array type
 * @param sz: the buffer receiving the number of elements of the array.
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_array_size(purc_variant_t array, size_t *sz);

/**
 * Get the number of elements in an array variant value.
 *
 * @param array: the variant value of array type
 *
 * Returns: The number of elements in the array;
 *  \PURC_VARIANT_BADSIZE (-1) if the variant is not an array.
 *
 * Note: This function is deprecated, use \purc_variant_array_size instead.
 *
 * Since: 0.0.1
 */
static inline size_t purc_variant_array_get_size(purc_variant_t array)
{
    size_t sz;
    if (!purc_variant_array_size(array, &sz))
        return PURC_VARIANT_BADSIZE;
    return sz;
}

/**
 * Creates a variant value of object type with key as c string
 *
 * @param nr_kv_pairs: the minimum of key-value pairs
 * @param key0 ..... keyn: the keys of key-value pairs
 * @param value0 ..... valuen: the values of key-value pairs
 *
 * Returns: A purc_variant_t with object type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_object_by_static_ckey(size_t nr_kv_pairs,
        const char* key0, purc_variant_t value0, ...);

/**
 * Creates a variant value of object type with key as another variant
 *
 * @param nr_kv_pairs: the minimum of key-value pairs
 * @param key0 ..... keyn: the keys of key-value pairs
 * @param value0 ..... valuen: the values of key-value pairs
 *
 * Returns: A purc_variant_t with object type,
 *      or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_object(size_t nr_kv_pairs,
        purc_variant_t key0, purc_variant_t value0, ...);

/**
 * Gets the value by key from an object with key as another variant
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_object_get(purc_variant_t obj, purc_variant_t key);

/**
 * Gets the value by key from an object with key as c string
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
static inline purc_variant_t
purc_variant_object_get_by_ckey(purc_variant_t obj, const char* key)
{
    purc_variant_t k = purc_variant_make_string_static(key, true);
    if (k==PURC_VARIANT_INVALID) {
        return PURC_VARIANT_INVALID;
    }
    purc_variant_t v = purc_variant_object_get(obj, k);
    purc_variant_unref(k);
    return v;
}


/**
 * Sets the value by key in an object with key as another variant
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 * @param value: the value of key-value pair
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_object_set(purc_variant_t obj,
        purc_variant_t key, purc_variant_t value);

/**
 * Sets the value by key in an object with key as c string
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 * @param value: the value of key-value pair
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
static inline bool
purc_variant_object_set_by_static_ckey(purc_variant_t obj, const char* key,
        purc_variant_t value)
{
    purc_variant_t k = purc_variant_make_string_static(key, true);
    if (k==PURC_VARIANT_INVALID) {
        return false;
    }
    bool b = purc_variant_object_set(obj, k, value);
    purc_variant_unref(k);
    return b;
}

/**
 * Remove a key-value pair from an object by key with key as another variant
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_object_remove(purc_variant_t obj, purc_variant_t key);

/**
 * Remove a key-value pair from an object by key with key as c string
 *
 * @param obj: the variant value of obj type
 * @param key: the key of key-value pair
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
static inline bool
purc_variant_object_remove_by_static_ckey(purc_variant_t obj, const char* key)
{
    purc_variant_t k = purc_variant_make_string_static(key, true);
    if (k==PURC_VARIANT_INVALID) {
        return false;
    }
    bool b = purc_variant_object_remove(obj, k);
    purc_variant_unref(k);
    return b;
}

/**
 * Get the number of key-value pairs in an object variant value.
 *
 * @param obj: the variant value of object type
 * @param sz: the buffer receiving the number of the key-value pairs
 *  in the object
 *
 * Returns: True on success, otherwise False if the variant is not an object.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_object_size(purc_variant_t obj, size_t *sz);

/**
 * Get the number of key-value pairs in an object variant value.
 *
 * @param obj: the variant value of object type
 *
 * Returns: The number of the key-value pairs in the object;
 *  \PURC_VARIANT_BADSIZE (-1) if the variant is not an object.
 *
 * Note: This function is deprecated, use \purc_variant_object_size instead.
 *
 * Since: 0.0.1
 */
static inline size_t purc_variant_object_get_size(purc_variant_t obj)
{
    size_t sz;
    if (!purc_variant_object_size(obj, &sz))
        return PURC_VARIANT_BADSIZE;
    return sz;
}

/**
 * object iterator usage example:
 *
 * purc_variant_t obj;
 * ...
 * purc_variant_object_iterator* it;
 * it = purc_variant_object_make_iterator_begin(obj);
 * while (it) {
 *     const char     *key = purc_variant_object_iterator_get_key(it);
 *     purc_variant_t  val = purc_variant_object_iterator_get_value(it);
 *     ...
 *     bool having = purc_variant_object_iterator_next(it);
 *     // behavior of accessing `val`/`key` is un-defined
 *     if (!having) {
 *         // behavior of accessing `it` is un-defined
 *         break;
 *     }
 * }
 * if (it)
 *     purc_variant_object_release_iterator(it);
 */

struct purc_variant_object_iterator;

/**
 * Get the begin-iterator of the object,
 * which points to the head key-val-pair of the object
 *
 * @param object: the variant value of object type
 *
 * Returns: the begin-iterator of the object.
 *          NULL if no key-val-pair in the object
 *          returned iterator will inc object's ref for iterator's lifetime
 *          returned iterator shall also inc the pointed key-val-pair's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT struct purc_variant_object_iterator*
purc_variant_object_make_iterator_begin(purc_variant_t object);

/**
 * Get the end-iterator of the object,
 * which points to the tail key-val-pair of the object
 *
 * @param object: the variant value of object type
 *
 * Returns: the end-iterator of the object
 *          NULL if no key-val-pair in the object
 *          returned iterator will hold object's ref for iterator's lifetime
 *          returned iterator shall also hold the pointed key-val-pair's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT struct purc_variant_object_iterator*
purc_variant_object_make_iterator_end(purc_variant_t object);

/**
 * Release the object's iterator
 *
 * @param it: iterator of itself
 *
 * Returns: void
 *          both object's ref and the pointed key-val-pair's ref shall be dec`d
 *
 * Since: 0.0.1
 */
PCA_EXPORT void
purc_variant_object_release_iterator(struct purc_variant_object_iterator* it);

/**
 * Make the iterator point to it's successor,
 * or the next key-val-pair of the bounded object value
 *
 * @param it: iterator of itself
 *
 * Returns: True if iterator `it` has no following key-val-pair,
 *          False otherwise
 *          dec original key-val-pair's ref
 *          inc current key-val-pair's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_object_iterator_next(struct purc_variant_object_iterator* it);

/**
 * Make the iterator point to it's predecessor,
 * or the previous key-val-pair of the bounded object value
 *
 * @param it: iterator of itself
 *
 * Returns: True if iterator `it` has no leading key-val-pair, False otherwise
 *          dec original key-val-pair's ref
 *          inc current key-val-pair's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_object_iterator_prev(struct purc_variant_object_iterator* it);

/**
 * Get the key of key-val-pair that the iterator points to
 *
 * @param it: iterator of itself
 *
 * Returns: the key of key-val-pair
 *
 * Since: 0.0.2
 */
PCA_EXPORT purc_variant_t
purc_variant_object_iterator_get_key(struct purc_variant_object_iterator* it);

/**
 * Get the key of key-val-pair that the iterator points to
 *
 * @param it: iterator of itself
 *
 * Returns: the key of key-val-pair, not duplicated
 *
 * Since: 0.0.2
 */
static inline const char*
purc_variant_object_iterator_get_ckey(struct purc_variant_object_iterator* it)
{
    purc_variant_t k = purc_variant_object_iterator_get_key(it);
    return purc_variant_get_string_const(k);
}

/**
 * Get the value of key-val-pair that the iterator points to
 *
 * @param it: iterator of itself
 *
 * Returns: the value of key-val-pair, not duplicated
 *          the returned value's ref remains unchanged
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_object_iterator_get_value(
        struct purc_variant_object_iterator* it);

/**
 * Creates a variant value of set type.
 *
 * @param sz: the initial number of elements in a set.
 * @param unique_key: the unique keys specified in a C string (nullable).
 *      If the unique keyis NULL, the set is a generic one.
 *
 * @param value0 ..... valuen: the values.
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Note: The key is legal, only when the value is object type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_set_by_ckey(size_t sz, const char* unique_key,
        purc_variant_t value0, ...);

/**
 * Creates a variant value of set type.
 *
 * @param sz: the initial number of elements in a set.
 * @param unique_key: the unique keys specified in a variant. If the unique key
 *      is PURC_VARIANT_INVALID, the set is a generic one.
 * @param value0 ... valuen: the values will be add to the set.
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Note: The key is legal, only when the value is object type.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_set(size_t sz, purc_variant_t unique_key,
        purc_variant_t value0, ...);

/**
 * Adds a variant value to a set.
 *
 * @param set: the variant value of the set type.
 * @param value: the value to be added.
 * Returns: @true on success, @false if:
 *      - there is already such a value in the set.
 *      - the value is not an object if the set is managed by unique keys.
 * @param override: If the set is managed by unique keys and @overwrite is
 *  true, the function will override the old value which is equal to
 *  the new value under the unique keys, and return true. otherwise,
 *  it returns false.
 *
 * @note If the new value has not a property (a key-value pair) under
 *  a specific unique key, the value of the key should be treated
 *  as `undefined`.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_set_add(purc_variant_t obj, purc_variant_t value, bool overwrite);

/**
 * Remove a variant value from a set.
 *
 * @param set: the set to be operated
 * @param value: the value to be removed
 *
 * Returns: @true on success, @false if:
 *      - no any matching member in the set.
 *
 * @note This function works if the set is not managed by unique keys, or
 *  there is only one unique key. If there are multiple unique keys,
 *  use @purc_variant_set_remove_member_by_key_values() instead.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_set_remove(purc_variant_t obj, purc_variant_t value);

/**
 * Gets the member by the values of unique keys from a set.
 *
 * @param set: the variant value of the set type.
 * @param v1...vN: the values for matching. The caller should pass one value
 *      for each unique key. The number of the matching values must match
 *      the number of the unique keys.
 *
 * Returns: The memeber matched on success, or PURC_VARIANT_INVALID if:
 *      - the set does not managed by the unique keys, or
 *      - no any matching member.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_set_get_member_by_key_values(purc_variant_t set,
        purc_variant_t v1, ...);

/**
 * Removes the member by the values of unique keys from a set.
 *
 * @param set: the variant value of the set type. The set should be managed
 *      by unique keys.
 * @param v1...vN: the values for matching. The caller should pass one value
 *      for each unique key. The number of the matching values must match
 *      the number of the unique keys.
 *
 * Returns: @true on success, or @false if:
 *      - the set does not managed by unique keys, or
 *      - no any matching member.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_set_remove_member_by_key_values(purc_variant_t set,
        purc_variant_t v1, ...);

/**
 * Get the number of elements in a set variant value.
 *
 * @param set: the variant value of set type
 * @param sz: the variant value of set type
 *
 * Returns: True on success, otherwise False if the variant is not a set.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool purc_variant_set_size(purc_variant_t set, size_t *sz);

/**
 * Get the number of elements in a set variant value.
 *
 * @param set: the variant value of set type
 *
 * Returns: The number of elements in a set variant value;
 *  \PURC_VARIANT_BADSIZE (-1) if the variant is not a set.
 *
 * Note: This function is deprecated, use \purc_variant_set_size instead.
 *
 * Since: 0.0.1
 */
static inline size_t purc_variant_set_get_size(purc_variant_t set)
{
    size_t sz;
    if (!purc_variant_set_size(set, &sz))
        return PURC_VARIANT_BADSIZE;
    return sz;
}

/**
 * Get an element from set by index.
 *
 * @param array: the variant value of set type
 * @param idx: the index of wanted element
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_set_get_by_index(purc_variant_t set, int idx);

/**
 * Remove the element in set by index and return
 *
 * @param array: the variant value of set type
 * @param idx: the index of the element to be removed
 *
 * Returns: the variant removed at the index or PURC_VARIANT_INVALID if failed
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_set_remove_by_index(purc_variant_t set, int idx);

/**
 * Set an element in set by index.
 *
 * @param array: the variant value of set type
 * @param idx: the index of the element to be replaced
 * @val: the val that's to be set in the set
 *
 * Returns: A boolean that indicates if it succeeds or not
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_set_set_by_index(purc_variant_t set, int idx, purc_variant_t val);

/**
 * set iterator usage example:
 *
 * purc_variant_t obj;
 * ...
 * purc_variant_set_iterator* it = purc_variant_set_make_iterator_begin(obj);
 * while (it) {
 *     purc_variant_t  val = purc_variant_set_iterator_get_value(it);
 *     ...
 *     bool having = purc_variant_set_iterator_next(it);
 *     // behavior of accessing `val`/`key` is un-defined
 *     if (!having) {
 *         // behavior of accessing `it` is un-defined
 *         break;
 *     }
 * }
 * if (it)
 *     purc_variant_set_release_iterator(it);
 */

struct purc_variant_set_iterator;

/**
 * Get the begin-iterator of the set,
 * which points to the head element of the set
 *
 * @param set: the variant value of set type
 * 
 * Returns: the begin-iterator of the set.
 *          NULL if no element in the set
 *          returned iterator will inc set's ref for iterator's lifetime
 *          returned iterator shall also inc the pointed element's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT struct purc_variant_set_iterator*
purc_variant_set_make_iterator_begin(purc_variant_t set);

/**
 * Get the end-iterator of the set,
 * which points to the head element of the set
 *
 * @param set: the variant value of set type
 *
 * Returns: the end-iterator of the set.
 *          NULL if no element in the set
 *          returned iterator will inc set's ref for iterator's lifetime
 *          returned iterator shall also inc the pointed element's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT struct purc_variant_set_iterator*
purc_variant_set_make_iterator_end(purc_variant_t set);

/**
 * Release the set's iterator
 *
 * @param it: iterator of itself
 *
 * Returns: void
 *          both set's ref and the pointed element's ref shall be dec`d
 *
 * Since: 0.0.1
 */
PCA_EXPORT void
purc_variant_set_release_iterator(struct purc_variant_set_iterator* it);

/**
 * Make the set's iterator point to it's successor,
 * or the next element of the bounded set
 *
 * @param it: iterator of itself
 *
 * Returns: True if iterator `it` has no following element, False otherwise
 *          dec original element's ref
 *          inc current element's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_set_iterator_next(struct purc_variant_set_iterator* it);

/**
 * Make the set's iterator point to it's predecessor,
 * or the prev element of the bounded set
 *
 * @param it: iterator of itself
 *
 * Returns: True if iterator `it` has no leading element, False otherwise
 *          dec original element's ref
 *          inc current element's ref
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_set_iterator_prev(struct purc_variant_set_iterator* it);

/**
 * Get the value of the element that the iterator points to
 *
 * @param it: iterator of itself
 *
 * Returns: the value of the element
 *          the returned value's ref remains unchanged
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_set_iterator_get_value(struct purc_variant_set_iterator* it);


/**
 * Creates a variant value from a string which contents Json data
 *
 * @param json: the pointer of string which contents json data
 *
 * @param sz: the size of string
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_make_from_json_string(const char* json, size_t sz);

/**
 * Creates a variant value from Json file
 *
 * @param file: the Json file name
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_load_from_json_file(const char* file);


/**
 * Creates a variant value from stream
 *
 * @param stream: the stream of purc_rwstream_t type
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
 *
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_load_from_json_stream(purc_rwstream_t stream);

/**
 * Trys to cast a variant value to a long integer.
 *
 * @param v: the variant value.
 * @param i64: the buffer to receive the casted long integer if success.
 * @param parse_str: a boolean indicates whether to parse a string for
 *      casting.
 *
 * Returns: @TRUE on success, or @FALSE on failure (the variant value can not
 *      be casted to a long integer).
 *
 * Note: A null, a boolean, a number, a long integer,
 *      an unsigned long integer, or a long double can always be casted to
 *      a long integer. If 
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_cast_to_longint(purc_variant_t v, int64_t *i64, bool parse_str);

/**
 * Trys to cast a variant value to a unsigned long integer.
 *
 * @param v: the variant value.
 * @param u64: the buffer to receive the casted unsigned long integer
 *      if success.
 * @param parse_str: a boolean indicates whether to parse a string for
 *      casting.
 *
 * Returns: @TRUE on success, or @FALSE on failure (the variant value can not
 *      be casted to an unsigned long integer).
 *
 * Note: A null, a boolean, a number, a long integer,
 *      an unsigned long integer, or a long double can always be casted to
 *      an unsigned long integer.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_cast_to_ulongint(purc_variant_t v, uint64_t *u64, bool parse_str);

/**
 * Trys to cast a variant value to a nubmer.
 *
 * @param v: the variant value.
 * @param d: the buffer to receive the casted number if success.
 * @param parse_str: a boolean indicates whether to parse a string for
 *      casting.
 *
 * Returns: @TRUE on success, or @FALSE on failure (the variant value can not
 *      be casted to a number).
 *
 * Note: A null, a boolean, a number, a long integer,
 *      an unsigned long integer, or a long double can always be casted to
 *      a number (double float number).
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_cast_to_number(purc_variant_t v, double *d, bool parse_str);

/**
 * Trys to cast a variant value to a long double float number.
 *
 * @param v: the variant value.
 * @param ld: the buffer to receive the casted long double if success.
 * @param parse_str: a boolean indicates whether to parse a string for
 *      casting.
 *
 * Returns: @TRUE on success, or @FALSE on failure (the variant value can not
 *      be casted to a long double).
 *
 * Note: A null, a boolean, a number, a long integer,
 *      an unsigned long integer, or a long double can always be casted to
 *      a long double float number.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_cast_to_long_double(purc_variant_t v, long double *ld,
        bool parse_str);

/**
 * Trys to cast a variant value to a byte sequence.
 *
 * @param v: the variant value.
 * @param bytes: the buffer to receive the pointer to the byte sequence.
 * @param sz: the buffer to receive the size of the byte sequence in bytes.
 *
 * Returns: @TRUE on success, or @FALSE on failure (the variant value can not
 *      be casted to a byte sequence).
 *
 * Note: Only a string, an atom string, or a byte sequence can be casted to
 *      a byte sequence.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool
purc_variant_cast_to_byte_sequence(purc_variant_t v,
        const void **bytes, size_t *sz);


/**
 * flagfor the purc_variant_compare() function.
 */
#define PCVARIANT_COMPARE_OPT_AUTO           0x0000
#define PCVARIANT_COMPARE_OPT_NUMBER         0x0001
#define PCVARIANT_COMPARE_OPT_CASE           0x0002
#define PCVARIANT_COMPARE_OPT_CASELESS       0x0003

/**
 * Compares two variant value
 *
 * @param v1: one of compared variant value
 * @param v2: the other variant value to be compared
 * @param flags: comparation flags
 *
 * Returns: The function returns an integer less than, equal to, or greater
 *      than zero if v1 is found, respectively, to be less than, to match,
 *      or be greater than v2.
 *
 * Since: 0.0.1
 */
PCA_EXPORT double
purc_variant_compare_ex(purc_variant_t v1, purc_variant_t v2, unsigned int flag);

PCA_EXPORT int
purc_variant_compare(purc_variant_t v1, purc_variant_t v2);
/**
 * A flag for the purc_variant_serialize() function which causes the output
 * to have no extra whitespace or formatting applied.
 */
#define PCVARIANT_SERIALIZE_OPT_PLAIN           0x0000

/**
 * A flag for the purc_variant_serialize() function which causes the output to
 * have minimal whitespace inserted to make things slightly more readable.
 */
#define PCVARIANT_SERIALIZE_OPT_SPACED          0x0001

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to be formatted.
 *
 * See the "Two Space Tab" option at http://jsonformatter.curiousconcept.com/
 * for an example of the format.
 */
#define PCVARIANT_SERIALIZE_OPT_PRETTY          0x0002

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to drop trailing zero for float values.
 */
#define PCVARIANT_SERIALIZE_OPT_NOZERO          0x0004

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to be formatted.
 *
 * Instead of a "Two Space Tab" this gives a single tab character.
 */
#define PCVARIANT_SERIALIZE_OPT_PRETTY_TAB      0x0010

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to not escape the forward slashes.
 */
#define PCVARIANT_SERIALIZE_OPT_NOSLASHESCAPE   0x0020

#define PCVARIANT_SERIALIZE_OPT_BSEQUECE_MASK   0x0F00
/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to use hexadecimal characters for byte sequence.
 */
#define PCVARIANT_SERIALIZE_OPT_BSEQUECE_HEX    0x0100

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to use binary characters for byte sequence.
 */
#define PCVARIANT_SERIALIZE_OPT_BSEQUECE_BIN    0x0200

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to use BASE64 encoding for byte sequence.
 */
#define PCVARIANT_SERIALIZE_OPT_BSEQUECE_BASE64 0x0300

/**
 * A flag for the purc_variant_serialize() function which causes
 * the output to have dot for binary sequence.
 */
#define PCVARIANT_SERIALIZE_OPT_BSEQUENCE_BIN_DOT       0x0040

/**
 * A flag for the purc_variant_serialize() function which causes
 * the function ignores the output errors.
 */
#define PCVARIANT_SERIALIZE_OPT_IGNORE_ERRORS           0x0080

/**
 * Serialize a variant value
 *
 * @param value: the variant value to be serialized.
 * @param stream: the stream to which the serialized data write.
 * @param indent_level: the initial indent level. 0 for most cases.
 * @param flags: the serialization flags.
 * @param len_expected: The buffer to receive the expected length of
 *      the serialized data (nullable). The value in the buffer should be
 *      set to 0 initially.
 *
 * Returns:
 * The size of the serialized data written to the stream;
 * On error, -1 is returned, and error code is set to indicate
 * the cause of the error.
 *
 * If the function is called with the flag
 * PCVARIANT_SERIALIZE_OPT_IGNORE_ERRORS set, this function always
 * returned the number of bytes written to the stream actually.
 * Meanwhile, if @len_expected is not null, the expected length of
 * the serialized data will be returned through this buffer.
 *
 * Therefore, you can prepare a small memory stream with the flag
 * PCVARIANT_SERIALIZE_OPT_IGNORE_ERRORS set to count the
 * expected length of the serialized data.
 *
 * Since: 0.0.1
 */
PCA_EXPORT ssize_t
purc_variant_serialize(purc_variant_t value, purc_rwstream_t stream,
        int indent_level, unsigned int flags, size_t *len_expected);


/**
 * Loads a variant value from an indicated library
 *
 * @param so_name: the library name
 *
 * @param var_name: the variant value name
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
.*
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_dynamic_value_load_from_so(const char* so_name,
        const char* var_name);

/**
 * Loads a variant value from an indicated library
 *
 * @param so_name: the library name
 *
 * @param var_name: the variant value name
 *
 * @param ver_code: version number
 *
 * Returns: A purc_variant_t on success, or PURC_VARIANT_INVALID on failure.
.*
 * Since: 0.0.1
 */
PCA_EXPORT purc_variant_t
purc_variant_load_dvobj_from_so (const char *so_name,
        const char *dvobj_name);

/**
 * Unloads a dynamic library
 *
 * @value: dynamic object
 *
 * Returns: True for success, false on failure.
.*
 * Since: 0.0.1
 */
PCA_EXPORT bool purc_variant_unload_dvobj (purc_variant_t dvobj);

typedef enum purc_variant_type
{
    PURC_VARIANT_TYPE_FIRST = 0,

    /* XXX: keep consistency with type names */
    PURC_VARIANT_TYPE_UNDEFINED = PURC_VARIANT_TYPE_FIRST,
    PURC_VARIANT_TYPE_NULL,
    PURC_VARIANT_TYPE_BOOLEAN,
    PURC_VARIANT_TYPE_NUMBER,
    PURC_VARIANT_TYPE_LONGINT,
    PURC_VARIANT_TYPE_ULONGINT,
    PURC_VARIANT_TYPE_LONGDOUBLE,
    PURC_VARIANT_TYPE_ATOMSTRING,
    PURC_VARIANT_TYPE_STRING,
    PURC_VARIANT_TYPE_BSEQUENCE,
    PURC_VARIANT_TYPE_DYNAMIC,
    PURC_VARIANT_TYPE_NATIVE,
    PURC_VARIANT_TYPE_OBJECT,
    PURC_VARIANT_TYPE_ARRAY,
    PURC_VARIANT_TYPE_SET,

    /* XXX: change this if you append a new type. */
    PURC_VARIANT_TYPE_LAST = PURC_VARIANT_TYPE_SET,
} purc_variant_type;

#define PURC_VARIANT_TYPE_NR \
    (PURC_VARIANT_TYPE_LAST - PURC_VARIANT_TYPE_FIRST + 1)

/**
 * Whether the vairant is indicated type.
 *
 * @param value: the variant value
 * @param type: wanted type
 *
 * Returns: True on success, otherwise False.
 *
 * Since: 0.0.1
 */
PCA_EXPORT bool purc_variant_is_type(purc_variant_t value,
        enum purc_variant_type type);

/**
 * Get the type of a vairant value.
 *
 * @param value: the variant value
 *
 * Returns: The type of input variant value
 *
 * Since: 0.0.1
 */
PCA_EXPORT enum purc_variant_type purc_variant_get_type(purc_variant_t value);


/**
 * Whether the value is of indicated type.
 *
 * @param v: the variant value
 *
 * Returns: True if Yes, otherwise False.
 *
 * Since: 0.0.1
 */
static inline bool purc_variant_is_boolean(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_BOOLEAN);
}

static inline bool purc_variant_is_number(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_NUMBER);
}

static inline bool purc_variant_is_longint(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_LONGINT);
}

static inline bool purc_variant_is_ulongint(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_ULONGINT);
}

static inline bool purc_variant_is_longdouble(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_LONGDOUBLE);
}

static inline bool purc_variant_is_atomstring(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_ATOMSTRING);
}

static inline bool purc_variant_is_string(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_STRING);
}

static inline bool purc_variant_is_sequence(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_BSEQUENCE);
}

static inline bool purc_variant_is_dynamic(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_DYNAMIC);
}

static inline bool purc_variant_is_native(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_NATIVE);
}

static inline bool purc_variant_is_object(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_OBJECT);
}

static inline bool purc_variant_is_array(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_ARRAY);
}

static inline bool purc_variant_is_set(purc_variant_t v)
{
    return purc_variant_is_type(v, PURC_VARIANT_TYPE_SET);
}

struct purc_variant_stat {
    size_t nr_values[PURC_VARIANT_TYPE_NR];
    size_t sz_mem[PURC_VARIANT_TYPE_NR];
    size_t nr_total_values;
    size_t sz_total_mem;
    size_t nr_reserved;
    size_t nr_max_reserved;
};

/**
 * Statistic of variant status.
 *
 * Returns: The pointer to struct purc_variant_stat on success, otherwise NULL.
 *
 * Since: 0.0.1
 */
PCA_EXPORT struct purc_variant_stat*  purc_variant_usage_stat(void);

/**
 * Numberify a variant value to double
 *
 * @param value: variant value to be operated
 *
 * Returns: a double number that is numberified from the variant value
 *
 * Since: 0.0.3
 */
PCA_EXPORT double
purc_variant_numberify(purc_variant_t value);

/**
 * Booleanize a variant value to boolean
 *
 * @param value: variant value to be operated
 *
 * Returns: a boolean value that is booleanized from the variant value
 *
 * Since: 0.0.3
 */
PCA_EXPORT bool
purc_variant_booleanize(purc_variant_t value);

/**
 * Stringify a variant value to buffer
 *
 * @param buf: the pointer that points to the buffer to store result content
 *
 * @param len: the size of the above buffer
 *
 * @param value: variant value to be operated
 *
 * Returns: totol # of result content that has been succesfully written
 *          or shall be written if buffer is large enough
 *          or -1 in case of other failure
 *
 * Note: API is similar to `snprintf`
 *
 * Since: 0.0.3
 */
PCA_EXPORT int
purc_variant_stringify(char *buf, size_t len, purc_variant_t value);

/**
 * Stringify a variant value in the similar way as `asprintf` does
 *
 * @param strp: the newly allocated space would be stored in *strp
 *
 * @param value: variant value to be operated
 *
 * Returns: totol # of result content that has been succesfully written
 *          or shall be written if buffer is large enough
 *          or -1 in case of other failure, such of OOM
 *
 * Note: API is similar to `asprintf`
 *
 * Since: 0.0.3
 */
PCA_EXPORT int
purc_variant_stringify_alloc(char **strp, purc_variant_t value);

/**
 * Register a variant listener
 *
 * @param v: the variant that is to be observed
 *
 * @param name: the name of the observer
 *
 * @param handler: the callback that is to be called upon when the observed
 *                 event is fired
 * @param ctxt: the context belongs to the callback
 *
 * Returns: boolean that designates if the operation succeeds or not
 *
 * Since: 0.0.4
 */
PCA_EXPORT bool
purc_variant_register_listener(purc_variant_t v, purc_atom_t name,
        pcvar_msg_handler handler, void *ctxt);

/**
 * Revoke a variant listener
 *
 * @param v: the variant whose listener is to be revoked
 *
 * @param name: the name of the listener that is to be revoked
 *
 * Returns: boolean that designates if the operation succeeds or not
 *
 * Since: 0.0.4
 */
PCA_EXPORT bool
purc_variant_revoke_listener(purc_variant_t v, purc_atom_t name);


PCA_EXTERN_C_END

#endif /* not defined PURC_PURC_VARIANT_H */
