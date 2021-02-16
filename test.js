#!/usr/bin/env node
napi_fftw3 = require('./index');
assert = require('assert');

// не работает т.е. big endian
// out_arr = new Float64Array(out_buf)

in_buf  = Buffer.alloc(4*16);
out_buf = Buffer.alloc(4*16);
out_rev_buf = Buffer.alloc(4*16);

offset = 0
in_buf.writeDoubleLE(1, offset); offset += 16;
in_buf.writeDoubleLE(1, offset); offset += 16;
in_buf.writeDoubleLE(1, offset); offset += 16;
in_buf.writeDoubleLE(1, offset); offset += 16;

buf_to_f64_list = function(buf) {
  var out_list = [];
  var offset = 0;
  while(offset < buf.length) {
    out_list.push(buf.readDoubleLE(offset));
    offset += 16;
  }
  return out_list;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
napi_fftw3.dft_1d_simple(in_buf, out_buf, napi_fftw3.FFTW_FORWARD, napi_fftw3.FFTW_ESTIMATE);

out_arr = buf_to_f64_list(out_buf);
console.log("out_arr", out_arr);

assert(out_arr[0] == 4)
assert(out_arr[1] == 0)
assert(out_arr[2] == 0)
assert(out_arr[3] == 0)

////////////////////////////////////////////////////////////////////////////////////////////////////
ctx = napi_fftw3.plan_alloc(4, napi_fftw3.FFTW_FORWARD, napi_fftw3.FFTW_ESTIMATE);

napi_fftw3.plan_exec(ctx, in_buf, out_buf);

napi_fftw3.plan_free(ctx);

out_arr = buf_to_f64_list(out_buf);
console.log("out_arr", out_arr);

assert(out_arr[0] == 4)
assert(out_arr[1] == 0)
assert(out_arr[2] == 0)
assert(out_arr[3] == 0)


////////////////////////////////////////////////////////////////////////////////////////////////////

napi_fftw3.dft_1d_simple(in_buf, out_buf, napi_fftw3.FFTW_FORWARD, napi_fftw3.FFTW_ESTIMATE);
napi_fftw3.dft_1d_simple(out_buf, out_rev_buf, napi_fftw3.FFTW_BACKWARD, napi_fftw3.FFTW_ESTIMATE);


out_rev_arr = buf_to_f64_list(out_rev_buf);
console.log("out_rev_arr", out_rev_arr);

// denormalized signal
assert(out_rev_arr[0] == 4)
assert(out_rev_arr[1] == 4)
assert(out_rev_arr[2] == 4)
assert(out_rev_arr[3] == 4)

////////////////////////////////////////////////////////////////////////////////////////////////////
