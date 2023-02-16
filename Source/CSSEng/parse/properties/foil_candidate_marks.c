/*
 * This file is part of CSSEng.
 * Licensed under the MIT License,
 *          http://www.opensource.org/licenses/mit-license.php
 * Copyright (C) 2023 Beijing FMSoft Technologies Co., Ltd.
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse _foil_candidate_mark
 *
 * \param c      Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx      Pointer to vector iteration context
 * \param result  resulting style
 * \return CSS_OK on success,
 *       CSS_NOMEM on memory exhaustion,
 *       CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *           If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse__foil_candidate_marks_impl(css_language *c,
        const parserutils_vector *vector, int *ctx,
        css_style *result, int np)
{
    (void)np;
    int orig_ctx = *ctx;
    css_error error;
    const css_token *token;
    bool match;

    token = parserutils_vector_iterate(vector, ctx);
    if ((token == NULL) || ((token->type != CSS_TOKEN_IDENT) &&
                (token->type != CSS_TOKEN_STRING))) {
        *ctx = orig_ctx;
        return CSS_INVALID;
    }

    if (token->type == CSS_TOKEN_IDENT &&
            (lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match)
             == lwc_error_ok && match)) {
        error = css_stylesheet_style_inherit(result,
                CSS_PROP_FOIL_CANDIDATE_MARKS);
    }
    else if ((token->type == CSS_TOKEN_IDENT) &&
            (lwc_string_caseless_isequal(token->idata, c->strings[AUTO], &match)
             == lwc_error_ok && match)) {
        error = css__stylesheet_style_appendOPV(result,
                CSS_PROP_FOIL_CANDIDATE_MARKS, 0,
                FOIL_CANDIDATE_MARKS_AUTO);
    }
    else if (token->type == CSS_TOKEN_STRING) {
        uint32_t snumber;

        error = css__stylesheet_string_add(c->sheet,
                lwc_string_ref(token->idata), &snumber);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        error = css__stylesheet_style_appendOPV(result,
                CSS_PROP_FOIL_CANDIDATE_MARKS, 0, FOIL_CANDIDATE_MARKS_SET);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        error = css__stylesheet_style_append(result, snumber);
    }
    else {
        error = CSS_INVALID;
    }

    if (error != CSS_OK)
        *ctx = orig_ctx;

    return error;
}

