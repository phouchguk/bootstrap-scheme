#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_MAX 1000

#define caar(obj)   car(car(obj))
#define cadr(obj)   car(cdr(obj))
#define cdar(obj)   cdr(car(obj))
#define cddr(obj)   cdr(cdr(obj))
#define caaar(obj)  car(car(car(obj)))
#define caadr(obj)  car(car(cdr(obj)))
#define cadar(obj)  car(cdr(car(obj)))
#define caddr(obj)  car(cdr(cdr(obj)))
#define cdaar(obj)  cdr(car(car(obj)))
#define cdadr(obj)  cdr(car(cdr(obj)))
#define cddar(obj)  cdr(cdr(car(obj)))
#define cdddr(obj)  cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))

/* MODEL */

typedef enum {BOOLEAN, CHARACTER, COMPOUND_PROC, FIXNUM,
              PAIR, PRIMITIVE_PROC, STRING, SYMBOL,
              THE_EMPTY_LIST, THE_EMPTY_STRING} object_type;

typedef struct object {
  object_type type;

  union {
    struct {
      char value;
    } boolean;
    struct {
      char value;
    } character;
    struct {
      struct object *parameters;
      struct object *body;
      struct object *env;
    } compound_proc;
    struct {
      long value;
    } fixnum;
    struct {
      struct object *car;
      struct object *cdr;
    } pair;
    struct {
      struct object *(*fn)(struct object *arguments);
    } primitive_proc;
    struct {
      char *value;
    } symbol;
    struct {
      char *value;
    } string;
  } data;
} object;

object *alloc_object(void) {
  object *obj;

  obj = malloc(sizeof(object));

  if (obj == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  return obj;
}

object *the_empty_list;
object *the_empty_string;

object *false;
object *true;

object *symbol_table;

object *begin_symbol;
object *cond_symbol;
object *define_symbol;
object *else_symbol;
object *if_symbol;
object *lambda_symbol;
object *let_symbol;
object *ok_symbol;
object *quote_symbol;
object *set_symbol;

object *the_empty_environment;
object *the_global_enironment;

object *car(object *pair);
object *cdr(object *pair);
object *cons(object *car, object *cdr);
void set_car(object *obj, object *value);
void set_cdr(object *obj, object *value);

void add_binding_to_frame(object *var,
			  object *val,
			  object *frame) {
  set_car(frame, cons(var, car(frame)));
  set_cdr(frame, cons(val, cdr(frame)));
}

object *car(object *pair) {
  return pair->data.pair.car;
}

object *cdr(object *pair) {
  return pair->data.pair.cdr;
}

object *cons(object *car, object *cdr) {
  object *obj;

  obj = alloc_object();
  obj->type = PAIR;
  obj->data.pair.car = car;
  obj->data.pair.cdr = cdr;

  return obj;
}

object *enclosing_environment(object *env) {
  return cdr(env);
}

object *first_frame(object *env);
object *frame_values(object *frame);
object *frame_variables(object *frame);
char is_the_empty_list(object *obj);

void define_variable(object *var, object *val, object *env) {
  object *frame;
  object *vars;
  object *vals;

  frame = first_frame(env);
  vars = frame_variables(frame);
  vals = frame_values(frame);

  while (!is_the_empty_list(vars)) {
    if (var == car(vars)) {
      set_car(vals, val);

      return;
    }

    vars = cdr(vars);
    vals = cdr(vals);
  }

  add_binding_to_frame(var, val, frame);
}

object *make_frame(object *vars, object *vals);

object *extend_environment(object *vars,
			   object *vals,
			   object *base_env) {
  return cons(make_frame(vars, vals), base_env);
}

object *first_frame(object *env) {
  return car(env);
}

object *frame_values(object *frame) {
  return cdr(frame);
}

object *frame_variables(object *frame) {
  return car(frame);
}

char is_boolean(object *obj) {
  return obj->type == BOOLEAN;
}

char is_character(object *obj) {
  return obj->type == CHARACTER;
}

char is_compound_proc(object *obj) {
  return obj->type == COMPOUND_PROC;
}

char is_false(object *obj) {
  return obj == false;
}

char is_fixnum(object *obj) {
  return obj->type == FIXNUM;
}

char is_initial(int c) {
  return isalpha(c) || c == '*' || c == '/' || c == '>' ||
    c == '<' || c == '=' || c == '?' || c == '!';
}

char is_pair(object *obj) {
  return obj->type == PAIR;
}

char is_primitive_proc(object *obj) {
  return obj->type == PRIMITIVE_PROC;
}

char is_string(object *obj) {
  return obj->type == STRING || obj->type == THE_EMPTY_STRING;
}

char is_symbol(object *obj) {
  return obj->type == SYMBOL;
}

char is_the_empty_list(object *obj) {
  return obj->type == THE_EMPTY_LIST;
}

char is_the_empty_string(object *obj) {
  return obj->type == THE_EMPTY_STRING;
}

char is_true(object *obj) {
  return !is_false(obj);
}

object *lookup_variable_value(object *var, object *env) {
  object *frame;
  object *vars;
  object *vals;

