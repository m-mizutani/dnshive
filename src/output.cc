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

#include <sstream>
#include <iostream>
#include "./output.h"
#include "./flow.h"

namespace dnshive {
  const int Output::ZMQ_IO_THREAD_ = 5;
  Output::Output() : zmq_ctx_(ZMQ_IO_THREAD_), zmq_sock_(NULL) {
  }
  Output::~Output() {
    delete this->zmq_sock_;
  }

  bool Output::enable_zmq(const std::string &addr) {
    std::stringstream ss;
    ss <<  "tcp://" << addr;
    this->zmq_sock_ = new zmq::socket_t (this->zmq_ctx_, ZMQ_PUB);
    try {
      this->zmq_sock_->bind(ss.str().c_str());
    } catch (zmq::error_t &e) {
      this->errmsg_ = e.what();
      return false;
    }
    
    return true;
  }

  const std::string Output::k_event  = "event";
  const std::string Output::k_ts     = "ts";
  const std::string Output::k_type   = "type";
  const std::string Output::k_src    = "src";
  const std::string Output::k_name   = "name";
  const std::string Output::k_res    = "res";
  const std::string Output::k_client = "client";
  const std::string Output::k_server = "server";

  void Output::dns_answer(double ts, const std::string &name, const std::string &type,
                          const std::string &addr, const std::string &dst_addr) {
    msgpack::sbuffer buf;
    msgpack::packer <msgpack::sbuffer> pk (&buf);
    pk.pack_map (6);
    if (type == "A" || type == "AAAA") {
      pk.pack (k_event); pk.pack (std::string("dns:address"));
    } else if (type == "CNAME") {
      pk.pack (k_event); pk.pack (std::string("dns:cname"));
    } else {
      assert(0);
    }
    pk.pack (k_ts); pk.pack (ts);
    pk.pack (k_src); pk.pack (dst_addr);  // Host that sent the query
    pk.pack (k_type); pk.pack (type);
    pk.pack (k_name); pk.pack (name);
    pk.pack (k_res);  pk.pack (addr);
    this->zmq_pub(buf);
  }

  void Output::new_flow(const Flow &f) {
    msgpack::sbuffer buf;
    msgpack::packer <msgpack::sbuffer> pk (&buf);
    pk.pack_map (6);
    pk.pack (k_event); pk.pack (std::string("flow:new"));
    pk.pack (k_ts); pk.pack (f.base_ts());
    pk.pack (k_client); pk.pack (f.c_name());  // Host that sent the query
    pk.pack (std::string("c_port")); pk.pack (f.c_port());
    pk.pack (k_server); pk.pack (f.s_name());
    pk.pack (std::string("s_port")); pk.pack (f.s_port());

    this->zmq_pub(buf);
  }
  void Output::expire_flow(const Flow &f) {
    msgpack::sbuffer buf;
    msgpack::packer <msgpack::sbuffer> pk (&buf);
    pk.pack_map (10);
    pk.pack (k_event); pk.pack (std::string("flow:end"));
    pk.pack (k_ts); pk.pack (f.base_ts());
    pk.pack (k_client); pk.pack (f.c_name());  // Host that sent the query
    pk.pack (std::string("c_size")); pk.pack (f.c_size());
    pk.pack (std::string("c_pkt")); pk.pack (f.c_pkt());
    pk.pack (std::string("c_port")); pk.pack (f.c_port());
    pk.pack (k_server); pk.pack (f.s_name());
    pk.pack (std::string("s_size")); pk.pack (f.s_size());
    pk.pack (std::string("s_pkt")); pk.pack (f.s_pkt());
    pk.pack (std::string("s_port")); pk.pack (f.s_port());

    this->zmq_pub(buf);
  }

  void Output::zmq_pub(const msgpack::sbuffer &buf) {
    if (this->zmq_sock_) {
      zmq::message_t message(buf.size());
      ::memcpy(message.data(), buf.data(), buf.size());
      try {
        this->zmq_sock_->send(message);
      } catch (zmq::error_t &e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }
  
}
