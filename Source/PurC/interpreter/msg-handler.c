/**
 * @file msg-handler.c
 * @author Xue Shuming
 * @date 2022/07/01
 * @brief The message handler for instance
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
 *
 */

#include "config.h"

#include "internal.h"
#include "ops.h"
#include "private/instance.h"
#include "private/msg-queue.h"
#include "private/interpreter.h"
#include "private/regex.h"

#define PLOG(...) do {                                                        \
    FILE *fp = fopen("/tmp/plog.log", "a+");                                  \
    fprintf(fp, ##__VA_ARGS__);                                               \
    fclose(fp);                                                               \
} while (0)

static void
on_observer_matched(void *ud)
{
    struct pcintr_observer_matched_data *p;
    p = (struct pcintr_observer_matched_data*)ud;
    PC_ASSERT(p);

    pcintr_stack_t stack = pcintr_get_stack();
    PC_ASSERT(stack);
    pcintr_coroutine_t co = stack->co;

    PC_ASSERT(co->state == CO_STATE_RUN);
    co->state = CO_STATE_RUN;

    // FIXME:
    // push stack frame
    struct pcintr_stack_frame_normal *frame_normal;
    frame_normal = pcintr_push_stack_frame_normal(stack);
    PC_ASSERT(frame_normal);

    struct pcintr_stack_frame *frame;
    frame = &frame_normal->frame;

    frame->ops = pcintr_get_ops_by_element(p->pos);
    frame->scope = p->scope;
    frame->pos = p->pos;
    frame->silently = pcintr_is_element_silently(frame->pos) ? 1 : 0;
    frame->edom_element = p->edom_element;
    frame->next_step = NEXT_STEP_AFTER_PUSHED;

    if (p->payload) {
        pcintr_set_question_var(frame, p->payload);
        purc_variant_unref(p->payload);
    }

    pcintr_execute_one_step_for_ready_co(co);

    free(p);
}

void
observer_matched(struct pcintr_observer *p, purc_variant_t payload)
{
    pcintr_stack_t stack = pcintr_get_stack();
    PC_ASSERT(stack);
    pcintr_coroutine_t co = stack->co;
    PC_ASSERT(&co->stack == stack);

    struct pcintr_observer_matched_data *data;
    data = (struct pcintr_observer_matched_data*)calloc(1, sizeof(*data));
    PC_ASSERT(data);
    data->pos = p->pos;
    data->scope = p->scope;
    data->edom_element = p->edom_element;
    if (payload) {
        data->payload = payload;
        purc_variant_ref(data->payload);
    }

    pcintr_post_msg(data, on_observer_matched);
}

static void handle_vdom_event(pcintr_stack_t stack, purc_vdom_t vdom,
        purc_atom_t type, purc_variant_t sub_type, purc_variant_t data)
{
    UNUSED_PARAM(stack);
    UNUSED_PARAM(vdom);
    UNUSED_PARAM(sub_type);
    UNUSED_PARAM(data);
    if (pchvml_keyword(PCHVML_KEYWORD_ENUM(MSG, CLOSE)) == type) {
        // TODO : quit runner
        fprintf(stderr, "## event msg not handle : close\n");
    }
}

void
pcintr_handle_message(void *ctxt)
{
    pcintr_stack_t stack = pcintr_get_stack();
    PC_ASSERT(stack);
    pcintr_coroutine_t co = stack->co;
    PC_ASSERT(co);

    PC_ASSERT(co->state == CO_STATE_RUN);

    struct pcintr_stack_frame *frame;
    frame = pcintr_stack_get_bottom_frame(stack);
    PC_ASSERT(frame == NULL);

    struct pcintr_message* msg = (struct pcintr_message*) ctxt;
    PC_ASSERT(msg);

    PC_ASSERT(stack == msg->stack);

    const char *msg_type = purc_variant_get_string_const(msg->type);
    PC_ASSERT(msg_type);

    const char *sub_type = NULL;
    if (msg->sub_type != PURC_VARIANT_INVALID)
        sub_type = purc_variant_get_string_const(msg->sub_type);

    purc_atom_t msg_type_atom = purc_atom_try_string_ex(ATOM_BUCKET_MSG,
            msg_type);
    PC_ASSERT(msg_type_atom);

    purc_variant_t observed = msg->source;

    bool handle = false;
    struct list_head* list = pcintr_get_observer_list(stack, observed);
    struct pcintr_observer *p, *n;
    list_for_each_entry_safe(p, n, list, node) {
        if (pcintr_is_observer_match(p, observed, msg_type_atom, sub_type)) {
            handle = true;
            observer_matched(p, msg->extra);
        }
    }

    if (!handle && purc_variant_is_native(observed)) {
        void *dest = purc_variant_native_get_entity(observed);
        // window close event dispatch to vdom
        if (dest == stack->vdom) {
            handle_vdom_event(stack, stack->vdom, msg_type_atom,
                    msg->sub_type, msg->extra);
        }
    }

    pcintr_message_destroy(msg);

    PC_ASSERT(co->state == CO_STATE_RUN);
}



