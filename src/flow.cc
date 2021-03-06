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
#include <iomanip>
#include <string.h>

#include "./flow.h"
#include "./output.h"
#include "./dnsdb.h"

namespace dnshive {
  Flow::Flow(const swarm::Property &p, const std::string &src, 
             const std::string &dst) {  
    const void *key = p.ssn_label(&(this->len_));
    this->key_ = malloc(this->len_);
    memcpy(this->key_, key, this->len_);
    this->hash_ = p.hash_value();
    this->dir_ = p.dir();

    this->base_ts_ = p.tv_sec();
    this->proto_ = p.proto();
    this->c_pkt_ = 0;
    this->c_size_ = 0;
    this->c_name_ = src;
    this->c_addr_ = p.src_addr();
    this->c_port_ = p.src_port();
    this->s_pkt_ = 0;
    this->s_size_ = 0;
    this->s_name_ = dst;
    this->s_port_ = p.dst_port();
    this->s_addr_ = p.dst_addr();

    /*    
    std::stringstream ss;
    const uint8_t *base = static_cast<const uint8_t*>(key);
    for (const uint8_t *p = base; p < base + this->len_; p++) {
      char tmp[4];
      snprintf(tmp, sizeof(tmp), "%02X", *p);
      ss << tmp;
    }
    std::cout << "(" << this->hash_ << ") [" << ss.str() << "]" << std::endl;
    */
  }
  Flow::~Flow() {
  }
  uint64_t Flow::hash() {
    return this->hash_;
  }
  bool Flow::match(const void *key, size_t len) {
    return (this->len_ == len && 0 == ::memcmp(key, this->key_, len));
  }
  void Flow::update(const swarm::Property &p) {
    this->last_ts_ = p.tv_sec();
    if (this->dir_ == p.dir()) {
      this->c_pkt_  += 1;
      this->c_size_ += p.len();
    } else {
      this->s_pkt_  += 1;
      this->s_size_ += p.len();
    }
  }

  FlowHandler::FlowHandler () : flow_table_(600), last_ts_(0), output_pkt_(true) {
  }
  FlowHandler::~FlowHandler () {
    this->flow_table_.flush();
    while (Flow *ef = dynamic_cast<Flow*>(this->flow_table_.pop())) {
      if (this->output_) {
        this->output_->expire_flow(*ef);
      }
      delete ef;
    }
  }

  void FlowHandler::set_db (DnsDB *db) {
    this->db_ = db;
  }
  void FlowHandler::set_output(Output *output) {
    this->output_ = output;
  }
  void FlowHandler::enable_output_pkt() {
    this->output_pkt_ = true;
  }
  void FlowHandler::disable_output_pkt() {
    this->output_pkt_ = false;
  }
  void FlowHandler::recv (swarm::ev_id eid, const  swarm::Property &p) {
    const int TIMEOUT = 300;

    // Timeout handling
    if (this->last_ts_ > 0 && this->last_ts_ < p.tv_sec()) {
      this->flow_table_.prog(p.tv_sec() - this->last_ts_);
      Flow *ef;
      while ((ef = dynamic_cast<Flow*>(this->flow_table_.pop()))) {
        if (p.tv_sec() < ef->last_ts() + TIMEOUT) {
          this->flow_table_.put(TIMEOUT, ef);
        } else {
          if (this->output_) {
            this->output_->expire_flow(*ef);
          }
          delete ef;
        }
      }
    }
    this->last_ts_ = p.tv_sec();

    // Look up host name
    const std::string *src = NULL, *dst = NULL;
    const std::string *src_name = NULL, *dst_name = NULL;
    std::string src_addr, dst_addr;
    size_t src_len, dst_len;
    void *s_addr = p.src_addr (&src_len);
    void *d_addr = p.dst_addr (&dst_len);

    // Need both of source address and destination address
    if (s_addr && d_addr) {
      src_name = this->db_->lookup (s_addr, src_len);
      dst_name = this->db_->lookup (d_addr, src_len);
      src_addr = p.src_addr();
      dst_addr = p.dst_addr();

      src = (src_name) ? src_name : &src_addr;
      dst = (dst_name) ? dst_name : &dst_addr;
    }

    assert(src && dst);

    if (this->output_pkt_) {
      printf("%u.%06u %s ",
             p.tv_sec (), p.tv_usec(), p.proto().c_str());

      if (src_name) {
        printf("%s(%s)/%d", src_name->c_str(), src_addr.c_str(), p.src_port());
      } else {
        printf("%s/%d", src_addr.c_str(), p.src_port());
      }

      printf(" -> ");

      if (dst_name) {
        printf("%s(%s)/%d", dst_name->c_str(), dst_addr.c_str(), p.dst_port());
      } else {
        printf("%s/%d", dst_addr.c_str(), p.dst_port());
      }

      printf (" Len:%d\n", p.len());
    }

    // Look up existing Flow object
    size_t len;
    const void *key = p.ssn_label(&len);
    Flow *f = dynamic_cast<Flow*>
      (this->flow_table_.get(p.hash_value(), key, len));

    // If not found,
    if (f == NULL && src && dst) {
      // Allocation new Flow object
      f = new Flow(p, *src, *dst);
      this->flow_table_.put(TIMEOUT, f);
      if (this->output_) {
        this->output_->new_flow(*f);
      }
    }
    assert(f);
    f->update(p);
    
  }
}



