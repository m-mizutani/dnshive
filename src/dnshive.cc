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

#include <sys/types.h>
#include <swarm.h>
#include <map>


#include "dnshive.h"
#include "proc.h"
#include "debug.h"

namespace dnshive {
  Hive::Hive () { 
    this->nd_ = new swarm::NetDec ();
    this->dns_db_ = new DnsFwdDB ();
    this->ip_flow_ = new IPFlow ();
    this->ip_flow_->set_db (this->dns_db_);
    this->nd_->set_handler ("dns.an", this->dns_db_);
    this->nd_->set_handler ("ipv4.packet", this->ip_flow_);
    this->nd_->set_handler ("ipv6.packet", this->ip_flow_);
  }
  Hive::~Hive () {
    delete this->nd_;
    delete this->ip_flow_;
    delete this->dns_db_;
  }

  bool Hive::capture (const std::string &arg, bool dev) {
    swarm::NetCap *nc = NULL;
    if (dev) {
      nc = new swarm::CapPcapDev(arg);
    } else {
      nc = new swarm::CapPcapFile(arg);
    }

    nc->bind_netdec(this->nd_);

    if (!nc->ready()) {
      this->errmsg_ = nc->errmsg();
      return false;
    }

    nc->start();
    return true;
  }

  bool Hive::enable_redis_db (const std::string &host, const std::string &port,
                              const std::string &db) {
    if (this->dns_db_->enable_redis_db (host, port, db)) {
      int rc = this->dns_db_->load_redis_db ();
      debug (0, "rc = %d", rc);
      return true;
    } else {
      this->errmsg_ = this->dns_db_->errmsg ();
      return false;
    }
  }

  void Hive::set_handler (Handler *hdlr) {
    this->ip_flow_->set_handler (hdlr);
  }
  void Hive::unset_handler () {
    this->ip_flow_->unset_handler ();
  }

  const std::string& Hive::errmsg () const {
    return this->errmsg_;
  }
  
}



