/*
** backtrace.c -
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include "mruby/proc.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/array.h"

mrb_value
mrb_backtrace(mrb_state *mrb, mrb_int ciidx, mrb_bool show_i)
{
  mrb_callinfo *ci;
  const char *filename, *method, *sep;
  int i, line;
  mrb_value result;
  mrb_value str, tmp;

  result = mrb_ary_new(mrb);

  for (i = ciidx; i >= 0; i--) {
    ci = &mrb->c->cibase[i];
    filename = "(unknown)";
    line = -1;

    if (MRB_PROC_CFUNC_P(ci->proc)) {
      continue;
    }
    else {
      mrb_irep *irep = ci->proc->body.irep;
      if (irep->filename != NULL)
        filename = irep->filename;
      if (irep->lines != NULL) {
        mrb_code *pc;

        if (i+1 <= ciidx) {
          pc = mrb->c->cibase[i+1].pc;
        }
        else {
          pc = (mrb_code*)mrb_voidp(mrb_obj_iv_get(mrb, mrb->exc, mrb_intern2(mrb, "lastpc", 6)));
        }

        if (irep->iseq <= pc && pc < irep->iseq + irep->ilen) {
          line = irep->lines[pc - irep->iseq - 1];
        }
      }
    }
    if (line == -1)
      continue;
    if (ci->target_class == ci->proc->target_class)
      sep = ".";
    else
      sep = "#";
    method = mrb_sym2name(mrb, ci->mid);
    fprintf(stderr, "--- %s\n", method);

    /* concat the caller string */
    if (method) {
      const char *cn = mrb_class_name(mrb, ci->proc->target_class);
      if (cn)
        str = mrb_str_new_format(mrb, "%s:%d:in %s%s%s\n", filename, line, cn, sep, method);
      else
        str = mrb_str_new_format(mrb, "%s:%d:in %s\n", filename, line, method);
    }
    else {
      str = mrb_str_new_format(mrb, "%s:%d\n", filename, line);
    }
    if (show_i) {
      tmp = mrb_str_new_format(mrb, "[%d] ", i);
      mrb_str_concat(mrb, tmp, str);
      str = tmp;
    }
    mrb_ary_push(mrb, result, str);
  }
  return result;
}

void
mrb_print_backtrace(mrb_state *mrb)
{
#ifdef ENABLE_STDIO
  mrb_int ciidx;
  mrb_value nil = mrb_nil_value();
  mrb_value str, ary;

  ciidx = mrb_fixnum(mrb_obj_iv_get(mrb, mrb->exc, mrb_intern2(mrb, "ciidx", 5)));
  if (ciidx >= mrb->c->ciend - mrb->c->cibase)
    ciidx = 10;

  fputs("trace:\n", stderr);
  ary = mrb_backtrace(mrb, ciidx, TRUE);
  while (TRUE) {
    str = mrb_ary_shift(mrb, ary);
    if (mrb_obj_equal(mrb, str, nil))
      break;
    fprintf(stderr, "\t%s", RSTRING_PTR(str));
  }
#endif
}
