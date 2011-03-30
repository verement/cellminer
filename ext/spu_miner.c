
# include <stdlib.h>
# include <string.h>
# include <ruby.h>
# include <libspe2.h>

# include "spu/params.h"

struct spu_miner {
  /* must be first for proper alignment */
  volatile struct worker_params params;

  spe_context_ptr_t spe_context;
  uint32_t spe_entry;
  spe_stop_info_t stop_info;
};

static
VALUE run_miner(void *data)
{
  struct spu_miner *miner = data;
  VALUE retval = Qnil;

  spe_context_run(miner->spe_context, &miner->spe_entry, 0,
		  (void *) &miner->params, 0, &miner->stop_info);

  if (miner->stop_info.stop_reason != SPE_STOP_AND_SIGNAL)
    /* restart on next run */
    miner->spe_entry = SPE_DEFAULT_ENTRY;
  else {
    switch (miner->stop_info.result.spe_signal_code) {
    case WORKER_IRQ_SIGNAL:
      miner->spe_entry = SPE_DEFAULT_ENTRY;
      /* fall through */

    case WORKER_FOUND_NOTHING:
      retval = Qfalse;
      break;

    case WORKER_FOUND_SOMETHING:
      retval = rb_str_new((const char *) miner->params.data, 128);
      break;
    }
  }

  return retval;
}

static
void unblock_miner(void *data)
{
  struct spu_miner *miner = data;

  /* signal SPE worker to stop */
  spe_signal_write(miner->spe_context, SPE_SIG_NOTIFY_REG_1, 1);
}

static
void get_stop_reason(const spe_stop_info_t *stop_info,
		     const char **reason, int *code)
{
  switch (stop_info->stop_reason) {
  case SPE_EXIT:
    *reason = "SPE_EXIT";
    *code = stop_info->result.spe_exit_code;
    break;

  case SPE_STOP_AND_SIGNAL:
    *reason = "SPE_STOP_AND_SIGNAL";
    *code = stop_info->result.spe_signal_code;
    break;

  case SPE_RUNTIME_ERROR:
    *reason = "SPE_RUNTIME_ERROR";
    *code = stop_info->result.spe_runtime_error;
    break;

  case SPE_RUNTIME_EXCEPTION:
    *reason = "SPE_RUNTIME_EXCEPTION";
    *code = stop_info->result.spe_runtime_exception;
    break;

  case SPE_RUNTIME_FATAL:
    *reason = "SPE_RUNTIME_FATAL";
    *code = stop_info->result.spe_runtime_fatal;
    break;

  case SPE_CALLBACK_ERROR:
    *reason = "SPE_CALLBACK_ERROR";
    *code = stop_info->result.spe_callback_error;
    break;

  case SPE_ISOLATION_ERROR:
    *reason = "SPE_ISOLATION_ERROR";
    *code = stop_info->result.spe_isolation_error;
    break;

  default:
    *reason = "unknown";
    *code = -1;
  }
}

static
VALUE m_initialize(int argc, VALUE *argv, VALUE self)
{
  struct spu_miner *miner;
  extern spe_program_handle_t spu_worker;

  if (argc < 0 || argc > 1)
    rb_raise(rb_eArgError, "wrong number of arguments (%d for max 1)", argc);

  Data_Get_Struct(self, struct spu_miner, miner);

  if (spe_program_load(miner->spe_context, &spu_worker))
    rb_raise(rb_eRuntimeError, "failed to load SPE program");

  if (argc > 0 && RTEST(argv[0]))
    miner->params.flags |= WORKER_FLAG_DEBUG;

  return Qnil;
}

static
VALUE m_run(VALUE self, VALUE data, VALUE target, VALUE midstate, VALUE hash1)
{
  struct spu_miner *miner;
  VALUE retval;

  Data_Get_Struct(self, struct spu_miner, miner);

  /* prepare parameters */

  StringValue(data);
  StringValue(target);
  StringValue(midstate);
  StringValue(hash1);

  if (RSTRING_LEN(data) != 128)
    rb_raise(rb_eArgError, "data must be 128 bytes");
  if (RSTRING_LEN(target) != 32)
    rb_raise(rb_eArgError, "target must be 32 bytes");
  if (RSTRING_LEN(midstate) != 32)
    rb_raise(rb_eArgError, "midstate must be 32 bytes");
  if (RSTRING_LEN(hash1) != 64)
    rb_raise(rb_eArgError, "hash1 must be 64 bytes");

  memcpy((void *) miner->params.data,     RSTRING_PTR(data),    128);
  memcpy((void *) miner->params.target,   RSTRING_PTR(target),   32);
  memcpy((void *) miner->params.midstate, RSTRING_PTR(midstate), 32);
  memcpy((void *) miner->params.hash1,    RSTRING_PTR(hash1),    64);

  /* unlock the global interpreter lock and run the SPE context */

  retval = rb_thread_blocking_region(run_miner, miner,
				     unblock_miner, miner);

  if (NIL_P(retval)) {
    const char *reason;
    int code;

    get_stop_reason(&miner->stop_info, &reason, &code);
    rb_raise(rb_eRuntimeError, "SPE worker stopped with %s (%d)", reason, code);
  }

  return retval;
}

static
void i_free(struct spu_miner *miner)
{
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

  miner->spe_entry = SPE_DEFAULT_ENTRY;

  miner->params.flags = 0;

  return Data_Wrap_Struct(klass, 0, i_free, miner);
}

void Init_spu_miner(void)
{
  VALUE mBitcoin, cSPUMiner;

  mBitcoin = rb_const_get(rb_mKernel, rb_intern("Bitcoin"));
  cSPUMiner = rb_define_class_under(mBitcoin, "SPUMiner", rb_cObject);
  rb_define_alloc_func(cSPUMiner, i_allocate);

  rb_define_method(cSPUMiner, "initialize", m_initialize, -1);
  rb_define_method(cSPUMiner, "run", m_run, 4);
}