int
process_coroutine_event(pcintr_coroutine_t co, pcrdr_msg *msg)
{
    UNUSED_PARAM(co);
    UNUSED_PARAM(msg);
    //pcintr_stack_t stack = &co->stack;

    return 0;
}

int
dispatch_coroutine_msg(pcintr_coroutine_t co, pcrdr_msg *msg)
{
    if (!co || !msg) {
        return 0;
    }

    switch (msg->type) {
    case PCRDR_MSG_TYPE_EVENT:
        return process_coroutine_event(co, msg);

    case PCRDR_MSG_TYPE_VOID:
        PC_ASSERT(0);
        break;

    case PCRDR_MSG_TYPE_REQUEST:
        PC_ASSERT(0);
        break;

    case PCRDR_MSG_TYPE_RESPONSE:
        PC_ASSERT(0);
        break;

    default:
        // NOTE: shouldn't happen, no way to recover gracefully, fail-fast
        PC_ASSERT(0);
    }
    return 0;
}

int
dispatch_move_buffer_msg(struct pcinst *inst, pcrdr_msg *msg)
{
    UNUSED_PARAM(inst);
    struct pcintr_heap *heap = inst->intr_heap;
    if (!heap) {
        return 0;
    }

    switch (msg->type) {
    case PCRDR_MSG_TYPE_EVENT:
    {
        // add msg to coroutine message queue
        struct rb_root *coroutines = &heap->coroutines;
        struct rb_node *p, *n;
        struct rb_node *first = pcutils_rbtree_first(coroutines);
        pcutils_rbtree_for_each_safe(first, p, n) {
            pcintr_coroutine_t co = container_of(p, struct pcintr_coroutine,
                    node);
            if (co->ident == msg->targetValue) {
                return pcinst_msg_queue_append(co->mq, msg);
            }
        }
    }
        break;

    case PCRDR_MSG_TYPE_VOID:
        PC_ASSERT(0);
        break;

    case PCRDR_MSG_TYPE_REQUEST:
        PC_ASSERT(0);
        break;

    case PCRDR_MSG_TYPE_RESPONSE:
        PC_ASSERT(0);
        break;

    default:
        // NOTE: shouldn't happen, no way to recover gracefully, fail-fast
        PC_ASSERT(0);
    }
    return 0;
}


void
handle_move_buffer_msg(void)
{
    size_t n;
    int r = purc_inst_holding_messages_count(&n);
    PC_ASSERT(r == 0);
    if (n <= 0) {
        return;
    }

    struct pcinst *inst = pcinst_current();

    for (size_t i = 0; i < n; i++) {
        pcrdr_msg *msg = purc_inst_take_away_message(0);
        if (msg == NULL) {
            PC_ASSERT(purc_get_last_error() == 0);
            return;
        }

        // TODO: how to handle dispatch failed
        dispatch_move_buffer_msg(inst, msg);
        pcinst_put_message(msg);
    }
}


void
handle_coroutine_msg(pcintr_coroutine_t co)
{
    UNUSED_PARAM(co);
    if (co == NULL
            || co->state == CO_STATE_WAIT
            || co->state == CO_STATE_RUN) {
        return;
    }

    struct pcinst_msg_queue *queue = co->mq;
    pcrdr_msg *msg = pcinst_msg_queue_get_msg(queue);
    while (msg) {
        dispatch_coroutine_msg(co, msg);
        pcinst_put_message(msg);
    }
}

void
pcintr_dispatch_msg(void)
{
    // handle msg from move buffer
    handle_move_buffer_msg();

    // handle msg from message queue of the current co
    handle_coroutine_msg(pcintr_get_coroutine());
}