  while (!is_the_empty_list(env)) {
    frame = first_frame(env);
    vars = frame_variables(frame);
    vals = frame_values(frame);

    while (!is_the_empty_list(vars)) {
      if (var == car(vars)) {
	return car(vals);
      }

      vars = cdr(vars);
      vals = cdr(vals);
    }

    env = enclosing_environment(env);
  }

  fprintf(stderr, "unbound variable\n");
  exit(1);
}

object *make_character(char value) {
  object *obj;

  obj = alloc_object();
  obj->type = CHARACTER;
  obj->data.character.value = value;

  return obj;
}

object *make_compound_proc(object *parameters,
                           object *body,
                           object *env) {
  object *obj;

  obj = alloc_object();
  obj->type = COMPOUND_PROC;
  obj->data.compound_proc.parameters = parameters;
  obj->data.compound_proc.body = body;
  obj->data.compound_proc.env = env;

  return obj;
}

object *make_fixnum(long value) {
  object *obj;

  obj = alloc_object();
  obj->type = FIXNUM;
  obj->data.fixnum.value = value;

  return obj;
}

object *make_frame(object *vars, object *vals) {
  return cons(vars, vals);
}

object *make_primitive_proc(object *(*fn)(struct object *arguments)) {
  object *obj;

  obj = alloc_object();
  obj->type = PRIMITIVE_PROC;
  obj->data.primitive_proc.fn = fn;

  return obj;
}

object *make_string(char *value) {
  object *obj;

  obj = alloc_object();
  obj->type = STRING;
  obj->data.string.value = malloc(strlen(value) + 1);

  if (obj->data.string.value == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  strcpy(obj->data.string.value, value);

  return obj;
}

object *make_symbol(char *value) {
  object *obj;
  object *element;

  element = symbol_table;

  while (!is_the_empty_list(element)) {
    if (strcmp(car(element)->data.symbol.value, value) == 0) {
      return car(element);
    }

    element = cdr(element);
  }

  obj = alloc_object();
  obj->type = SYMBOL;
  obj->data.symbol.value = malloc(strlen(value) + 1);

  if (obj->data.symbol.value == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  strcpy(obj->data.symbol.value, value);
  symbol_table = cons(obj, symbol_table);

  return obj;
}

void set_car(object *obj, object *value) {
  obj->data.pair.car = value;
}

void set_cdr(object *obj, object *value) {
  obj->data.pair.cdr = value;
}

void set_variable_value(object *var, object *val, object *env) {
  object *frame;
  object *vars;
  object *vals;

  while (!is_the_empty_list(env)) {
    frame = first_frame(env);
    vars = frame_variables(frame);
    vals = frame_values(frame);

    while (!is_the_empty_list(vars)) {
      if (var == car(vars)) {
	set_car(vals, val);

	return;
      }

      vars = cdr(vars);
      vals = cdr(vals);
    }

    env = enclosing_environment(env);
  }

  fprintf(stderr, "unbound variable\n");
  exit(1);
}

object *setup_environment(void) {
  object *initial_env;

