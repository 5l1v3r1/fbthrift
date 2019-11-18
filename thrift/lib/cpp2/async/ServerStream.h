/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Portability.h>
#if FOLLY_HAS_COROUTINES
#include <folly/experimental/coro/AsyncGenerator.h>
#endif // FOLLY_HAS_COROUTINES
#include <folly/Try.h>
#include <thrift/lib/cpp2/async/ServerGeneratorStream.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache {
namespace thrift {

template <typename T>
class ServerStream {
 public:
#if FOLLY_HAS_COROUTINES
  /* implicit */ ServerStream(folly::coro::AsyncGenerator<T&&>&& gen)
      : fn_(detail::ServerGeneratorStream::fromAsyncGenerator(std::move(gen))) {
  }
#endif

  detail::ServerStreamFactory operator()(
      folly::Executor::KeepAlive<> serverExecutor,
      folly::Try<StreamPayload> (*encode)(folly::Try<T>&&)) {
    return fn_(std::move(serverExecutor), encode);
  }

  // convenience operator for tests
  void operator()(
      FirstResponsePayload&& payload,
      StreamClientCallback* callback,
      folly::EventBase* clientEb,
      folly::Executor::KeepAlive<> serverExecutor,
      folly::Try<StreamPayload> (*encode)(folly::Try<T>&&)) {
    fn_(std::move(serverExecutor), encode)(
        std::move(payload), callback, clientEb);
  }

 private:
  detail::ServerStreamFn<T> fn_;
};

template <typename Response, typename StreamElement>
struct ResponseAndServerStream {
  using ResponseType = Response;
  using StreamElementType = StreamElement;

  Response response;
  ServerStream<StreamElement> stream;
};
struct ResponseAndServerStreamFactory {
  folly::IOBufQueue response;
  detail::ServerStreamFactory stream;
};

} // namespace thrift
} // namespace apache
