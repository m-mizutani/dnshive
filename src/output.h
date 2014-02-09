/*-
 * Copyright (c) 2013-2014 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SRC__OUTPUT__
#define SRC__OUTPUT__

#include <string>
#include <zmq.hpp>
#include <msgpack.hpp>

namespace dnshive {
  class Flow;
  class Output {
  private:
    static const std::string k_event;
    static const std::string k_ts;
    static const std::string k_type;
    static const std::string k_src;
    static const std::string k_name;
    static const std::string k_addr;
    static const std::string k_client;
    static const std::string k_server;

    static const int ZMQ_IO_THREAD_ ;
    zmq::context_t zmq_ctx_;
    zmq::socket_t *zmq_sock_;
    std::string errmsg_;
    void zmq_pub(const msgpack::sbuffer &buf);

  public:
    Output();
    ~Output();
    bool enable_zmq (const std::string &addr);
    const std::string &errmsg() const { return this->errmsg_; }
    void dns_answer(double ts, const std::string &name, const std::string &type,
                    const std::string &addr, const std::string &dst_addr);
    void new_flow(const Flow &f);
    void expire_flow(const Flow &f);
  };
}

#endif  // SRC__OUTPUT__
