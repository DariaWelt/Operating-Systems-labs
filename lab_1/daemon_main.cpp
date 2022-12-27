
#include <iostream>
#include <cstdlib>

#include "src/Daemon.h"
int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    std::cout << "Wrong num of arguments" << std::endl;
    return EXIT_FAILURE;
  }
  auto daemon = Daemon::getInstance();
  daemon->init(argv[1]);
  auto rc = daemon->execute();
  delete daemon;
  return rc;
}