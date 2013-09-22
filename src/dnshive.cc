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

namespace dnshive {
  /*
  void capture (const std::string &dev, const std::string &filter = "") {
    // ----------------------------------------------
    // setup NetDec
    swarm::NetDec *nd = new swarm::NetDec ();
    DnsFwdDB *dns_db = new DnsFwdDB ();
    IPFlow *ip4_flow = new IPFlow ();
    ip4_flow->set_db (dns_db);

    nd->set_handler ("dns.an", dns_db);
    nd->set_handler ("ipv4.packet", ip4_flow);

    swarm::NetCap *nc = new swarm::NetCap (nd);
    if (!nc->capture (dev, filter)) {
      printf ("error: %s\n", nc->errmsg ().c_str ());
    }
  }


  void read_pcapfile (const std::string &fpath) {
    printf ("open: \"%s\"\n", fpath.c_str ());

    // ----------------------------------------------
    // setup NetDec
    ip4_flow->set_db (dns_db);

    nd->set_handler ("dns.an", dns_db);
    nd->set_handler ("ipv4.packet", ip4_flow);

    // ----------------------------------------------
    // processing packets from pcap file
    swarm::NetCap *nc = new swarm::NetCap (nd);
    if (!nc->read_pcapfile (fpath)) {
      printf ("error: %s\n", nc->errmsg ().c_str ());
    }

    return;
  }
  */

  Hive::Hive () { 
    this->nd_ = new swarm::NetDec ();
    this->dns_db_ = new DnsFwdDB ();
    this->ip4_flow_ = new IPFlow ();
    this->ip4_flow_->set_db (this->dns_db_);
    this->nd_->set_handler ("dns.an", this->dns_db_);
    // this->nd_->set_handler ("ipv4.packet", this->ip4_flow_);
  }
  Hive::~Hive () {
    delete this->nd_;
    delete this->ip4_flow_;
    delete this->dns_db_;
  }

  bool Hive::capture (const std::string &arg, bool dev) {
    swarm::NetCap *nc = new swarm::NetCap (this->nd_);

    if (dev) {
      if (!nc->capture (arg)) {
        printf ("error: %s\n", nc->errmsg ().c_str ());
      }
    } else {
      if (!nc->read_pcapfile (arg)) {
        printf ("error: %s, %s\n", arg.c_str (), nc->errmsg ().c_str ());
      }
    }

    return true;
  }
  
}



