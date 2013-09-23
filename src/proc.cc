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

#include <string.h>
#include <msgpack.hpp>
#include <iostream>

#include "proc.h"
#include "debug.h"

namespace dnshive {
  const std::string DnsFwdDB::REDIS_HOST_ = "localhost";
  const int DnsFwdDB::REDIS_PORT_ = 6379;
  const bool DBG = false;

  DnsFwdDB::DnsFwdDB () : redis_ctx_(NULL) {
  }
  DnsFwdDB::~DnsFwdDB () {
    if (this->redis_ctx_) {
      redisFree (this->redis_ctx_);
    }
  }

  bool DnsFwdDB::enable_redis_db (const std::string &host, const std::string &port,
                                  const std::string &db) {
    bool rc = true;

    char *e;
    int p = ::strtol (port.c_str (), &e, 0);

    if (*e != '\0') {
      this->errmsg_ = "port number \"" + port + "\" should be digit";
    } else {
      // connect to redis server
      this->redis_ctx_ = redisConnect(host.c_str (), p);
      if (this->redis_ctx_ == NULL) {
        this->errmsg_ = "Critical Error in hiredis";
        rc = false;
      } else if (this->redis_ctx_->err) {
        this->errmsg_ = this->redis_ctx_->errstr;
        rc = false;
      } else {
        // select redis DB
        struct redisReply * com = static_cast<struct redisReply *>
          (redisCommand(this->redis_ctx_, "select %s", db.c_str ()));
        if (NULL == com) {
          this->errmsg_ = "Critical error in hiredis to select DB \"" + db + "\"";
          rc = false;
        } else if (com->type == REDIS_REPLY_ERROR) {
          this->errmsg_.assign (com->str, com->len);
          rc = false;
        }

        if (com) {
          freeReplyObject (com);
        }
      }
    }

    if (!rc && this->redis_ctx_) {
      redisFree (this->redis_ctx_);
      this->redis_ctx_ = NULL;
    }

    return rc;
  }

  int DnsFwdDB::load_redis_db () {
    struct redisReply * com = static_cast<struct redisReply *>
      (redisCommand(this->redis_ctx_, "keys *"));

    std::deque <std::string> key_list;
    std::string key, name;
    if (com->type == REDIS_REPLY_ARRAY) {
      for (int i = 0; i < com->elements; i++) {
        key.assign (com->element[i]->str, com->element[i]->len);
        key_list.push_back (key);
      }
    } else {
      this->errmsg_ = "invalid data type (";
      this->errmsg_ += com->type;
      this->errmsg_ += ")";
      return -1;
    }
    freeReplyObject (com);

    int rc = 0;
    for (auto it = key_list.begin (); it != key_list.end (); it++) {
      auto rep = static_cast<struct redisReply *>
        (redisCommand(this->redis_ctx_, "lrange %b -1 -1", key.data (), key.size ()));

      if (rep->element > 0) {
        msgpack::unpacked msg;
        msgpack::unpack(&msg, rep->element[0]->str, rep->element[0]->len);
        msgpack::object obj = msg.get ();

        std::string t;

        if(obj.via.map.size != 0) {
          msgpack::object_kv* p(obj.via.map.ptr);
          for(msgpack::object_kv* const pend(obj.via.map.ptr + obj.via.map.size);
              p < pend; ++p) {
            std::string k;
            p->key.convert (&k);
            if (k == "name") {
              p->val.convert (&name);
              
              // std::cout << k << "=>" << name;
              this->rev_map_.insert (std::make_pair (key, name));
              rc++;
              break;
            }
          }
        }
      }
    }

    return rc;
  }


  const std::string * DnsFwdDB::lookup (void * addr, size_t len) {
    std::string key (static_cast<char *>(addr), len);
    auto it = this->rev_map_.find (key);
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

      size_t len;
      void * ptr = p.param ("dns.an_data")->get (&len, i);

      if (ptr && (type == "A" || type == "AAAA")) {
        // register to in-memory DB
        std::string key (static_cast<char*>(ptr), len);
        this->rev_map_.insert (std::make_pair (key, name));

        // register to redis
        if (this->redis_ctx_) {
          msgpack::sbuffer buf;
          msgpack::packer <msgpack::sbuffer> pk (&buf);

          pk.pack_map (4);
          pk.pack (k_ts); pk.pack (p.ts ());
          pk.pack (k_src); pk.pack (p.dst_addr ());  // Host that sent the query
          pk.pack (k_type); pk.pack (type);
          pk.pack (k_name); pk.pack (name);

          void *com = redisCommand(this->redis_ctx_, 
                                   "lpush %b %b", ptr, len, buf.data (), buf.size ());

          freeReplyObject (com);
        }
      }
    }
    return;
  }

  const std::string &DnsFwdDB::errmsg () const {
    return this->errmsg_;
  }


  IPFlow::IPFlow () : hdlr_(NULL) {
  }
  IPFlow::~IPFlow () {
  }

  void IPFlow::set_db (DnsFwdDB *db) {
    this->db_ = db;
  }

  void IPFlow::recv (swarm::ev_id eid, const  swarm::Property &p) {
    if (this->hdlr_) {
      std::string s_tmp, d_tmp;
      const std::string *src, *dst;
      size_t src_len, dst_len;
      void *s_addr = p.src_addr (&src_len);
      void *d_addr = p.dst_addr (&dst_len);
      if (!s_addr || !d_addr) {
        return;
      }

      if (NULL == (src = this->db_->lookup (s_addr, src_len))) {
        s_tmp = p.src_addr ();
        src = &s_tmp;
      }
      if (NULL == (dst = this->db_->lookup (d_addr, dst_len))) {
        d_tmp = p.dst_addr ();
        dst = &d_tmp;
      }

      this->hdlr_->flow (*src, *dst, p);
    }
  }


  void IPFlow::set_handler (dnshive::Handler *hdlr) {
    this->hdlr_ = hdlr;
  }
  void IPFlow::unset_handler () {
    this->hdlr_ = NULL;
  }


}  // namespace dnshive
