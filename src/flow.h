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

#ifndef SRC__FLOW__
#define SRC__FLOW__

#include <swarm/swarm.h>
#include "./lru-hash.h"

namespace dnshive {
  class DnsDB;
  class Output;

  class Flow : public LRUHash::Node {
  private:
    void *key_;
    size_t len_;
    uint64_t hash_;
    time_t base_ts_;
    time_t last_ts_;
    double syn_ts_;
    std::string proto_;
    swarm::FlowDir dir_;
    // Source of first packet is client
    
    size_t c_pkt_;
    size_t c_size_;
    std::string c_name_;
    int c_port_;
    // Destination of first packet is server
    size_t s_pkt_;
    size_t s_size_;
    std::string s_name_;
    int s_port_;

  public:
    Flow(const swarm::Property &p, const std::string &src, const std::string &dst);
    ~Flow();
    uint64_t hash();
    bool match(const void *key, size_t len);
    void update(const swarm::Property &p);
    time_t base_ts() const { return this->base_ts_; }
    time_t last_ts() const { return this->last_ts_; }
    time_t duration() const { return (this->last_ts_ - this->base_ts_); }

    size_t c_pkt() const { return this->c_pkt_; }
    size_t c_size() const { return this->c_size_; }
    const std::string &c_name() const { return this->c_name_; }
    int c_port() const { return this->c_port_; }
    size_t s_pkt() const { return this->s_pkt_; }
    size_t s_size() const { return this->s_size_; }
    const std::string &s_name() const { return this->s_name_; }
    int s_port() const { return this->s_port_; }
  };

  class FlowHandler : public swarm::Handler {
  private:
    DnsDB * db_;
    LRUHash flow_table_;
    Output *output_;
    time_t last_ts_;

  public:
    FlowHandler ();
    virtual ~FlowHandler ();
    void set_db (DnsDB *db);
    void set_output(Output *output);
    void recv (swarm::ev_id eid, const  swarm::Property &p);

  };
}

#endif  // SRC__FLOW__
