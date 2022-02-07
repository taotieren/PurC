/*
 * @file hvml-rwswrap.c
 * @author XueShuming
 * @date 2021/09/05
 * @brief The impl of hvml rwswrap.
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
#include "hvml-rwswrap.h"

#if HAVE(GLIB)
#include <gmodule.h>
#else
#include <stdlib.h>
#endif

#if HAVE(GLIB)
#define    PCHVML_ALLOC(sz)   g_slice_alloc0(sz)
#define    PCHVML_FREE(p)     g_slice_free1(sizeof(*p), (gpointer)p)
#else
#define    PCHVML_ALLOC(sz)   calloc(1, sz)
#define    PCHVML_FREE(p)     free(p)
#endif

struct pchvml_rwswrap {
    purc_rwstream_t rws;
    struct list_head uc_list;
    struct list_head char_consumed;
    size_t nr_char_consumed;
};

struct pchvml_uc* pchvml_uc_new (void)
{
    return PCHVML_ALLOC(sizeof(struct pchvml_uc));
}

void pchvml_uc_destroy (struct pchvml_uc* uc)
{
    if (uc) {
        PCHVML_FREE(uc);
    }
}

struct pchvml_rwswrap* pchvml_rwswrap_new (void)
{
    struct pchvml_rwswrap* wrap = PCHVML_ALLOC(sizeof(struct pchvml_rwswrap));
    if (!wrap) {
        return NULL;
    }
    INIT_LIST_HEAD(&wrap->uc_list);
    return wrap;
}

void pchvml_rwswrap_set_rwstream (struct pchvml_rwswrap* wrap,
        purc_rwstream_t rws)
{
    wrap->rws = rws;
}

static uint32_t pchvml_rwswrap_read_from_rwstream (struct pchvml_rwswrap* wrap)
{
    char c[8] = {0};
    uint32_t uc = 0;
    int nr_c = purc_rwstream_read_utf8_char (wrap->rws, c, &uc);
    if (nr_c < 0) {
        uc = PCHVML_INVALID_CHARACTER;
    }
    return uc;
}

static uint32_t pchvml_rwswrap_read_from_uc_list (struct pchvml_rwswrap* wrap)
{
    struct pchvml_uc* puc = list_entry(wrap->uc_list.next,
            struct pchvml_uc, list);
    uint32_t uc = puc->uc;
    list_del_init(&puc->list);
    pchvml_uc_destroy(puc);
    return uc;
}

uint32_t pchvml_rwswrap_next_char (struct pchvml_rwswrap* wrap)
{
    if (list_empty (&wrap->uc_list)) {
        return pchvml_rwswrap_read_from_rwstream (wrap);
    }
    return pchvml_rwswrap_read_from_uc_list (wrap);
}

bool pchvml_rwswrap_buffer_chars (struct pchvml_rwswrap* wrap,
        uint32_t* ucs, size_t nr_ucs)
{
    for (int i = nr_ucs - 1; i >= 0; i--) {
        struct pchvml_uc* puc = pchvml_uc_new ();
        if (!puc) {
            pcinst_set_error(PURC_ERROR_OUT_OF_MEMORY);
            return false;
        }
        puc->uc = ucs[i];
        list_add(&puc->list, &wrap->uc_list);
    }
    return true;
}

bool pchvml_rwswrap_buffer_arrlist (struct pchvml_rwswrap* wrap,
        struct pcutils_arrlist* ucs)
{
    size_t length = pcutils_arrlist_length(ucs);
    for (int i = length - 1; i >= 0; i--) {
        uint32_t uc = (uint32_t)(uintptr_t) pcutils_arrlist_get_idx (ucs, i);
        struct pchvml_uc* puc = pchvml_uc_new ();
        if (!puc) {
            pcinst_set_error(PURC_ERROR_OUT_OF_MEMORY);
            return false;
        }
        puc->uc = uc;
        list_add(&puc->list, &wrap->uc_list);
    }
    return true;
}

void pchvml_rwswrap_destroy (struct pchvml_rwswrap* wrap)
{
    if (wrap) {
        struct list_head *p, *n;
        list_for_each_safe(p, n, &wrap->uc_list) {
            struct pchvml_uc* puc = list_entry(p, struct pchvml_uc, list);
            list_del_init(&puc->list);
            pchvml_uc_destroy(puc);
        }
        PCHVML_FREE(wrap);
    }
}
