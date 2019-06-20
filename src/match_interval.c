/**
 * collectd - src/match_interval.c
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
#include "utils/common/common.h"
#include "utils/avltree/avltree.h"

/*
 * private data types
 */
struct mi_match_s;
typedef struct mi_match_s mi_match_t;
struct mi_match_s {
  gauge_t min;
  gauge_t max;
  int invert;
  c_avl_tree_t *timestamps;
};

/*
 * internal helper functions
 */
static void mi_free_match(mi_match_t *m) /* {{{ */
{
  char *key = NULL;
  char *value = NULL;
  for (;c_avl_pick(m->timestamps, (void *)&key, (void *)&value) == 0;) {
    sfree(key);
    /* value is not an actual pointer */
  }
  c_avl_destroy(m->timestamps);
  sfree(m);
} /* }}} void mi_free_match */

static int mi_config_add_gauge(gauge_t *ret_value, /* {{{ */
                               oconfig_item_t *ci) {

  if ((ci->values_num != 1) || (ci->values[0].type != OCONFIG_TYPE_NUMBER)) {
    ERROR("`interval' match: `%s' needs exactly one numeric argument.", ci->key);
    return -1;
  }

  *ret_value = ci->values[0].value.number;

  return 0;
} /* }}} int mi_config_add_gauge */

static int mi_config_add_boolean(int *ret_value, /* {{{ */
                                 oconfig_item_t *ci) {

  if ((ci->values_num != 1) || (ci->values[0].type != OCONFIG_TYPE_BOOLEAN)) {
    ERROR("`interval' match: `%s' needs exactly one boolean argument.", ci->key);
    return -1;
  }

  if (ci->values[0].value.boolean)
    *ret_value = 1;
  else
    *ret_value = 0;

  return 0;
} /* }}} int mi_config_add_boolean */

static int mi_create(const oconfig_item_t *ci, void **user_data) /* {{{ */
{
  mi_match_t *m;
  int status = 0;

  m = calloc(1, sizeof(*m));
  if (m == NULL) {
    ERROR("mi_create: calloc failed.");
    return -ENOMEM;
  }

  m->timestamps = c_avl_create((int (*)(const void *, const void *))strcmp);

  for (int i = 0; i < ci->children_num; i++) {
    oconfig_item_t *child = ci->children + i;

    if (strcasecmp("Min", child->key) == 0)
      status = mi_config_add_gauge(&m->min, child);
    else if (strcasecmp("Max", child->key) == 0)
      status = mi_config_add_gauge(&m->max, child);
    else if (strcasecmp("Invert", child->key) == 0)
      status = mi_config_add_boolean(&m->invert, child);
    else {
      ERROR("`interval' match: The `%s' configuration option is not "
            "understood and will be ignored.",
            child->key);
      status = 0;
    }

    if (status != 0)
      break;
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
  mi_match_t *m;
  int match_status = FC_MATCH_MATCHES;
  int nomatch_status = FC_MATCH_NO_MATCH;
  char identifier[768];
  int status;
  cdtime_t timestamp, now = cdtime();

  if ((user_data == NULL) || (*user_data == NULL))
    return nomatch_status;

  m = *user_data;
  if (m->invert) {
    match_status = FC_MATCH_NO_MATCH;
    nomatch_status = FC_MATCH_MATCHES;
  }

  status = FORMAT_VL(identifier, sizeof(identifier), vl);
  if (status != 0)
    return FC_MATCH_NO_MATCH;

  if (c_avl_get(m->timestamps, identifier, (void**)&timestamp)) {
    /* not found */
    c_avl_insert(m->timestamps, sstrdup(identifier), (void**)now);
    return FC_MATCH_NO_MATCH;
  }

  /* found */

  if (false) {
    return match_status;
  }

  return nomatch_status;
} /* }}} int mi_match */

void module_register(void) {
  match_proc_t mproc = {0};

  mproc.create = mi_create;
  mproc.destroy = mi_destroy;
  mproc.match = mi_match;
  fc_register_match("interval", mproc);
} /* module_register */
