/**
 * collectd - src/match_value.c
 * Copyright (C) 2019 Takuro Ashie <ashie@clear-code.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Takuro Ashie <ashie at clear-code.com>
 **/


#include "collectd.h"

#include "filter_chain.h"

/*
 * private data types
 */
struct mi_match_s;
typedef struct mi_match_s mi_match_t;
struct mi_match_s {
};

/*
 * internal helper functions
 */
static void mi_free_match(mi_match_t *m) /* {{{ */
{
  free(m);
} /* }}} void mi_free_match */

static int mi_create(const oconfig_item_t *ci, void **user_data) /* {{{ */
{
  mi_match_t *m;
  int status = 0;

  m = calloc(1, sizeof(*m));
  if (m == NULL) {
    ERROR("mi_create: calloc failed.");
    return -ENOMEM;
  }

  *user_data = m;

  return status;
} /* }}} int mi_create */

static int mi_destroy(void **user_data) /* {{{ */
{
  if ((user_data != NULL) && (*user_data != NULL))
    mi_free_match(*user_data);
  return 0;
} /* }}} int mi_destroy */

static int mi_match(const data_set_t *ds, const value_list_t *vl, /* {{{ */
                    notification_meta_t __attribute__((unused)) * *meta,
                    void **user_data) {
  int status = FC_MATCH_NO_MATCH;

  if ((user_data == NULL) || (*user_data == NULL))
    return -1;

  return status;
} /* }}} int mi_match */

void module_register(void) {
  match_proc_t mproc = {0};

  mproc.create = mi_create;
  mproc.destroy = mi_destroy;
  mproc.match = mi_match;
  fc_register_match("interval", mproc);
} /* module_register */
