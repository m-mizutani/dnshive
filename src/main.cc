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

#include <dnshive.h>
#include "./optparse.h"

class PktHandler : public dnshive::Handler {
 public:
  PktHandler() {
  }
  virtual ~PktHandler() {
  }
  
  void flow (const std::string &src, const std::string &dst,
             const swarm::Property &prop) {
    std::string proto = prop.proto ();
    printf ("%14.6f %s %s/%d -> %s/%d %d length\n", prop.ts (), proto.c_str (),
            src.c_str (), prop.src_port (), dst.c_str (), prop.dst_port (),
            prop.len ());
  }
};

int main (int argc, char *argv[]) {
  // Handling command line arguments
  const std::string &od_file = "read_file";
  optparse::OptionParser psr = optparse::OptionParser();
  psr.add_option("-r").action("append").dest(od_file)
    .help("Specify read pcap format file(s)");
  psr.add_option("-i").action("store").dest("interface")
    .help("Specify interface to monitor on the fly");
  psr.add_option("-d").action("store").dest("redis_db")
    .metavar ("INT")
    .help("Redis DB Index (MUST be set if you want redis DB");
  psr.add_option("-h").action("store").dest("redis_host")
    .set_default ("localhost")
    .help("Reids DB Host, default is localhost");
  psr.add_option("-p").action("store").dest("redis_port")
    .set_default ("6379").metavar ("INT")
    .help("Reids DB Port, default is 6379");

  optparse::Values& opt = psr.parse_args(argc, argv);
  std::vector <std::string> args = psr.args();

  // Preparing DnsHive instance
  dnshive::Hive *h = new dnshive::Hive ();
  PktHandler * ph = new PktHandler ();
  h->set_handler (ph);

  if (opt.is_set ("redis_db")) {
    if (!h->enable_redis_db (opt["redis_host"], opt["redis_port"],
                             opt["redis_db"])) {
      std::cerr << "Error: " << h->errmsg () << std::endl;
    }
  }

  if (opt.is_set ("interface")) {
    // monitoring network interface
    std::cerr << "Interface: " << opt["interface"] << std::endl;
    h->capture (opt["interface"], true);  // dev = true
  } else {
    // reading files
    for (auto it = opt.all(od_file).begin ();
         it != opt.all(od_file).end (); it++) {

      std::cerr << "Read file: " << (*it) << std::endl;
      h->capture (*it, false);  // dev = false
    }
  }

  return 0;
}
