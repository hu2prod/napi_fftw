# napi_fftw3
## install requirements

    apt-get install -y fftw3-dev fftw3

## use

    napi_fftw3 = require("napi_fftw3")
    in_buf  = Buffer.alloc(4*16);
    out_buf = Buffer.alloc(4*16);
    out_rev_buf = Buffer.alloc(4*16);
    
    offset = 0
    in_buf.writeDoubleLE(1, offset); offset += 16;
    in_buf.writeDoubleLE(1, offset); offset += 16;
    in_buf.writeDoubleLE(1, offset); offset += 16;
    in_buf.writeDoubleLE(1, offset); offset += 16;
    
    // simple
    // FFT
    napi_fftw3.dft_1d_simple(in_buf, out_buf, napi_fftw3.FFTW_FORWARD, napi_fftw3.FFTW_ESTIMATE);
    // iFFT (denormalized)
    napi_fftw3.dft_1d_simple(out_buf, in_buf, napi_fftw3.FFTW_BACKWARD, napi_fftw3.FFTW_ESTIMATE);
    
    // faster with plan
    ctx = napi_fftw3.plan_alloc(4, napi_fftw3.FFTW_FORWARD, napi_fftw3.FFTW_ESTIMATE);
    napi_fftw3.plan_exec(ctx, in_buf, out_buf);
    napi_fftw3.plan_free(ctx);
    
    // NOT works, because BigEndian
    // out_arr = new Float64Array(out_buf)
    
    // works
    buf_to_f64_list = function(buf) {
      var out_list = [];
      var offset = 0;
      while(offset < buf.length) {
        out_list.push(buf.readDoubleLE(offset));
        offset += 16;
      }
      return out_list;
    }
