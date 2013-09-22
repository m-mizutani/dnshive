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

#include <msgpack.hpp>
#include "proc.h"
#include "debug.h"

namespace dnshive {
  const std::string DnsFwdDB::REDIS_HOST_ = "localhost";
  const int DnsFwdDB::REDIS_PORT_ = 6379;
  const bool DBG = true;

  DnsFwdDB::DnsFwdDB () {
    this->redis_ctx_ = redisConnect(REDIS_HOST_.c_str (), REDIS_PORT_);
    if (this->redis_ctx_ == NULL) {
      printf ("Critical Error in hiredis\n");
    } else if (this->redis_ctx_->err) {
      printf("Error: %s\n", this->redis_ctx_->errstr);
      redisFree (this->redis_ctx_);
      this->redis_ctx_ = NULL;
    }
  }
  DnsFwdDB::~DnsFwdDB () {
    if (this->redis_ctx_) {
      redisFree (this->redis_ctx_);
    }
  }

  const std::string * DnsFwdDB::lookup (u_int32_t * v4addr) {
    auto it = this->rev_map_.find (*v4addr);
    if (it == this->rev_map_.end ()) {
      return NULL;
    } else {
      return &(it->second);
    }
  }

  void DnsFwdDB::recv (swarm::ev_id eid, const  swarm::Property &p) {
    debug (DBG, "pkt recv");
    static const std::string k_ts   = "ts";
    static const std::string k_type = "type";
    static const std::string k_src  = "src";
    static const std::string k_name = "name";
    
    for (size_t i = 0; i < p.param ("dns.an_name")->size (); i++) {
      std::string name = p.param ("dns.an_name")->repr (i);
      std::string type = p.param ("dns.an_type")->repr (i);
      std::string addr;

      addr = p.param ("dns.an_data")->repr (i);
      debug (DBG, "%s (%s) %s", name.c_str (), type.c_str (), addr.c_str ());

      void * ptr = p.param ("dns.an_data")->get (NULL, i);

      if (ptr) {
        // register to in-memory DB
        u_int32_t * a = static_cast<u_int32_t*> (ptr);
        this->rev_map_.insert (std::make_pair (*a, name));

        // register to redis
        if (this->redis_ctx_) {
          msgpack::sbuffer buf;
          msgpack::packer <msgpack::sbuffer> pk (&buf);

          pk.pack_map (4);
          pk.pack (k_ts); pk.pack (p.ts ());
          pk.pack (k_src); pk.pack (p.dst_addr ());  // Host that sent the query
          pk.pack (k_type); pk.pack (type);
          pk.pack (k_name); pk.pack (name);

          const std::string &data = p.param ("dns.an_data")->repr (i);
          void *com = redisCommand(this->redis_ctx_, 
                                   "lpush %s %b", data.c_str (), buf.data (), buf.size ());
          freeReplyObject (com);
        }

      }
    }
    return;
  }


  void IPFlow::set_db (DnsFwdDB *db) {
    this->db_ = db;
  }

  void IPFlow::recv (swarm::ev_id eid, const  swarm::Property &p) {
    std::string s_tmp, d_tmp;
    const std::string *src, *dst;
    void *s_addr = p.param ("ipv4.src")->get ();
    void *d_addr = p.param ("ipv4.dst")->get ();
    if (!s_addr || !d_addr) {
      return;
    }

    if (NULL == (src = this->db_->lookup (static_cast<u_int32_t*>(s_addr)))) {
      s_tmp = p.param("ipv4.src")->repr ();
      src = &s_tmp;
    }
    if (NULL == (dst = this->db_->lookup (static_cast<u_int32_t*>(d_addr)))) {
      d_tmp = p.param("ipv4.dst")->repr ();
      dst = &d_tmp;
    }

    // printf ("%s -> %s\n", src->c_str (), dst->c_str ());
  }

}  // namespace dnshive
