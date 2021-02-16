#include <node_api.h>

#include <stdio.h>
#include <string.h>
#include <fftw3.h>

#include "runtime_native.c"
#include "array_size_t.c"
#include "hash.c"

////////////////////////////////////////////////////////////////////////////////////////////////////
//    simple version (but with alloc/free, no plan reuse)
////////////////////////////////////////////////////////////////////////////////////////////////////
napi_value dft_1d_simple(napi_env env, napi_callback_info info) {
  napi_status status;
  
  napi_value ret_dummy;
  status = napi_create_int32(env, 0, &ret_dummy);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value ret_dummy");
    return ret_dummy;
  }
  
  size_t argc = 4;
  napi_value argv[4];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Failed to parse arguments");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  u8 *data_src;
  size_t data_src_len;
  status = napi_get_buffer_info(env, argv[0], (void**)&data_src, &data_src_len);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid buffer was passed as argument of data_src");
    return ret_dummy;
  }
  
  u8 *data_dst;
  size_t data_dst_len;
  status = napi_get_buffer_info(env, argv[1], (void**)&data_dst, &data_dst_len);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid buffer was passed as argument of data_dst");
    return ret_dummy;
  }
  
  i32 sign;
  status = napi_get_value_int32(env, argv[2], &sign);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of sign");
    return ret_dummy;
  }
  
  i32 flags;
  status = napi_get_value_int32(env, argv[3], &flags);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of flags");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  if (data_src_len != data_dst_len) {
    printf("data_src_len = %ld\n", data_src_len);
    printf("data_dst_len = %ld\n", data_dst_len);
    napi_throw_error(env, NULL, "data_src_len != data_dst_len");
    return ret_dummy;
  }
  if (data_src_len % sizeof(fftw_complex) != 0) {
    printf("data_src_len                        = %ld\n", data_src_len);
    printf("sizeof(fftw_complex)                = %ld\n", sizeof(fftw_complex));
    printf("data_src_len %% sizeof(fftw_complex) = %ld\n", data_src_len % sizeof(fftw_complex));
    napi_throw_error(env, NULL, "data_src_len % sizeof(fftw_complex) != 0");
    return ret_dummy;
  }
  
  fftw_complex *in, *out;
  fftw_plan plan;
  int N = data_src_len / sizeof(fftw_complex);
  
  in  = (fftw_complex*)fftw_malloc(data_src_len);
  if (!in) {
    napi_throw_error(env, NULL, "fftw_malloc fail");
    return ret_dummy;
  }
  out = (fftw_complex*)fftw_malloc(data_dst_len);
  if (!out) {
    napi_throw_error(env, NULL, "fftw_malloc fail");
    return ret_dummy;
  }
  
  plan = fftw_plan_dft_1d(N, in, out, sign, flags);
  memcpy(in, data_src, data_src_len);
  
  fftw_execute(plan);
  fftw_destroy_plan(plan);
  
  memcpy(data_dst, out, data_dst_len);
  
  fftw_free(in);
  fftw_free(out);
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  return ret_dummy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//    
//    
//    complex version
//    
//    
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//    context
////////////////////////////////////////////////////////////////////////////////////////////////////
u32 free_context_counter = 0;
void* free_context_fifo = NULL;
void** alloc_context_hash = NULL;

void free_context_fifo_ensure_init() {
  if (free_context_fifo == NULL) {
    free_context_fifo = array_size_t_alloc(16);
    alloc_context_hash = hash_size_t_alloc();
  }
}

struct Plan_context {
  u32 ctx_idx;
  int N;
  fftw_complex *in, *out;
  fftw_plan plan;
};

struct Plan_context* context_by_id(int id) {
  return (struct Plan_context*)hash_size_t_get(alloc_context_hash, id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
napi_value plan_alloc(napi_env env, napi_callback_info info) {
  napi_status status;
  
  napi_value ret_dummy;
  status = napi_create_int32(env, 0, &ret_dummy);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value ret_dummy");
    return ret_dummy;
  }
  
  size_t argc = 3;
  napi_value argv[3];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Failed to parse arguments");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  i32 N;
  status = napi_get_value_int32(env, argv[0], &N);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of N");
    return ret_dummy;
  }
  
