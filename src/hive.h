/*-
 * Copyright (c) 2013 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>
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

#ifndef __LIB_DNSHIVE_H__
#define __LIB_DNSHIVE_H__

#include <vector>
#include <swarm/swarm.h>

namespace dnshive {
  class DnsDB;
  class FlowHandler;
  class Output;

  class Handler {
  public:
    virtual void flow (const std::string &src, const std::string &dst,
                       const swarm::Property &prop) = 0;
  };

  class Hive {
  private:
    swarm::NetDec *nd_;
    DnsDB *dns_db_;
    FlowHandler *ip_flow_;
    std::string errmsg_;
    bool quiet_;
    Output *output_;

  public:
    Hive ();
    ~Hive ();
    bool capture (const std::string &arg, bool dev=true);
    bool enable_redis_db (const std::string &host, const std::string &port,
                          const std::string &db);
    bool enable_zmq (const std::string &addr);
    bool enable_msgpack_ofs (const std::string &addr);
    void set_handler (Handler *hdlr);
    void unset_handler ();
    void enable_quiet ();
    const std::string& errmsg () const;
  };
}

#endif
