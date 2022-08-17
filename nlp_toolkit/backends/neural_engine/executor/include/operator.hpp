//  Copyright (c) 2021 Intel Corporation
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef ENGINE_EXECUTOR_INCLUDE_OPERATOR_HPP_
#define ENGINE_EXECUTOR_INCLUDE_OPERATOR_HPP_

#include <algorithm>
#include <string>
#include <vector>

#include "common.hpp"
#include "operator_registry.hpp"
#include "tensor.hpp"

namespace executor {

/**
 * @brief An interface for the units of computation which can be composed into a
 *        Model.
 *
 * Operator%s must implement a Forward function, in which they take their input
 * (input) Tensor%s (if any) and compute their output Tensor%s (if any).
 */

enum KERNEL_TYPE {
  Unsupported = 0,
  Dense = 1,
  Sparse = 2,
  SparseLib = 3
};

class Operator {
 public:
  explicit Operator(const OperatorConfig& conf) : operator_conf_(conf) {
    name_ = operator_conf_.name();
    type_ = operator_conf_.type();
  }

  virtual ~Operator() {}

  virtual void Prepare(const vector<Tensor*>& input,
                       const vector<Tensor*>& output) {}

  // use Reshape to calculate the output shape from input tensor
  virtual void Reshape(const vector<Tensor*>& input,
                       const vector<Tensor*>& output) = 0;

  virtual void Forward(const vector<Tensor*>& input,
                       const vector<Tensor*>& output) = 0;

  inline void unref_tensors(const vector<Tensor*>& input) {
    for (size_t i = 0; i < input.size(); ++i) {
      auto status = input[i]->unref_data();
      // (TODO) maybe check the tensors
    }
  }

  inline const string& name() const { return name_; }
  inline const string& type() const { return type_; }
  const OperatorConfig& operator_conf() const { return operator_conf_; }
  // dispatch kernel may need to do reshape and receive config, like InnerProduct to Convolution
  inline void set_dispatch_from_type(const string& type) { dispatch_from_ = type; }
  inline void set_dispatch_config(const vector<string>& config) { dispatch_config_ = config; }
  inline void set_do_shape_infer(const bool& do_shape_infer) { do_shape_infer_ = do_shape_infer; }
  inline const bool& do_shape_infer() const { return do_shape_infer_; }
  inline const bool& monopolize_dispatcher() const { return monopolize_dispatcher_; }
  // for profiling
  inline void set_post_op(const string& post_op) { post_op_ = post_op; }
  inline const string& post_op() const { return post_op_; }
  inline void set_latency(const float latency) { latency_.emplace_back(latency); }
  inline const vector<float>& latency() const { return latency_; }
  inline void set_enable_sparse(const bool enable_sparse) { enable_sparse_ = enable_sparse; }
  inline const float& enable_sparse() const { return enable_sparse_; }
  inline const KERNEL_TYPE& kernel_type() const { return kernel_type_; }
  inline const float& weight_zero_ratio() const { return weight_zero_ratio_; }
  inline void set_weight_shape(const vector<int64_t>& weight_shape) { weight_shape_ = weight_shape; }
  inline const vector<int64_t>& weight_shape() const { return weight_shape_; }
  inline void set_table_id(const string& table_id) { table_id_ = table_id; }
  inline const string& table_id() const { return table_id_; }
  inline void set_perf_ratio_id(const string& perf_ratio_id) { perf_ratio_id_ = perf_ratio_id; }
  inline const string& perf_ratio_id() const { return perf_ratio_id_; }

 protected:
  /** The conf that stores the operator configurations */
  string name_;
  string type_;
  OperatorConfig operator_conf_;
  string dispatch_from_;
  vector<string> dispatch_config_;
  bool do_shape_infer_ = false;
  bool monopolize_dispatcher_ = false;
  // for profiling
  string post_op_;
  vector<float> latency_;
  float enable_sparse_ = false;
  KERNEL_TYPE kernel_type_ = Unsupported;
  float weight_zero_ratio_ = 0.0;
  vector<int64_t> weight_shape_;
  string table_id_;
  string perf_ratio_id_;
};  // class Operator

}  // namespace executor

#endif  // ENGINE_EXECUTOR_INCLUDE_OPERATOR_HPP_