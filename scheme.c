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

typedef enum {BOOLEAN, CHARACTER, FIXNUM, PAIR,
              STRING, SYMBOL, THE_EMPTY_LIST, THE_EMPTY_STRING} object_type;

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
      long value;
    } fixnum;
    struct {
      struct object *car;
      struct object *cdr;
    } pair;
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

object *define_symbol;
object *if_symbol;
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


object *make_symbol(char *value);
object *setup_environment(void);

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

  define_symbol = make_symbol("define");
  if_symbol = make_symbol("if");
  ok_symbol = make_symbol("ok");
  quote_symbol = make_symbol("quote");
  set_symbol = make_symbol("set!");

  the_empty_environment = the_empty_list;
  the_global_enironment = setup_environment();
}

char is_boolean(object *obj) {
  return obj->type == BOOLEAN;
}

char is_character(object *obj) {
  return obj->type == CHARACTER;
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

char is_string(object *obj) {
  return obj->type == STRING;
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

object *definition_value(object *exp) {
  return caddr(exp);
}

object *definition_variable(object *exp) {
  return cadr(exp);
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

char is_if(object *exp) {
  return is_tagged_list(exp, if_symbol);
}

char is_assignment(object *exp) {
  return is_tagged_list(exp, set_symbol);
}

char is_definition(object *exp) {
  return is_tagged_list(exp, define_symbol);
}

char is_quoted(object *exp) {
  return is_tagged_list(exp, quote_symbol);
}

char is_self_evaluating(object *exp) {
  return is_boolean(exp) ||
    is_fixnum(exp) ||
    is_character(exp) ||
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

object *text_of_quotation(object *exp) {
  return cadr(exp);
}

object *eval(object *exp, object *env);

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

  case FIXNUM:
    printf("%ld", obj->data.fixnum.value);

    break;

  case PAIR:
    printf("(");
    write_pair(obj);
    printf(")");

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
