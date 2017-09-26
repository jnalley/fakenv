#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/*
 * Usage:
 *
 * LD_PRELOAD=${PWD}/libfakenv.so FOO=bar /bin/sh -c \
 *  'echo $FOO; tr "\0" "\n" < /proc/$$/environ | grep FOO'
 *
 * Observe that the value in /proc/$$/environ is 'bar' while the value
 * returned by $FOO is a random number.
 *
 * This behavior is due to /proc/$$/environ being populated when exec(3) is
 * called by the parent process.  This is before the constructor in
 * libfakenv.so has been invoked.
 */

static void setup() __attribute__((constructor()));
static void teardown() __attribute__((destructor()));

static char *buffer = NULL;

void die(const char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

void setup(void)
{
    if (unsetenv("LD_PRELOAD") != 0) {
        die("Failed to unset LD_PRELOAD");
    }

    if ((buffer = malloc(32)) == NULL) {
        die("Failed to allocate buffer");
    }

    srand(time(NULL));

    snprintf(buffer, (size_t) 32, "FOO=%d", rand());

    if (putenv(buffer) != 0) {
        die("Failed to set default environment");
    }
}

void teardown(void)
{
  if (NULL != buffer) {
    free(buffer);
  }
}
