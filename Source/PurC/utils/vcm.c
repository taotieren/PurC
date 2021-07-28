/*
 * @file vcm.c
 * @author XueShuming
 * @date 2021/07/28
 * @brief The API for vcm.
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "purc-utils.h"
#include "purc-errors.h"
#include "private/errors.h"
#include "private/vcm.h"

purc_variant_t pcvcm_node_to_variant (struct pcvcm_node* node);

purc_variant_t pcvcm_node_object_to_variant (struct pcvcm_node* node)
{
    struct pctree_node* tree_node = pcvcm_node_to_pctree_node (node);
    purc_variant_t object = purc_variant_make_object (0,
            PURC_VARIANT_INVALID, PURC_VARIANT_INVALID);

    struct pctree_node* k_node = tree_node->first_child;
    struct pctree_node* v_node = k_node->next;
    while (k_node && v_node) {
        purc_variant_t key = pcvcm_node_to_variant (
                pcvcm_node_from_pctree_node(k_node));
        purc_variant_t value = pcvcm_node_to_variant (
                pcvcm_node_from_pctree_node(v_node));

        purc_variant_object_set (object, key, value);

        k_node = v_node->next;
        v_node = k_node ? k_node->next : NULL;
    }

    return object;
}

purc_variant_t pcvcm_node_array_to_variant (struct pcvcm_node* node)
{
    struct pctree_node* tree_node = pcvcm_node_to_pctree_node (node);
    purc_variant_t array = purc_variant_make_array (0, PURC_VARIANT_INVALID);

    struct pctree_node* array_node = tree_node->first_child;
    while (array_node) {
        purc_variant_t vt = pcvcm_node_to_variant (
                pcvcm_node_from_pctree_node(array_node));
        purc_variant_array_append (array, vt);

        array_node = array_node->next;
    }
    return array;
}

purc_variant_t pcvcm_node_to_variant (struct pcvcm_node* node)
{
    const char* buf = (const char*) node->buf;
    switch (node->type)
    {
        case PCVCM_NODE_TYPE_OBJECT:
            return pcvcm_node_object_to_variant (node);

        case PCVCM_NODE_TYPE_ARRAY:
            return pcvcm_node_array_to_variant (node);

        case PCVCM_NODE_TYPE_KEY:
        case PCVCM_NODE_TYPE_STRING:
            return purc_variant_make_string (buf, false);

        case PCVCM_NODE_TYPE_NULL:
            return purc_variant_make_null ();

        case PCVCM_NODE_TYPE_BOOLEAN:
            return purc_variant_make_boolean (strcmp ("true", buf) == 0);

        case PCVCM_NODE_TYPE_NUMBER:
            return purc_variant_make_number (atof(buf));

        case PCVCM_NODE_TYPE_LONG_INTEGER_NUMBER:
            return purc_variant_make_longint (atoll(buf));

        case PCVCM_NODE_TYPE_UNSIGNED_LONG_INTEGER_NUMBER:
            return purc_variant_make_ulongint (atoll(buf));

        case PCVCM_NODE_TYPE_LONG_DOUBLE_NUMBER:
            return purc_variant_make_longdouble (atof(buf));

        case PCVCM_NODE_TYPE_BYTE_SEQUENCE:
            return purc_variant_make_byte_sequence(buf, strlen(buf));
    }
    return purc_variant_make_null();
}

purc_variant_t pcvcm_eval (struct pcvcm_node* tree)
{
    if (!tree) {
        return purc_variant_make_null();
    }
    return pcvcm_node_to_variant (tree);
}
