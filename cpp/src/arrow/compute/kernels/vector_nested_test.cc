// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <gtest/gtest.h>

#include "arrow/chunked_array.h"
#include "arrow/compute/api.h"
#include "arrow/compute/kernels/test_util.h"
#include "arrow/result.h"
#include "arrow/testing/gtest_util.h"

namespace arrow {
namespace compute {

TEST(TestVectorNested, ListFlatten) {
  for (auto ty : {list(int16()), large_list(int16())}) {
    auto input = ArrayFromJSON(ty, "[[0, null, 1], null, [2, 3], []]");
    auto expected = ArrayFromJSON(int16(), "[0, null, 1, 2, 3]");
    CheckVectorUnary("list_flatten", input, expected);

    // Construct a list with a non-empty null slot
    TweakValidityBit(input, 0, false);
    expected = ArrayFromJSON(int16(), "[2, 3]");
    CheckVectorUnary("list_flatten", input, expected);
  }
}

TEST(TestVectorNested, ListFlattenChunkedArray) {
  for (auto ty : {list(int16()), large_list(int16())}) {
    auto input = ChunkedArrayFromJSON(ty, {"[[0, null, 1], null]", "[[2, 3], []]"});
    auto expected = ChunkedArrayFromJSON(int16(), {"[0, null, 1]", "[2, 3]"});
    CheckVectorUnary("list_flatten", input, expected);

    input = ChunkedArrayFromJSON(ty, {});
    expected = ChunkedArrayFromJSON(int16(), {});
    CheckVectorUnary("list_flatten", input, expected);
  }
}

TEST(TestVectorNested, ListParentIndices) {
  for (auto ty : {list(int16()), large_list(int16())}) {
    auto input = ArrayFromJSON(ty, "[[0, null, 1], null, [2, 3], [], [4, 5]]");

    auto out_ty = ty->id() == Type::LIST ? int32() : int64();
    auto expected = ArrayFromJSON(out_ty, "[0, 0, 0, 2, 2, 4, 4]");
    CheckVectorUnary("list_parent_indices", input, expected);
  }

  // Construct a list with a non-empty null slot
  auto input = ArrayFromJSON(list(int16()), "[[0, null, 1], [0, 0], [2, 3], [], [4, 5]]");
  TweakValidityBit(input, 1, false);
  auto expected = ArrayFromJSON(int32(), "[0, 0, 0, 1, 1, 2, 2, 4, 4]");
  CheckVectorUnary("list_parent_indices", input, expected);
}

TEST(TestVectorNested, ListParentIndicesChunkedArray) {
  for (auto ty : {list(int16()), large_list(int16())}) {
    auto input =
        ChunkedArrayFromJSON(ty, {"[[0, null, 1], null]", "[[2, 3], [], [4, 5]]"});

    auto out_ty = ty->id() == Type::LIST ? int32() : int64();
    auto expected = ChunkedArrayFromJSON(out_ty, {"[0, 0, 0]", "[2, 2, 4, 4]"});
    CheckVectorUnary("list_parent_indices", input, expected);

    input = ChunkedArrayFromJSON(ty, {});
    expected = ChunkedArrayFromJSON(out_ty, {});
    CheckVectorUnary("list_parent_indices", input, expected);
  }
}

}  // namespace compute
}  // namespace arrow
