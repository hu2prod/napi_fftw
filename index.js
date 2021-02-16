mod = require('./build/Release/module');
module.exports = {};
Object.assign(module.exports, mod);

// https://github.com/FFTW/fftw3/blob/master/api/fftw3.h

module.exports.FFTW_FORWARD = -1
module.exports.FFTW_BACKWARD = +1


module.exports.FFTW_NO_TIMELIMIT = -1


module.exports.FFTW_ESTIMATE = +1
module.exports.FFTW_MEASURE = +1
module.exports.FFTW_PATIENT = +1
module.exports.FFTW_EXHAUSTIVE = +1
module.exports.FFTW_WISDOM_ONLY = +1
module.exports.FFTW_DESTROY_INPUT = +1
module.exports.FFTW_PRESERVE_INPUT = +1
module.exports.FFTW_UNALIGNED = +1


module.exports.FFTW_MEASURE         = (0)
module.exports.FFTW_DESTROY_INPUT   = (1 << 0)
module.exports.FFTW_UNALIGNED       = (1 << 1)
module.exports.FFTW_CONSERVE_MEMORY = (1 << 2)
module.exports.FFTW_EXHAUSTIVE      = (1 << 3) /* NO_EXHAUSTIVE is default */
module.exports.FFTW_PRESERVE_INPUT  = (1 << 4) /* cancels FFTW_DESTROY_INPUT */
module.exports.FFTW_PATIENT         = (1 << 5) /* IMPATIENT is default */
module.exports.FFTW_ESTIMATE        = (1 << 6)
module.exports.FFTW_WISDOM_ONLY     = (1 << 21)


/* undocumented beyond-guru flags */
module.exports.FFTW_ESTIMATE_PATIENT        = (1 << 7)
module.exports.FFTW_BELIEVE_PCOST           = (1 << 8)
module.exports.FFTW_NO_DFT_R2HC             = (1 << 9)
module.exports.FFTW_NO_NONTHREADED          = (1 << 10)
module.exports.FFTW_NO_BUFFERING            = (1 << 11)
module.exports.FFTW_NO_INDIRECT_OP          = (1 << 12)
module.exports.FFTW_ALLOW_LARGE_GENERIC     = (1 << 13) /* NO_LARGE_GENERIC is default */
module.exports.FFTW_NO_RANK_SPLITS          = (1 << 14)
module.exports.FFTW_NO_VRANK_SPLITS         = (1 << 15)
module.exports.FFTW_NO_VRECURSE             = (1 << 16)
module.exports.FFTW_NO_SIMD                 = (1 << 17)
module.exports.FFTW_NO_SLOW                 = (1 << 18)
module.exports.FFTW_NO_FIXED_RADIX_LARGE_N  = (1 << 19)
module.exports.FFTW_ALLOW_PRUNING           = (1 << 20)


// TODO fftw_set_timelimit
