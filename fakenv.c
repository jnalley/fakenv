#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

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
 *
 * Other examples:
 *
 * LD_PRELOAD=${PWD}/libfakenv.so FOO=bar lua -l os \
 *  -e 'repeat v=os.getenv("FOO") print(v) until v == "All Done!" os.exit()'
 *
 * LD_PRELOAD=${PWD}/libfakenv.so FOO=bar /bin/sh \
 *  -c 'while :; do echo $FOO; test "$FOO" = "All Done!" && break; done'
 *
 */


static pthread_t my_thread;

static void setup() __attribute__((constructor()));
static void teardown() __attribute__((destructor()));

static char *buffer = NULL;

static void *thread_start(void *arg)
{
  int i,r;
  UNUSED(arg);
  for (i=0;i<100;i++) {
    r = rand();
    sprintf(buffer, "FOO=%d", r);
    usleep(50000);
  }
  sprintf(buffer, "FOO=All Done!");
  return NULL;
}

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

    if(pthread_create(&my_thread, NULL, &thread_start, NULL)) {
      die("Error creating thread\n");
    }
}

void teardown(void)
{
  if(pthread_join(my_thread, NULL)) {
    die("Error joining thread\n");
  }

  if (NULL != buffer) {
    free(buffer);
  }
}
