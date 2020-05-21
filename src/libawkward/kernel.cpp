#include "awkward/kernel.h"
#include "awkward/util.h"
#include "awkward/cpu-kernels/operations.h"
#include "awkward/common_utils.h"
#include "awkward/cpu-kernels/getitem.h"
#include "awkward/cpu-kernels/identities.h"
#include "awkward/cpu-kernels/reducers.h"

#ifdef ENABLE_GPU_KERNELS
  #include "awkward/gpu-kernels/identities.h"
#endif

namespace kernel {
template<>
ERROR new_identities<int32_t>(int64_t memory_loc,
                              int32_t *toptr,
                              int64_t length) {
  if (memory_loc < 0)
    return awkward_new_identities32(toptr, length);
  #ifdef ENABLE_GPU_KERNELS
  else {
    return awkward_gpu_new_identities32(memory_loc, toptr, length);
  }
  #endif
}

template<>
ERROR new_identities<int64_t>(int64_t memory_loc,
                              int64_t *toptr,
                              int64_t length) {
  if (memory_loc < 0)
    return awkward_new_identities64(toptr, length);
  #ifdef ENABLE_GPU_KERNELS
  else {
    return awkward_gpu_new_identities64(memory_loc, toptr, length);
  }
  #endif
}
}



