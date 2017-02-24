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
object *quote_symbol;

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

object *make_symbol(char *value);

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
  quote_symbol = make_symbol("quote");
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

char is_quoted(object *exp) {
  return is_tagged_list(exp, quote_symbol);
}

object *text_of_quotation(object *exp) {
  return cadr(exp);
}

object *eval(object *exp) {
  if (is_self_evaluating(exp)) {
    return exp;
  }

  if (is_quoted(exp)) {
    return text_of_quotation(exp);
  }


  fprintf(stderr, "cannot eval unkown expression type\n");
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
    write(eval(read(stdin)));
    printf("\n");
  }

  return 0;
}
