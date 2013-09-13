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

#include "proc.h"

namespace dnshive {
  const std::string * DnsFwdDB::lookup (u_int32_t * v4addr) {
    auto it = this->rev_map_.find (*v4addr);
    if (it == this->rev_map_.end ()) {
      return NULL;
    } else {
      return &(it->second);
    }
  }

  void DnsFwdDB::recv (swarm::ev_id eid, const  swarm::Property &p) {
    for (size_t i = 0; i < p.param ("dns.an_name")->size (); i++) {
      std::string name = p.param ("dns.an_name")->repr (i);
      std::string type = p.param ("dns.an_type")->repr (i);
      std::string addr;

      addr = p.param ("dns.an_data")->repr (i);
      printf ("%s (%s) %s\n", name.c_str (), type.c_str (), addr.c_str ());

      void * ptr = p.param ("dns.an_data")->get (NULL, i);
      if (ptr) {
        u_int32_t * a = static_cast<u_int32_t*> (ptr);
        this->rev_map_.insert (std::make_pair (*a, name));
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

    printf ("%s -> %s\n", src->c_str (), dst->c_str ());
  }

}  // namespace dnshive
