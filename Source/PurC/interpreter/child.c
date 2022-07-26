/*
 * @file child.c
 * @author XueShuming
 * @date 2022/07/25
 * @brief The operation for child coroutine
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

#include "purc.h"

#include "config.h"

#include "internal.h"

#include "private/var-mgr.h"
#include "private/errors.h"
#include "private/instance.h"
#include "private/utils.h"
#include "private/variant.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_RUNNER_NAME     "_self"

void
fill_cor_rdr_info(purc_renderer_extra_info *rdr_info, purc_variant_t rdr)
{
    purc_variant_t tmp;

    tmp = purc_variant_object_get_by_ckey(rdr, "class");
    if (tmp)
        rdr_info->klass = purc_variant_get_string_const(tmp);

    tmp = purc_variant_object_get_by_ckey(rdr, "title");
    if (tmp)
        rdr_info->title = purc_variant_get_string_const(tmp);

    tmp = purc_variant_object_get_by_ckey(rdr, "layoutStyle");
    if (tmp)
        rdr_info->layout_style = purc_variant_get_string_const(tmp);

    rdr_info->toolkit_style = purc_variant_object_get_by_ckey(rdr,
            "toolkitStyle");

    tmp = purc_variant_object_get_by_ckey(rdr, "pageGroups");
    if (tmp) {
        rdr_info->page_groups = purc_variant_get_string_const(tmp);
    }
}

purc_atom_t
pcintr_schedule_child_co(const char *hvml, purc_atom_t curator,
        const char *runner, const char *rdr_target, purc_variant_t request,
        bool create_runner)
{

    purc_atom_t cid = 0;
    purc_vdom_t vdom = NULL;
    char endpoint_name[PURC_LEN_ENDPOINT_NAME + 1];

    pcrdr_page_type page_type= PCRDR_PAGE_TYPE_NULL;
    char *target_workspace = NULL;
    char *target_group = NULL;
    char *page_name = NULL;
    const char *p = NULL;


    struct pcinst *inst = pcinst_current();
    const char *app_name = inst->app_name;
    const char *runner_name = runner;
    if (!runner || strcmp(runner, DEFAULT_RUNNER_NAME) == 0) {
        runner_name = inst->runner_name;
    }

    purc_assemble_endpoint_name_ex(PCRDR_LOCALHOST,
            app_name, runner_name,
            endpoint_name, sizeof(endpoint_name) - 1);
    purc_atom_t atom = purc_atom_try_string_ex(PURC_ATOM_BUCKET_DEF,
            endpoint_name);
    if (atom == 0 && !create_runner) {
        goto out;
    }

    vdom = purc_load_hvml_from_string(hvml);
    if (vdom == NULL) {
        goto out;
    }

    if (!rdr_target) {
        page_type = PCRDR_PAGE_TYPE_PLAINWIN;
    }
    else {
        p = strchr(rdr_target, ':');
        if (p) {
            size_t len = p - rdr_target;
            if (strncmp(rdr_target, PCRDR_PAGE_TYPE_NAME_WIDGET, len) == 0) {
                page_type = PCRDR_PAGE_TYPE_WIDGET;
            }
            else {
                page_type = PCRDR_PAGE_TYPE_PLAINWIN;
            }
            p = p + 1;
        }
        else {
            page_type = PCRDR_PAGE_TYPE_PLAINWIN;
            p = rdr_target;
        }

        const char *g = strchr(p, '@');
        if (g) {
            page_name = strndup(p, g - p);
            target_group = strdup(g + 1);
        }
        else {
            page_name = strdup(p);
        }
    }

    purc_atom_t dest_inst = purc_inst_create_or_get(app_name,
            runner_name, NULL, NULL);
    if (!dest_inst) {
        goto out_free_names;
    }

    purc_renderer_extra_info rdr_info = {};
    if (request && purc_variant_is_object(request)) {
        purc_variant_t rdr =
            purc_variant_object_get_by_ckey(request, "_renderer");
        if (purc_variant_is_object(rdr)) {
            fill_cor_rdr_info(&rdr_info, rdr);
        }
    }

    if (inst->intr_heap->move_buff != dest_inst) {
        cid = purc_inst_schedule_vdom(dest_inst, vdom,
                curator, request, page_type,
                "main", target_group, page_name,
                NULL, NULL);
    }
    else {
        purc_coroutine_t cco = purc_schedule_vdom(vdom,
                curator, request, page_type,
                "main", target_group, page_name,
                NULL, NULL, NULL);
        if (cco) {
            cid = cco->cid;
        }
    }

out_free_names:
    free(page_name);
    free(target_group);
    free(target_workspace);

out:
    return cid;
}

