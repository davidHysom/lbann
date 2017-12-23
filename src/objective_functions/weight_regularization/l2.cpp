////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2016, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
// Written by the LBANN Research Team (B. Van Essen, et al.) listed in
// the CONTRIBUTORS file. <lbann-dev@llnl.gov>
//
// LLNL-CODE-697807.
// All rights reserved.
//
// This file is part of LBANN: Livermore Big Artificial Neural Network
// Toolkit. For details, see http://software.llnl.gov/LBANN or
// https://github.com/LLNL/LBANN.
//
// Licensed under the Apache License, Version 2.0 (the "Licensee"); you
// may not use this file except in compliance with the License.  You may
// obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the license.
////////////////////////////////////////////////////////////////////////////////

#include "lbann/objective_functions/weight_regularization/l2.hpp"
#include "lbann/models/model.hpp"
#ifdef __LIB_CUDNN
#include "lbann/utils/cublas_wrapper.hpp"
#endif // __LIB_CUDNN


namespace lbann {

void l2_weight_regularization::setup(model& m) {
  objective_function_term::setup(m);

  // Check that term has no layer pointers
  if (!m_layers.empty()) {
    std::stringstream err;
    err << __FILE__ << " " << __LINE__ << " :: "
        << "attempted to setup L2 weight regularization with layer pointers";
    throw lbann_exception(err.str());
  }

  // Add all weights in model if no weights pointers are provided
  if (m_weights.empty()) {
    for (weights* w : m.get_weights()) {
      if (w->get_optimizer() != nullptr) {
        m_weights.push_back(w);
      }
    }
  }

}

EvalType l2_weight_regularization::local_squared_l2_norm(const Mat& mat) const {
  const El::Int height = mat.Height();
  const El::Int width = mat.Width();
  const El::Int ldim = mat.LDim();
  const DataType* __restrict__ buf = mat.LockedBuffer();
  EvalType sqsum = EvalType(0);
  // Check if data is contiguous.
  if (ldim == height) {
    const El::Int size = height*width;
    #pragma omp parallel for reduction(+:sqsum)
    for (El::Int i = 0; i < size; ++i) {
      const EvalType val = buf[i];
      sqsum += val * val;
    }
  } else {
    #pragma omp parallel for reduction(+:sqsum) collapse(2)
    for (El::Int j = 0; j < width; ++j) {
      for (El::Int i = 0; i < height; ++i) {
        const EvalType val = buf[i + j*ldim];
        sqsum += val * val;
      }
    }
  }
  return sqsum;
}

EvalType l2_weight_regularization::evaluate() {
  if (m_scale_factor == EvalType(0)) { return EvalType(0); }
  auto value = EvalType(0);
  for (weights* w : m_weights) {
    cudnn::cudnn_manager* cudnn = w->get_cudnn_manager();
    if (cudnn != nullptr) {
#ifdef __LIB_CUDNN
      CHECK_CUDA(cudaSetDevice(cudnn->get_gpu(0)));
      EvalType norm = cublas::nrm2(cudnn->get_cublas_handle(0),
                                   w->get_height() * w->get_width(),
                                   w->get_values_gpu()[0], 1);
      value += norm * norm;
#endif // __LIB_CUDNN
    } else {
      // Further optimization: Can batch allreduces on the same communicator.
      const AbsDistMat& values = w->get_values();
      EvalType local_norm = local_squared_l2_norm(values.LockedMatrix());
      value += get_comm().allreduce(local_norm, values.DistComm());
    }
  }
  return m_scale_factor * value;
}

void l2_weight_regularization::differentiate() {
  if (m_scale_factor == EvalType(0)) { return; }
  for (weights* w : m_weights) {
    optimizer* opt = w->get_optimizer();
    if (w->get_cudnn_manager() != nullptr) {
#ifdef __LIB_CUDNN
      std::vector<DataType*> values_d = w->get_values_gpu();
      opt->add_to_gradient_gpu(values_d, 2 * m_scale_factor);
#endif // __LIB_CUDNN
    } else {
      opt->add_to_gradient(w->get_values(), 2 * m_scale_factor);
    }
  }
}

}  // namespace lbann
