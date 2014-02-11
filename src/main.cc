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

#include <hive.h>
#include "./optparse.h"

int run(const optparse::Values &opt, const std::vector<std::string> &args) {

  // Preparing DnsHive instance
  dnshive::Hive *h = new dnshive::Hive ();

  // Enable redis DB
  if (opt.is_set ("redis_db")) {
    if (!h->enable_redis_db (opt["redis_host"], opt["redis_port"],
                             opt["redis_db"])) {
      std::cerr << "Error: " << h->errmsg () << std::endl;
      return 1;
    }
  }

  // Disable output of packet detail
  if (opt.get("quiet")) {
    h->enable_quiet();
  }

  // Enable ZeroMQ PUB server
  if (opt.is_set("zmq_addr")) {
    if (!h->enable_zmq(opt["zmq_addr"])) {
      std::cerr << "Error: " << h->errmsg () << std::endl;
      return 1;
    } 
  }

  // Enable MsgPack object
  if (opt.is_set("msgpack_path")) {
    if (!h->enable_msgpack_ofs(opt["msgpack_path"])) {
      std::cerr << "Error: " << h->errmsg () << std::endl;
      return 1;
    } 
  }

  if (opt.is_set ("interface")) {
    // monitoring network interface
    std::cerr << "Interface: " << opt["interface"] << std::endl;
    if (!h->capture (opt["interface"], true)) {  // dev = true
      std::cerr << "Error: " << h->errmsg() << std::endl;
      return 1;
    }
  } else if (opt.is_set("read_file")) {
    // reading files
    for (auto it = opt.all("read_file").begin ();
         it != opt.all("read_file").end (); it++) {

      std::cerr << "Read file: " << (*it) << std::endl;
      if (!h->capture (*it, false)) {
        std::cerr << "Error: " << h->errmsg() << std::endl;
        return 1;
      }
    }
  } else {
    std::cerr << "No file and interface to capture. exiting..." << std::endl;
  }

  delete h;
  return 0;
}

int main (int argc, char *argv[]) {
  // Handling command line arguments
  optparse::OptionParser psr = optparse::OptionParser();
  psr.add_option("-r").action("append").dest("read_file")
    .help("Specify read pcap format file(s)");
  psr.add_option("-i").action("store").dest("interface")
    .help("Specify interface to monitor on the fly");

  // Options for redis DB
  psr.add_option("-d").action("store").dest("redis_db")
    .metavar ("INT")
    .help("Enable redis DB and specify Index (MUST be set if you want redis DB");
  psr.add_option("-s").action("store").dest("redis_host")
    .set_default ("localhost")
    .help("Reids DB Host, default is localhost");
  psr.add_option("-p").action("store").dest("redis_port")
    .set_default ("6379").metavar ("INT")
    .help("Reids DB Port, default is 6379");

  // Options for ZeroMQ
  psr.add_option("-z").action("store").dest("zmq_addr")
    .metavar ("STR")
    .help("Enable ZeroMQ PUB address, such as '*:9000'");

  // options for MsgPack
  psr.add_option("-m").action("store").dest("msgpack_path")
    .metavar ("STR")
    .help("Enable MsgPack object to save events");

  psr.add_option("-q").action("store_true").dest("quiet")
    .help("Quiet mode");

  optparse::Values& opt = psr.parse_args(argc, argv);
  std::vector <std::string> args = psr.args();
  int rc = (0 == run(opt, args)) ? EXIT_SUCCESS : EXIT_FAILURE;
  ::exit(rc);
}