  initial_env = extend_environment(the_empty_list,
				   the_empty_list,
				   the_empty_environment);

  return initial_env;
}

/* BIFS */

object *proc_add(object *arguments) {
  long result = 0;

  while (!is_the_empty_list(arguments)) {
    result += (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }

  return make_fixnum(result);
}

object *proc_car(object *arguments) {
  return caar(arguments);
}

object *proc_set_car(object *arguments) {
  set_car(car(arguments), cadr(arguments));

  return ok_symbol;
}

object *proc_cdr(object *arguments) {
  return cdar(arguments);
}

object *proc_set_cdr(object *arguments) {
  set_cdr(car(arguments), cadr(arguments));

  return ok_symbol;
}

object *proc_cons(object *arguments) {
  return cons(car(arguments), cadr(arguments));
}

object *proc_list(object *arguments) {
  return arguments;
}

object *proc_mul(object *arguments) {
  long result = 1;

  while (!is_the_empty_list(arguments)) {
    result *= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }

  return make_fixnum(result);
}

object *proc_sub(object *arguments) {
  long result;

  result = (car(arguments))->data.fixnum.value;

  while (!is_the_empty_list(arguments = cdr(arguments))) {
    result -= (car(arguments))->data.fixnum.value;
  }

  return make_fixnum(result);
}

object *proc_quotient(object *arguments) {
  return make_fixnum(((car(arguments))->data.fixnum.value) /
		     ((cadr(arguments))->data.fixnum.value));

}

object *proc_remainder(object *arguments) {
  return make_fixnum(((car(arguments))->data.fixnum.value) %
		     ((cadr(arguments))->data.fixnum.value));

}

object *proc_is_boolean(object *arguments) {
  return is_boolean(car(arguments)) ? true : false;
}

object *proc_is_char(object *arguments) {
  return is_character(car(arguments)) ? true : false;
}

object *proc_is_eq(object *arguments) {
  object *obj1;
  object *obj2;

  obj1 = car(arguments);
  obj2 = cadr(arguments);

  if (obj1->type != obj2->type) {
    return false;
  }

  switch (obj1->type) {
  case FIXNUM:
    return (obj1->data.fixnum.value ==
	    obj2->data.fixnum.value) ?
      true : false;

    break;

  case CHARACTER:
    return (obj1->data.character.value ==
	    obj2->data.character.value) ?
      true : false;

    break;

  case STRING:
    return (strcmp(obj1->data.string.value,
		   obj2->data.string.value) == 0) ?
      true : false;

    break;

  default:
    return (obj1 == obj2) ? true : false;

  }
}

object *proc_is_greater_than(object *arguments) {
  long previous;
  long next;

  previous = (car(arguments))->data.fixnum.value;

  while (!is_the_empty_list(arguments = cdr(arguments))) {
    next = (car(arguments))->data.fixnum.value;

    if (previous <= next) {
      return false;
    }

    previous = next;
  }

  return true;
}

object *proc_is_integer(object *arguments) {
  return is_fixnum(car(arguments)) ? true : false;
}

object *proc_is_less_than(object *arguments) {
  long previous;
  long next;

  previous = (car(arguments))->data.fixnum.value;

  while (!is_the_empty_list(arguments = cdr(arguments))) {
    next = (car(arguments))->data.fixnum.value;

    if (previous >= next) {
      return false;
    }

    previous = next;
  }

  return true;
}

object *proc_is_null(object *arguments) {
  return is_the_empty_list(car(arguments)) ? true : false;
}

object *proc_is_number_equal(object *arguments) {
  long value;

  value = (car(arguments))->data.fixnum.value;

  while (!is_the_empty_list(arguments = cdr(arguments))) {
    if (value != (car(arguments))->data.fixnum.value) {
      return false;
    }
  }

  return true;
}

object *proc_is_pair(object *arguments) {
  return is_pair(car(arguments)) ? true : false;
}

object *proc_is_procedure(object *arguments) {
  object *obj;

