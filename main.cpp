#include <iostream>
#include <filesystem>

#include "resource_packer.h"
#include "resource_pack.h"

using namespace base::resources;

int main(int argc, char* args[])
{
  std::cout << "----------- Started ------------\n" << std::endl;

    std::filesystem::path input = std::filesystem::current_path() / "input";
    std::filesystem::path output = std::filesystem::current_path() / "output";
    std::string type = "shaders textures audios texture_arrays";

    std::cout << "Parameter Count: " << argc << std::endl;
    if (argc > 1) {
      input = std::filesystem::path(std::string(args[1]));
      output = std::filesystem::path(std::string(args[2]));
      type = std::string(args[3]);

      std::cout << "First Arg: " << args[1] << std::endl;
      std::cout << "Second Arg: " << args[2] << std::endl;
      std::cout << "Third Arg: " << args[3] << std::endl;
    }

    std::cout << std::endl;

    resource_packer::options options{};
    resource_packer packer{};

    if (type.find("textures") != std::string::npos) {
      options.content = content::textures;
      options.name = "textures.rp";

      // test textures
      std::cout << "Packing Textures..." << std::endl;
      std::cout << std::endl;

      try {
        packer.pack(input.string(), output.string(), options);
      }
      catch (std::runtime_error& e) {
        std::cout << "Failed: " << e.what() << std::endl;
      }
    }

    if (type.find("audios") != std::string::npos) {
      // test audio
      std::cout << std::endl;
      std::cout << std::endl;

      std::cout << "Packing Audios..." << std::endl;

      options.content = content::audios;
      options.name = "audios.rp";
      try {
        packer.pack(input.string(), output.string(), options);
      }
      catch (std::runtime_error& e) {
        std::cout << "Failed: " << e.what() << std::endl;
      }
    }

    if (type.find("shaders") != std::string::npos) {
      // test shader
      std::cout << std::endl;
      std::cout << std::endl;

      std::cout << "Packing Shaders..." << std::endl;

      options.content = content::shaders;
      options.name = "shaders.rp";
      try {
        packer.pack(input.string(), output.string(), options);
      }
      catch (std::runtime_error& e) {
        std::cout << "Failed: " << e.what() << std::endl;
      }
    }

    if (type.find("texture_arrays") != std::string::npos) {
      options.content = content::texture_arrays;
      options.name = "texture_arrays.rp";

      // test textures
      std::cout << "Packing Texture Arrays..." << std::endl;
      std::cout << std::endl;

      try {
        packer.pack(input.string(), output.string(), options);
      }
      catch (std::runtime_error& e) {
        std::cout << "Failed: " << e.what() << std::endl;
      }
    }

    std::cout << "----------- Ended ------------\n" << std::endl;
}