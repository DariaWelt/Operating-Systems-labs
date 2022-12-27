//
// Created by aleksei on 10.10.2021.
//

#ifndef DAEMON_RC_H
#define DAEMON_RC_H

enum RC {
  SUCCESS             = 0,
  FILE_CLOSED         = 1,
  FILE_DOES_NOT_EXIST = 2,
  WRONG_CONFIG_FORMAT = 3,
  FAILED_FORK         = 4,
  PARENT_PROCESS      = 5,
  FAILED_CHANGE_DIR   = 6,
  EMPTY_STRING        = 7,
};

#endif //DAEMON_RC_H