  obj = car(arguments);

  return (is_primitive_proc(obj) || is_compound_proc(obj)) ? true : false;
}

object *proc_is_string(object *arguments) {
  return is_string(car(arguments)) ? true : false;
}

object *proc_is_symbol(object *arguments) {
  return is_symbol(car(arguments)) ? true : false;
}

object *proc_char_to_integer(object *arguments) {
  return make_fixnum((car(arguments))->data.character.value);
}

object *proc_integer_to_char(object *arguments) {
  return make_character((car(arguments))->data.fixnum.value);
}

object *proc_number_to_string(object *arguments) {
  char buffer[100];

  sprintf(buffer, "%ld", (car(arguments))->data.fixnum.value);

  return make_string(buffer);
}

object *proc_string_to_number(object *arguments) {
  return make_fixnum(atoi((car(arguments))->data.string.value));
}

object *proc_string_to_symbol(object *arguments) {
  return make_symbol((car(arguments))->data.string.value);
}

object *proc_symbol_to_string(object *arguments) {
  return make_string((car(arguments))->data.symbol.value);
}

void init(void) {
  the_empty_list = alloc_object();
  the_empty_list->type = THE_EMPTY_LIST;

  the_empty_string = alloc_object();
  the_empty_string->type = THE_EMPTY_STRING;

  false = alloc_object();
  false->type = BOOLEAN;
  false->data.boolean.value = 0;

  true = alloc_object();
  true->type = BOOLEAN;
  true->data.boolean.value = 1;

  symbol_table = the_empty_list;

  begin_symbol = make_symbol("begin");
  cond_symbol = make_symbol("cond");
  define_symbol = make_symbol("define");
  else_symbol = make_symbol("else");
  if_symbol = make_symbol("if");
  lambda_symbol = make_symbol("lambda");
  let_symbol = make_symbol("let");
  ok_symbol = make_symbol("ok");
  quote_symbol = make_symbol("quote");
  set_symbol = make_symbol("set!");

  the_empty_environment = the_empty_list;
  the_global_enironment = setup_environment();

#define add_procedure(scheme_name, c_name) \
  define_variable(make_symbol(scheme_name), \
		  make_primitive_proc(c_name), \
		  the_global_enironment);

  add_procedure("null?", proc_is_null);
  add_procedure("boolean?", proc_is_boolean);
  add_procedure("symbol?", proc_is_symbol);
  add_procedure("integer?", proc_is_integer);
  add_procedure("char?", proc_is_char);
  add_procedure("string?", proc_is_string);
  add_procedure("pair?", proc_is_pair);
  add_procedure("procedure?", proc_is_procedure);

  add_procedure("char->integer", proc_char_to_integer);
  add_procedure("integer->char", proc_integer_to_char);
  add_procedure("number->string", proc_number_to_string);
  add_procedure("string->number", proc_string_to_number);
  add_procedure("symbol->string", proc_symbol_to_string);
  add_procedure("string->symbol", proc_string_to_symbol);

  add_procedure("+", proc_add);
  add_procedure("-", proc_sub);
  add_procedure("*", proc_mul);
  add_procedure("quotient", proc_quotient);
  add_procedure("remainder", proc_remainder);
  add_procedure("=", proc_is_number_equal);
  add_procedure("<", proc_is_less_than);
  add_procedure(">", proc_is_greater_than);

  add_procedure("cons", proc_cons);
  add_procedure("car", proc_car);
  add_procedure("cdr", proc_cdr);
  add_procedure("set-car!", proc_set_car);
  add_procedure("set-cdr!", proc_set_cdr);
  add_procedure("list", proc_list);

  add_procedure("eq?", proc_is_eq);
}

/* READ */

char is_delimiter(int c) {
  return isspace(c) || c == EOF ||
    c == '(' || c == ')' ||
    c == '"' || c == ';';
}

int peek(FILE *in) {
  int c;

  c = getc(in);
  ungetc(c, in);

  return c;
}

void eat_whitespace(FILE *in) {
  int c;

  while ((c = getc(in)) != EOF) {
    if (isspace(c)) {
      continue;
    } else if (c == ';') {
      while (((c = getc(in)) != EOF) && (c != '\n'));
      continue;
    }

    ungetc(c, in);

    break;
  }
}

void eat_expected_string(FILE *in, char *str) {
  int c;

  while (*str != '\0') {
    c = getc(in);

    if (c != *str) {
      fprintf(stderr, "unexpected character '%c'\n", c);
      exit(1);
    }

    str++;
  }
}

void peek_expected_delimiter(FILE *in) {
  if (!is_delimiter(peek(in))) {
    fprintf(stderr, "character not followed by delimiter\n");
    exit(1);
  }
}

object *read_character(FILE *in) {
  int c;

