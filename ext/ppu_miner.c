
# include <stdint.h>
# include <string.h>
# include <pthread.h>
# include <ruby.h>

# include "ppu/params.h"

extern int ppu_mine(volatile struct worker_params *);

struct ppu_miner {
  volatile struct worker_params params;

  pthread_t thread;
};

static
VALUE m_initialize(int argc, VALUE *argv, VALUE self)
{
  struct ppu_miner *miner;

  if (argc < 0 || argc > 1)
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 0..1)", argc);

  Data_Get_Struct(self, struct ppu_miner, miner);

  if (argc > 0 && RTEST(argv[0]))
    miner->params.flags |= WORKER_FLAG_DEBUG;

  return self;
}

static
void *run_miner(void *data)
{
  struct ppu_miner *miner = data;
  int oldvalue;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldvalue);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldvalue);

  switch (ppu_mine(&miner->params)) {
  case WORKER_VERIFY_ERROR:
    return (void *) Qnil;

  case WORKER_FOUND_NOTHING:
    return (void *) Qfalse;

  case WORKER_FOUND_SOMETHING:
    return (void *) Qtrue;
  }

  return (void *) Qundef;
}

static
VALUE wait_miner(void *data)
{
  struct ppu_miner *miner = data;
  void *retval;

  if (pthread_join(miner->thread, &retval))
    return Qundef;

  if (retval == PTHREAD_CANCELED)
    return Qfalse;

  return (VALUE) retval;
}

static
void unblock_miner(void *data)
{
  struct ppu_miner *miner = data;

  pthread_cancel(miner->thread);
}

static
VALUE m_run(VALUE self, VALUE data, VALUE target, VALUE midstate,
	    VALUE start_nonce, VALUE range)
{
  struct ppu_miner *miner;
  VALUE retval;

  Data_Get_Struct(self, struct ppu_miner, miner);

  /* prepare parameters */

  StringValue(data);
  StringValue(target);
  StringValue(midstate);

  if (RSTRING_LEN(data) != 128)
    rb_raise(rb_eArgError, "data must be 128 bytes");
  if (RSTRING_LEN(target) != 32)
    rb_raise(rb_eArgError, "target must be 32 bytes");
  if (RSTRING_LEN(midstate) != 32)
    rb_raise(rb_eArgError, "midstate must be 32 bytes");

  memcpy((void *) miner->params.data,     RSTRING_PTR(data),    128);
  memcpy((void *) miner->params.target,   RSTRING_PTR(target),   32);
  memcpy((void *) miner->params.midstate, RSTRING_PTR(midstate), 32);

  miner->params.start_nonce = NUM2ULONG(start_nonce);
  miner->params.range       = NUM2ULONG(range);

  /* start a mining thread and unlock the Global Interpreter Lock */

  if (pthread_create(&miner->thread, 0, run_miner, miner))
    rb_raise(rb_eRuntimeError, "failed to create PPU thread");

  retval = rb_thread_blocking_region(wait_miner, miner,
				     unblock_miner, miner);

  switch (retval) {
  case Qtrue:
    retval = rb_str_new((const char *) miner->params.data, 128);
    break;

  case Qnil:
    rb_raise(rb_eArgError, "midstate verification failed");
    break;

  case Qundef:
    rb_raise(rb_eRuntimeError, "PPU thread error");
    break;
  }

  return retval;
}

static
VALUE i_allocate(VALUE klass)
{
  struct ppu_miner *miner;

  return Data_Make_Struct(klass, struct ppu_miner, 0, RUBY_DEFAULT_FREE, miner);
}

void Init_ppu_miner(VALUE container)
{
  VALUE cPPUMiner;

  cPPUMiner = rb_define_class_under(container, "PPUMiner", rb_cObject);
  rb_define_alloc_func(cPPUMiner, i_allocate);

  rb_define_method(cPPUMiner, "initialize", m_initialize, -1);
  rb_define_method(cPPUMiner, "run", m_run, 5);
}
