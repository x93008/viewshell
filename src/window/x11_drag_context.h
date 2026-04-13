#pragma once

namespace viewshell {

struct DragContext {
  bool is_valid = false;
  int button = 0;
  int root_x = 0;
  int root_y = 0;
  unsigned int timestamp = 0;
};

}
