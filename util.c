/*
 *  util.c
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>

#include "util.h"

void delete_file(const char *filename) {
  if (filename == NULL) return;

  struct stat st;

  if (stat(filename, &st) == 0)
    unlink(filename);
}

void die(const char *errstr, ...) {
  va_list ap;

  va_start(ap, errstr);
  vfprintf(stderr, errstr, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

char *expand_tilde(char *path) {
  if (! line_starts_with(path, "~/"))
    return strndup(path, PATH_MAX);

  char *buf;

  buf = calloc(1, PATH_MAX + 1);
  if (buf == NULL) {
    fprintf(stderr, "Error allocating %d bytes.\n", PATH_MAX + 1);
    return NULL;
  }

  if (snprintf(buf, PATH_MAX, "%s%s", getenv("HOME"), strchr(path, '/')) > 0) {
    path = buf;
  } else {
    fprintf(stderr, "Error expanding path: %s\n", path);
    free(buf);
  }

  return path;
}

int file_exists(const char *filename) {
  struct stat st;

  return stat(filename, &st) == 0;
}

char* get_password(size_t max_length) {
  struct termios t;
  char *buf;

  buf = calloc(1, max_length + 1);
  if (buf == NULL) {
    fprintf(stderr, "Error allocating %zd bytes.\n", max_length + 1);
    return NULL;
  }

  printf("Enter password: ");

  /* turn off the echo flag */
  tcgetattr(fileno(stdin), &t);
  t.c_lflag &= ~ECHO;
  tcsetattr(fileno(stdin), TCSANOW, &t);

  /* fgets() will leave a newline char on the end */
  fgets(buf, max_length, stdin);
  *(buf + strlen(buf) - 1) = '\0';

  putchar('\n');
  t.c_lflag |= ECHO;
  tcsetattr(fileno(stdin), TCSANOW, &t);

  return buf;
}

char *get_tmpfile(const char *format) {
  char *buf;

  buf = calloc(1, PATH_MAX + 1);
  if (buf == NULL) {
    fprintf(stderr, "Error allocating %d bytes.\n", PATH_MAX + 1);
    return NULL;
  }

  snprintf(buf, PATH_MAX, format, getpid());

  return buf;
}

char *get_username(size_t max_length) {
  char *buf;

  buf = calloc(1, max_length + 1);
  if (buf == NULL) {
    fprintf(stderr, "Error allocating %zd bytes.\n", max_length + 1);
    return NULL;
  }

  printf("Enter username: ");

  /* fgets() will leave a newline char on the end */
  fgets(buf, max_length, stdin);
  *(buf + strlen(buf) - 1) = '\0';

  return buf;
}

int line_starts_with(const char *line, const char *starts_with) {
  return strncmp(line, starts_with, strlen(starts_with)) == 0;
}

char *strtrim(char *str) {
  char *pch = str;

  if (str == NULL || *str == '\0')
    return str;

  while (isspace(*pch)) pch++;

  if (pch != str)
    memmove(str, pch, (strlen(pch) + 1));

  if (*str == '\0')
    return str;

  pch = (str + strlen(str) - 1);

  while (isspace(*pch))
    pch--;

  *++pch = '\0';

  return str;
}

int touch(const char *filename) {
  int fd;

  fd = open(filename, O_WRONLY | O_CREAT | O_NONBLOCK | O_NOCTTY,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  if (fd == -1) {
    return 1;
  }

  return close(fd);
}

void *xmalloc(size_t size) {
  void *ret = malloc(size);
  if (!ret)
    die("error: failed to allocate %zd bytes", size);

  return ret;
}

void *xcalloc(size_t nmemb, size_t size) {
  void *ret = calloc(nmemb, size);
  if (!ret)
    die("error: failed to allocate %zd bytes", size);

  return ret;
}
