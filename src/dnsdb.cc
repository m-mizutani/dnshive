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
#include <sstream>

#include "./output.h"
#include "./dnsdb.h"
#include "./debug.h"

namespace dnshive {
  const std::string DnsDB::REDIS_HOST_ = "localhost";
  const int DnsDB::REDIS_PORT_ = 6379;
  const bool DBG = false;

  DnsDB::DnsDB () : redis_ctx_(NULL), output_(NULL) {
  }
  DnsDB::~DnsDB () {
    if (this->redis_ctx_) {
      redisFree (this->redis_ctx_);
    }
  }

  bool DnsDB::enable_redis_db (const std::string &host, const std::string &port,
                                  const std::string &db) {
    bool rc = true;

    char *e;
    int p = ::strtol (port.c_str (), &e, 0);

    if (*e != '\0') {
      this->errmsg_ = "port number \"" + port + "\" should be digit";
      rc = false;
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

  int DnsDB::load_redis_db () {
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
              p && p < pend; ++p) {
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


  const std::string *DnsDB::lookup (void * addr, size_t len) {
    std::string key (static_cast<char *>(addr), len);
    auto it = this->rev_map_.find (key);
    if (it == this->rev_map_.end ()) {
      return NULL;
    } else {
      std::string *name = &(it->second);
      static const size_t MAX_RECUR = 10;
      for (size_t i = 0; i < MAX_RECUR; i++) {
        auto cit = this->cname_map_.find(*name);
        if (cit != this->cname_map_.end()) {
          debug(DBG, "cname trace: %s -> %s", name->c_str(), (cit->second).c_str());
          name = &(cit->second);
        } else {
          break;
        }
      }

      assert(name);
      return name;
    }
  }

  void DnsDB::insert(const std::string &name, const std::string &type,
                        const std::string &addr, void *ptr, size_t len,
                        double ts, const std::string &dst_addr) {
    static const std::string k_ts   = "ts";
    static const std::string k_type = "type";
    static const std::string k_src  = "src";
    static const std::string k_name = "name";
    static const std::string k_addr = "addr";

    if (!ptr) {
      return;
    }

    if (type == "CNAME") {
      debug(DBG, "insert: %s -> %s", addr.c_str(), name.c_str());
      this->cname_map_.insert(std::make_pair(addr, name));
    }

    if (type == "A" || type == "AAAA") {
      // register to in-memory DB
      std::string key (static_cast<char*>(ptr), len);
      this->rev_map_.insert (std::make_pair (key, name));

      // create msgpack object
      msgpack::sbuffer buf;
      msgpack::packer <msgpack::sbuffer> pk (&buf);

      pk.pack_map (5);
      pk.pack (k_ts); pk.pack (ts);
      pk.pack (k_src); pk.pack (dst_addr);  // Host that sent the query
      pk.pack (k_type); pk.pack (type);
      pk.pack (k_name); pk.pack (name);
      pk.pack (k_addr); pk.pack (addr);

      // register to redis
      if (this->redis_ctx_) {
        void *com = redisCommand(this->redis_ctx_, 
                                 "lpush %b %b", ptr, len, buf.data (), buf.size ());
        freeReplyObject (com);
      }
    }

    if (type == "A" || type == "AAAA" || type == "CNAME") {
      // publish zmq message
      if (this->output_) {
        this->output_->dns_answer(ts, name, type, addr, dst_addr);
      }
    }
  }
  
  void DnsDB::recv (swarm::ev_id eid, const  swarm::Property &p) {
    debug (DBG, "pkt recv");
    
    for (size_t i = 0; i < p.value_size("dns.an_name"); i++) {
      std::string name = p.value("dns.an_name", i).repr();
      std::string type = p.value("dns.an_type", i).repr();
      std::string addr = p.value("dns.an_data", i).repr();
      debug (DBG, "%s (%s) %s", name.c_str (), type.c_str (), addr.c_str ());

      size_t len;
      void * ptr = p.value("dns.an_data", i).ptr(&len);
      this->insert(name, type, addr, ptr, len, p.ts(), p.dst_addr());
    }

    return;
  }

  void DnsDB::set_output(Output *output) {
    this->output_ = output;
  }
  const std::string &DnsDB::errmsg () const {
    return this->errmsg_;
  }



}  // namespace dnshive
