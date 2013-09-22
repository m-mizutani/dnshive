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

int main (int argc, char *argv[]) {
  const std::string &od_file = "read_file";
  optparse::OptionParser psr = optparse::OptionParser();
  psr.add_option("-r", "--read-file").action("append").dest(od_file);
  psr.add_option("-i", "--interface").action("store").dest("interface");

  optparse::Values& opt = psr.parse_args(argc, argv);
  std::vector <std::string> args = psr.args();

  dnshive::Hive *h = new dnshive::Hive ();

  if (opt.is_set ("interface")) {
    h->capture (opt["interface"], true);  // dev = true
  } else {
    for (auto it = opt.all(od_file).begin ();
         it != opt.all(od_file).end (); it++) {
      
      printf ("file: %s\n", (*it).c_str ());
      h->capture (*it, false);  // dev = false
    }
  }

  return 0;
}
