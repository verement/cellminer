
# include <ruby.h>
# include <libspe2.h>

static
void load_run(spe_program_handle_t *program)
{
  spe_context_ptr_t spe;
  uint32_t entry = SPE_DEFAULT_ENTRY;
  spe_stop_info_t stop_info;

  spe = spe_context_create(0, 0);
  if (spe == 0)
    rb_raise(rb_eRuntimeError, "failed to create SPE context");

  if (spe_program_load(spe, program))
    rb_raise(rb_eRuntimeError, "failed to load SPE program");

  if (spe_context_run(spe, &entry, 0, 0, 0, &stop_info) < 0)
    rb_raise(rb_eRuntimeError, "failed to run SPE context");

  if (spe_context_destroy(spe))
    rb_raise(rb_eRuntimeError, "failed to destroy SPE context");
}

void Init_spu_miner(void)
{
  extern spe_program_handle_t spu_worker;

  load_run(&spu_worker);
}
