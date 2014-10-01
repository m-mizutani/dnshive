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

#ifndef SRC__DNSDB__
#define SRC__DNSDB__

#include <swarm.h>
#include <hiredis/hiredis.h>
#include "./hive.h"

namespace dnshive {
  class Output;

  class DnsDB : public swarm::Handler {
  private:
    static const std::string REDIS_HOST_;
    static const int REDIS_PORT_;
    static const int ZMQ_IO_THREAT_;

    std::map <std::string, std::string> cname_map_;
    std::map <std::string, std::string> rev_map_;
    redisContext *redis_ctx_;
    Output *output_;

    std::string errmsg_;

    void insert(const std::string &name, const std::string &type,
                const std::string &addr, void *ptr, size_t len,
                double ts, const std::string &dst_addr);
  public:
    DnsDB ();
    ~DnsDB ();
    const std::string * lookup (void * addr, size_t len);
    void recv (swarm::ev_id eid, const  swarm::Property &p);
    bool enable_redis_db (const std::string &host, const std::string &port,
                          const std::string &db);
    int load_redis_db ();
    void set_output(Output *output);
    const std::string &errmsg () const;
  };

}  // namespace dnshive

#endif  // SRC__DNSDB__

