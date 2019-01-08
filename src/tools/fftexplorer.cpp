#include "a8/script/script.h"
#include "a8/script/document.h"

#include <ffts/ffts.h>
#include <boost/align/aligned_allocator.hpp>
#include <iostream>
#include <string>
#include <vector>

int a8main(int argc, char *argv[]) {
  std::string line;
  std::vector<float, boost::alignment::aligned_allocator<float, 32>> inputs;

  // std::cout << "Enter stuff\n";
  // a8::script::Script script;
  // do {
  //   std::getline(std::cin, line);
  // } while (script.run(line.c_str()));
  a8::script::StdinDocument doc;
  doc.onAppend([](IDocument &doc, unsigned split) {
    
  });
  doc.run();
  return 0;
}

