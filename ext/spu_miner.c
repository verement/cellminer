
# include <stdio.h>

# include <stdlib.h>
# include <ruby.h>
# include <libspe2.h>
# include <pthread.h>

# include "spu/params.h"

struct spu_miner {
  /* must be first for proper alignment */
  volatile struct worker_params params;

  spe_context_ptr_t spe_context;
  uint32_t entry;
};

static
VALUE m_initialize(VALUE self)
{
  struct spu_miner *miner;
  extern spe_program_handle_t spu_worker;

  Data_Get_Struct(self, struct spu_miner, miner);

  if (spe_program_load(miner->spe_context, &spu_worker))
    rb_raise(rb_eRuntimeError, "failed to load SPE program");

  fprintf(stderr, "Bitcoin::SPUMiner initialized (thread %lx)\n",
	  pthread_self());

  return Qnil;
}

static
VALUE m_run(VALUE self, VALUE arg)
{
  struct spu_miner *miner;
  int n = NUM2INT(arg);
  spe_stop_info_t stop_info;
  char *stop_reason = "unknown";
  int code = -1;

  Data_Get_Struct(self, struct spu_miner, miner);

  miner->params.n = n;

  if (spe_context_run(miner->spe_context, &miner->entry, 0,
		      (void *) &miner->params, 0, &stop_info) < 0)
    rb_raise(rb_eRuntimeError, "failed to run SPE context");

  switch (stop_info.stop_reason) {
  case SPE_EXIT:
    stop_reason = "SPE_EXIT";
    code = stop_info.result.spe_exit_code;
    break;
  case SPE_STOP_AND_SIGNAL:
    stop_reason = "SPE_STOP_AND_SIGNAL";
    code = stop_info.result.spe_signal_code;
    break;
  case SPE_RUNTIME_ERROR:
    stop_reason = "SPE_RUNTIME_ERROR";
    code = stop_info.result.spe_runtime_error;
    break;
  case SPE_RUNTIME_EXCEPTION:
    stop_reason = "SPE_RUNTIME_EXCEPTION";
    code = stop_info.result.spe_runtime_exception;
    break;
  case SPE_RUNTIME_FATAL:
    stop_reason = "SPE_RUNTIME_FATAL";
    code = stop_info.result.spe_runtime_fatal;
    break;
  case SPE_CALLBACK_ERROR:
    stop_reason = "SPE_CALLBACK_ERROR";
    code = stop_info.result.spe_callback_error;
    break;
  case SPE_ISOLATION_ERROR:
    stop_reason = "SPE_ISOLATION_ERROR";
    code = stop_info.result.spe_isolation_error;
    break;
  }

  fprintf(stderr, "SPU stopped (reason %s[%u], code %d, status %x)\n",
	  stop_reason, stop_info.stop_reason, code, stop_info.spu_status);

  if (stop_info.stop_reason != SPE_STOP_AND_SIGNAL) {
    miner->entry = SPE_DEFAULT_ENTRY;
    rb_raise(rb_eRuntimeError, "SPE stopped with %s (%d)", stop_reason, code);
  }

  switch (code) {
  case WORKER_FOUND_NOTHING:
    /* not unexpected */
    break;
  case WORKER_FOUND_SOMETHING:
    fprintf(stderr, "SPU worker found something (%d)\n", miner->params.n);
    break;
  default:
    rb_raise(rb_eRuntimeError, "SPE worker signaled strange code (%d)", code);
  };

  return Qnil;
}

static
void i_free(struct spu_miner *miner)
{
  fprintf(stderr, "i_free (%p)\n", miner);

  spe_context_destroy(miner->spe_context);

  free(miner);
}

static
VALUE i_allocate(VALUE klass)
{
  struct spu_miner *miner;

  if (posix_memalign((void **) &miner, 128, sizeof(*miner)))
    rb_raise(rb_eRuntimeError, "unable to allocate aligned memory");

  miner->spe_context = spe_context_create(0, 0);
  if (miner->spe_context == 0) {
    free(miner);
    rb_raise(rb_eRuntimeError, "failed to create SPE context");
  }

  miner->entry = SPE_DEFAULT_ENTRY;

  return Data_Wrap_Struct(klass, 0, i_free, miner);
}

void Init_spu_miner(void)
{
  VALUE mBitcoin, cSPUMiner;

  mBitcoin = rb_const_get(rb_mKernel, rb_intern("Bitcoin"));
  cSPUMiner = rb_define_class_under(mBitcoin, "SPUMiner", rb_cObject);
  rb_define_alloc_func(cSPUMiner, i_allocate);

  rb_define_method(cSPUMiner, "initialize", m_initialize, 0);
  rb_define_method(cSPUMiner, "run", m_run, 1);
}