  i32 sign;
  status = napi_get_value_int32(env, argv[1], &sign);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of sign");
    return ret_dummy;
  }
  
  i32 flags;
  status = napi_get_value_int32(env, argv[2], &flags);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of flags");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  struct Plan_context* ctx;
  int ctx_idx;
  if (array_size_t_length_get(free_context_fifo)) {
    ctx = (struct Plan_context*)array_size_t_pop(free_context_fifo);
    ctx_idx = ctx->ctx_idx;
  } else {
    // alloc
    ctx = (struct Plan_context*)malloc(sizeof(struct Plan_context));
    ctx_idx = free_context_counter++;
    ctx->ctx_idx = ctx_idx;
    alloc_context_hash = hash_size_t_set(alloc_context_hash, ctx_idx, (size_t)ctx);
  }
  
  fftw_complex *in, *out;
  in  = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*N);
  if (!in) {
    napi_throw_error(env, NULL, "fftw_malloc fail");
    return ret_dummy;
  }
  out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*N);
  if (!out) {
    napi_throw_error(env, NULL, "fftw_malloc fail");
    return ret_dummy;
  }
  
  ctx->in = in;
  ctx->out = out;
  ctx->N = N;
  ctx->plan = fftw_plan_dft_1d(N, in, out, sign, flags);
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  napi_value ret_idx;
  status = napi_create_int32(env, ctx->ctx_idx, &ret_idx);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value ret_idx");
    return ret_dummy;
  }
  
  return ret_idx;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
napi_value plan_free(napi_env env, napi_callback_info info) {
  napi_status status;
  
  napi_value ret_dummy;
  status = napi_create_int32(env, 0, &ret_dummy);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value ret_dummy");
    return ret_dummy;
  }
  
  size_t argc = 1;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Failed to parse arguments");
    return ret_dummy;
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  i32 ctx_idx;
  status = napi_get_value_int32(env, argv[0], &ctx_idx);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of ctx_idx");
    return ret_dummy;
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //    checks
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  struct Plan_context* ctx = context_by_id(ctx_idx);
  if (!ctx) {
    napi_throw_error(env, NULL, "Invalid ctx_idx");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  fftw_destroy_plan(ctx->plan);
  fftw_free(ctx->in);
  fftw_free(ctx->out);
  ctx->plan = NULL;
  ctx->in   = NULL;
  ctx->out  = NULL;
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  return ret_dummy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
napi_value plan_exec(napi_env env, napi_callback_info info) {
  napi_status status;
  
  napi_value ret_dummy;
  status = napi_create_int32(env, 0, &ret_dummy);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value ret_dummy");
    return ret_dummy;
  }
  
  size_t argc = 3;
  napi_value argv[3];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Failed to parse arguments");
    return ret_dummy;
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  i32 ctx_idx;
  status = napi_get_value_int32(env, argv[0], &ctx_idx);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid i32 was passed as argument of ctx_idx");
    return ret_dummy;
  }
  
  u8 *data_src;
  size_t data_src_len;
  status = napi_get_buffer_info(env, argv[1], (void**)&data_src, &data_src_len);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid buffer was passed as argument of data_src");
    return ret_dummy;
  }
  
  u8 *data_dst;
  size_t data_dst_len;
  status = napi_get_buffer_info(env, argv[2], (void**)&data_dst, &data_dst_len);
  
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Invalid buffer was passed as argument of data_dst");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //    checks
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  struct Plan_context* ctx = context_by_id(ctx_idx);
  if (!ctx) {
    napi_throw_error(env, NULL, "Invalid ctx_idx");
    return ret_dummy;
  }
  if (data_src_len != sizeof(fftw_complex) * ctx->N) {
    printf("data_src_len                  = %ld\n", data_src_len);
    printf("sizeof(fftw_complex)          = %ld\n", sizeof(fftw_complex));
    printf("sizeof(fftw_complex) * ctx->N = %ld\n", sizeof(fftw_complex) * ctx->N);
    
    napi_throw_error(env, NULL, "data_src_len != sizeof(fftw_complex) * ctx->N");
    return ret_dummy;
  }
  if (data_dst_len != sizeof(fftw_complex) * ctx->N) {
    printf("data_dst_len                  = %ld\n", data_dst_len);
    printf("sizeof(fftw_complex)          = %ld\n", sizeof(fftw_complex));
    printf("sizeof(fftw_complex) * ctx->N = %ld\n", sizeof(fftw_complex) * ctx->N);
    
    napi_throw_error(env, NULL, "data_dst_len != sizeof(fftw_complex) * ctx->N");
    return ret_dummy;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  memcpy(ctx->in, data_src, data_src_len);
  fftw_execute(ctx->plan);
  memcpy(data_dst, ctx->out, data_dst_len);
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  return ret_dummy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value fn;
  
  __alloc_init();
  free_context_fifo_ensure_init();
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  status = napi_create_function(env, NULL, 0, dft_1d_simple, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }
  
  status = napi_set_named_property(env, exports, "dft_1d_simple", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  status = napi_create_function(env, NULL, 0, plan_alloc, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }
  
  status = napi_set_named_property(env, exports, "plan_alloc", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  status = napi_create_function(env, NULL, 0, plan_free, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }
  
  status = napi_set_named_property(env, exports, "plan_free", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  status = napi_create_function(env, NULL, 0, plan_exec, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }
  
  status = napi_set_named_property(env, exports, "plan_exec", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)