  c = getc(in);

  switch (c) {
  case EOF:
    fprintf(stderr, "incomplete character literal\n");
    exit(1);
  case 's':
    if (peek(in) == 'p') {
      eat_expected_string(in, "pace");
      peek_expected_delimiter(in);

      return make_character(' ');
    }

    break;
  case 'n':
    if (peek(in) == 'e') {
      eat_expected_string(in, "ewline");
      peek_expected_delimiter(in);

      return make_character('\n');
    }

    break;
  }

  peek_expected_delimiter(in);

  return make_character(c);
}

object *read(FILE *in);

object *read_pair(FILE *in) {
  int c;
  object *car_obj;
  object *cdr_obj;

  eat_whitespace(in);

  c = getc(in);

  if (c == ')') { /* read the empty list */
    return the_empty_list;
  }

  ungetc(c, in);

  car_obj = read(in);

  eat_whitespace(in);

  c = getc(in);

  if (c == '.') { /* read improper list */
    c = peek(in);

    if (!is_delimiter(c)) {
      fprintf(stderr, "dot not followed by delimiter\n");
      exit(1);
    }

    cdr_obj = read(in);
    eat_whitespace(in);
    c = getc(in);

    if (c != ')') {
      fprintf(stderr,
              "where was the trailing right paren?\n");
      exit(1);
    }
  } else {
    ungetc(c, in);
    cdr_obj = read_pair(in);
  }

  return cons(car_obj, cdr_obj);
}

object *read(FILE *in) {
  int c;
  int i;
  short sign = 1;
  long num = 0;
  char buffer[BUFFER_MAX];

  eat_whitespace(in);

  c = getc(in);

  if (c == '#') {
    c = getc(in);

    switch (c) {
    case 't':
      return true;
    case 'f':
      return false;
    case '\\':
      return read_character(in);
    default:
      fprintf(stderr, "unknown boolean or character literal\n");
      exit(1);
    }
  } else if (isdigit(c) || (c == '-' && (isdigit(peek(in))))) {
    if (c == '-') {
      sign = -1;
    } else {
      ungetc(c, in);
    }

    while (isdigit(c = getc(in))) {
      num = (num * 10) + (c - '0');
    }

    num *= sign;

    if (is_delimiter(c)) {
      ungetc(c, in);

      return make_fixnum(num);
    } else {
      fprintf(stderr, "number not followed by delimiter\n");
      exit(1);
    }
  } else if (is_initial(c) ||
             ((c == '+' || c == '-') &&
              is_delimiter(peek(in)))) { /* read a symbol */
    i = 0;

    while (is_initial(c) || isdigit(c) || c == '+' || c == '-') {
      if (i < BUFFER_MAX - 1) {
        buffer[i++] = c;
      } else {
        fprintf(stderr, "symbol too long. "
                "Maximum length is %d\n", BUFFER_MAX);
        exit(1);
      }

      c = getc(in);
    }

    if (is_delimiter(c)) {
      buffer[i] = '\0';
      ungetc(c, in);

      return make_symbol(buffer);
    } else {
      fprintf(stderr, "symbol not followed by a delimiter. "
              "Found '%c'\n", c);
      exit(1);
    }
  } else if (c == '"') {
    i = 0;

    while ((c = getc(in)) != '"') {
      if (c == '\\') {
	c = getc(in);

	if (c == 'n') {
	  c = '\n';
	}
      }

      if (c == EOF) {
	fprintf(stderr, "non-terminated string literal\n");
	exit(1);
      }

      if (i < BUFFER_MAX - 1) {
	buffer[i++] = c;
      } else {
	fprintf(stderr,
		"string too long. Maximum length is %d\n",
		BUFFER_MAX);
	exit(1);
      }
    }

    if (i == 0) { /* read the empty string */
      return the_empty_string;
    }

    buffer[i] = '\0';

    return make_string(buffer);
  } else if (c == '(') { /* read the empty list or pair */
    return read_pair(in);
  } else if (c == '\'') { /* read quoted expression */
    return cons(quote_symbol, cons(read(in), the_empty_string));
  } else {
    fprintf(stderr, "bad input. Unexpected '%c'\n", c);
    exit(1);
  }

