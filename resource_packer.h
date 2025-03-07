#pragma once

#include <string>

#include "format.h"

namespace base {
  namespace resources {
    class resource_packer
    {
    public:
      class options {
      public:
        std::string name;
        content content;
      };

    public:
      void pack(const std::string& inputDir, const std::string& outputDir, const options& options);
    };
  }
}

