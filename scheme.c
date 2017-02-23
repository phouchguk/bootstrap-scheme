#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_MAX 1000

/* MODEL */

typedef enum {BOOLEAN, CHARACTER, FIXNUM, STRING, THE_EMPTY_LIST} object_type;

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
object *false;
object *true;

void init(void) {
  the_empty_list = alloc_object();
  the_empty_list->type = THE_EMPTY_LIST;

  false = alloc_object();
  false->type = BOOLEAN;
  false->data.boolean.value = 0;

  true = alloc_object();
  true->type = BOOLEAN;
  true->data.boolean.value = 1;
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

char is_string(object *obj) {
  return obj->type == STRING;
}

char is_the_empty_list(object *obj) {
  return obj->type == THE_EMPTY_LIST;
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

    buffer[i] = '\0';

    return make_string(buffer);
  } else if (c == '(') {
    eat_whitespace(in);
    c = getc(in);

    if (c == ')') {
      return the_empty_list;
    } else {
      fprintf(stderr,
	      "unexpected character '%c'. "
	      "Expecting ')'\n", c);
      exit(1);
    }
  } else {
    fprintf(stderr, "bad input. Unexpected '%c'\n", c);
    exit(1);
  }

    fprintf(stderr, "read illegal state\n");
    exit(1);
}

/* EVAL */

object *eval(object *exp) {
  return exp;
}

/* PRINT */

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

  case THE_EMPTY_LIST:
    printf("()");
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