  fprintf(stderr, "read illegal state\n");
  exit(1);
}

/* EVAL */

object *assignment_value(object *exp) {
  return caddr(exp);
}

object *assignment_variable(object *exp) {
  return cadr(exp);
}

object *begin_actions(object *exp) {
  return cdr(exp);
}

object *binding_argument(object *binding) {
  return cadr(binding);
}

object *binding_arguments(object *bindings) {
  return is_the_empty_list(bindings) ?
    the_empty_list :
    cons(binding_argument(car(bindings)),
         binding_arguments(cdr(bindings)));
}

object *binding_parameter(object *binding) {
  return car(binding);
}

object *binding_parameters(object *bindings) {
  return is_the_empty_list(bindings) ?
    the_empty_list :
    cons(binding_parameter(car(bindings)),
         binding_parameters(cdr(bindings)));
}

object *cond_actions(object *clause) {
  return cdr(clause);
}

object *cond_clauses(object *exp) {
  return cdr(exp);
}

object *cond_predicate(object *clause) {
  return car(clause);
}

object *expand_clauses(object *clauses);

object *cond_to_if(object *exp) {
  return expand_clauses(cond_clauses(exp));
}

object *make_lambda(object *parameters, object *body);

object *definition_value(object *exp) {
  if (is_symbol(cadr(exp))) {
      return caddr(exp);
  }

  return make_lambda(cdadr(exp), cddr(exp));
}

object *definition_variable(object *exp) {
  if (is_symbol(cadr(exp))) {
    return cadr(exp);
  }

  return caadr(exp);
}

char is_cond_else_clause(object *clause);
object *make_if(object *predicate,
                object *consequent,
                object *alternative);
object *sequence_to_exp(object *seq);

object *expand_clauses(object *clauses) {
  object *first;
  object *rest;

  if (is_the_empty_list(clauses)) {
    return false;
  }

  first = car(clauses);
  rest = cdr(clauses);

  if (is_cond_else_clause(first)) {
    if (is_the_empty_list(rest)) {
      return sequence_to_exp(cond_actions(first));
    }

    fprintf(stderr, "else clause isn't the last cond->if'");
    exit(1);
  }

  return make_if(cond_predicate(first),
                 sequence_to_exp(cond_actions(first)),
                 expand_clauses(rest));
}

object *first_exp(object *seq) {
  return car(seq);
}

object *first_operand(object *ops) {
  return car(ops);
}

object *if_alternative(object *exp) {
  if (is_the_empty_list(cdddr(exp))) {
    return false;
  }

  return cadddr(exp);
}

object *if_consequent(object *exp) {
  return caddr(exp);
}

object *if_predicate(object *exp) {
  return cadr(exp);
}

char is_tagged_list(object *exp, object *tag);

char is_application(object *exp) {
  return is_pair(exp);
}

char is_assignment(object *exp) {
  return is_tagged_list(exp, set_symbol);
}

char is_begin(object *exp) {
  return is_tagged_list(exp, begin_symbol);
}

char is_cond(object *exp) {
  return is_tagged_list(exp, cond_symbol);
}

char is_cond_else_clause(object *clause) {
  return cond_predicate(clause) == else_symbol;
}

char is_definition(object *exp) {
  return is_tagged_list(exp, define_symbol);
}

char is_if(object *exp) {
  return is_tagged_list(exp, if_symbol);
}

char is_lambda(object *exp) {
  return is_tagged_list(exp, lambda_symbol);
}

char is_let(object *exp) {
  return is_tagged_list(exp, let_symbol);
}

char is_last_exp(object *seq) {
  return is_the_empty_list(cdr(seq));
}

char is_no_operands(object *ops) {
  return is_the_empty_list(ops);
}

char is_quoted(object *exp) {
  return is_tagged_list(exp, quote_symbol);
}

char is_self_evaluating(object *exp) {
  return is_boolean(exp) ||
    is_fixnum(exp) ||
    is_character(exp) ||
    is_the_empty_string(exp) ||
    is_string(exp);
}

char is_tagged_list(object *exp, object *tag) {
  object *the_car;

  if (is_pair(exp)) {
    the_car = car(exp);

    return is_symbol(the_car) && (the_car == tag);
  }

  return 0;
}

char is_variable(object *exp) {
  return is_symbol(exp);
}

object *lambda_body(object *exp) {
  return cddr(exp);
}

object *lambda_parameters(object *exp) {
  return cadr(exp);
}

object *let_bindings(object *exp);

object *let_arguments(object *exp) {
  return binding_arguments(let_bindings(exp));
}

object *let_bindings(object *exp) {
  return cadr(exp);
}

object *let_body(object *exp) {
  return cddr(exp);
}

object *let_parameters(object *exp) {
  return binding_parameters(let_bindings(exp));
}

object *make_application(object *operator, object *operands);

object *let_to_application(object *exp) {
  return make_application(make_lambda(let_parameters(exp),
                                      let_body(exp)),
                          let_arguments(exp));
}

object *make_application(object *operator, object *operands) {
  return cons(operator, operands);
}

object *make_begin(object *exp) {
  return cons(begin_symbol, exp);
}

object *make_if(object *predicate,
                object *consequent,
                object *alternative) {
  return cons(if_symbol,
              cons(predicate,
                   cons(consequent,
                        cons(alternative, the_empty_list))));
}

object *make_lambda(object *parameters, object *body) {
  return cons(lambda_symbol, cons(parameters, body));
}

object *operands(object *exp) {
  return cdr(exp);
}

object *operator(object *exp) {
  return car(exp);
}

object *rest_operands(object *ops) {
  return cdr(ops);
}

object *rest_exps(object *seq) {
  return cdr(seq);
}

object *sequence_to_exp(object *seq) {
  if (is_the_empty_list(seq)) {
    return seq;
  }

  if (is_last_exp(seq)) {
    return first_exp(seq);
  }

  return make_begin(seq);
}

object *text_of_quotation(object *exp) {
  return cadr(exp);
}

object *eval(object *exp, object *env);

object *list_of_values(object *exps, object *env) {
  if (is_no_operands(exps)) {
    return the_empty_list;
  }

  return cons(eval(first_operand(exps), env),
	      list_of_values(rest_operands(exps), env));
}

object *eval_assignment(object *exp, object *env) {
  set_variable_value(assignment_variable(exp),
		     eval(assignment_value(exp), env),
		     env);

  return ok_symbol;
}

object *eval_definition(object *exp, object *env) {
  define_variable(definition_variable(exp),
		  eval(definition_value(exp), env),
		  env);

  return ok_symbol;
}

object *eval(object *exp, object *env) {
  object *procedure;
  object *arguments;

 tailcall:
  if (is_self_evaluating(exp)) {
    return exp;
  }

  if (is_variable(exp)) {
    return lookup_variable_value(exp, env);
  }

  if (is_quoted(exp)) {
    return text_of_quotation(exp);
  }

  if (is_assignment(exp)) {
    return eval_assignment(exp, env);
  }

  if (is_definition(exp)) {
    return eval_definition(exp, env);
  }

  if (is_if(exp)) {
    exp = is_true(eval(if_predicate(exp), env)) ?
      if_consequent(exp) :
      if_alternative(exp);

    goto tailcall;
  }

  if (is_lambda(exp)) {
    return make_compound_proc(lambda_parameters(exp),
                              lambda_body(exp),
                              env);
  }

  if (is_begin(exp)) {
    exp = begin_actions(exp);

    while (!is_last_exp(exp)) {
      eval(first_exp(exp), env);
      exp = rest_exps(exp);
    }

    exp = first_exp(exp);

    goto tailcall;
  }

  if (is_cond(exp)) {
    exp = cond_to_if(exp);

    goto tailcall;
  }

  if (is_let(exp)) {
    exp = let_to_application(exp);

    goto tailcall;
  }

  if (is_application(exp)) {
    procedure = eval(operator(exp), env);
    arguments = list_of_values(operands(exp), env);

    if (is_primitive_proc(procedure)) {
      return (procedure->data.primitive_proc.fn)(arguments);
    }

    if (is_compound_proc(procedure)) {
      env = extend_environment(procedure->data.compound_proc.parameters,
                               arguments,
                               procedure->data.compound_proc.env);

      exp = make_begin(procedure->data.compound_proc.body);

      goto tailcall;
    }

    fprintf(stderr, "unknown procedure type\n");
    exit(1);
  }

  fprintf(stderr, "cannot eval unknown expression type\n");
  exit(1);
}

/* PRINT */

void write(object *obj);

void write_pair(object *pair) {
  object *car_obj;
  object *cdr_obj;

  car_obj = car(pair);
  cdr_obj = cdr(pair);

  write(car_obj);

  if (cdr_obj->type == PAIR) {
    printf(" ");
    write_pair(cdr_obj);
  } else if (cdr_obj->type == THE_EMPTY_LIST) {
    return;
  } else {
    printf(" . ");
    write(cdr_obj);
  }
}

void write(object *obj) {
  char c;
  char *str;
  object *body;

  switch (obj->type) {
  case BOOLEAN:
    printf("#%c", is_false(obj) ? 'f' : 't');

    break;

  case CHARACTER:
    c = obj->data.character.value;
    printf("#\\");

    switch (c) {
    case '\n':
      printf("newline");
      break;
    case ' ':
      printf("space");
      break;
    default:
      putchar(c);
    }

    break;

  case COMPOUND_PROC:
    body = obj->data.compound_proc.body;

    printf("(lambda ");
    write(obj->data.compound_proc.parameters);
    printf(" ");

    if (is_pair(body)) {
      write_pair(body);
    } else {
      write(body);
    }

    printf(")");

    break;

  case FIXNUM:
    printf("%ld", obj->data.fixnum.value);

    break;

  case PAIR:
    printf("(");
    write_pair(obj);
    printf(")");

    break;

  case PRIMITIVE_PROC:
    printf("#<procedure>");

    break;

  case STRING:
    str = obj->data.string.value;

    putchar('"');

    while (*str != '\0') {
      switch (*str) {
      case '\n':
	printf("\\n");

	break;

      case '\\':
	printf("\\\\");

	break;

      case '"':
	printf("\\\"");
	break;

      default:
	putchar(*str);
      }

      str++;
    }

    putchar('"');

    break;

  case SYMBOL:
    printf("%s", obj->data.symbol.value);

    break;

  case THE_EMPTY_LIST:
    printf("()");
    break;

  case THE_EMPTY_STRING:
    printf("\"\"");
    break;

  default:
    fprintf(stderr, "cannot write unknown type\n");

    exit(1);
  }
}

/* REPL */

int main(void) {
  printf("Welcome to Bootstrap Scheme. "
	 "Use ctrl-c to exit.\n");

  init();

  while (1) {
    printf("> ");
    write(eval(read(stdin), the_global_enironment));
    printf("\n");
  }

  return 0;
}
