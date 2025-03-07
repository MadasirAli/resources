#include "resource_packer.h"

#include <iostream>
#include <filesystem>
#include <assert.h>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <map>
#include <unordered_map>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define DRWAV_IMPLEMENTATION
#include "dr_wav.h"

#include "resource_pack_header.h"

using namespace base::resources;

void resource_packer::pack(const std::string& inputDir, const std::string& outputDir, const options& options)
{
  using namespace std;

  if (filesystem::exists(inputDir) == false) {
    cout << "Input directroy does not exists: " << inputDir;
    throw runtime_error(inputDir + "\nInput directory does not exists.\n");
  }
  else {
    if (filesystem::is_directory(inputDir) == false) {
      cout << "Invalid input directory: " << inputDir;
      throw runtime_error(inputDir + "\nInput directory is not a valid.\n");
    }
  }

  if (filesystem::exists(outputDir) == false) {
    filesystem::create_directory(outputDir);
    cout << "Output directory creared: " << outputDir << endl;
  }
  else {
    if (filesystem::is_directory(outputDir) == false) {
      filesystem::remove(outputDir);
      cout << "Removed invalid output file in path of output directroy: " << outputDir << endl;

      filesystem::create_directory(outputDir);
      cout << "Output Directory created: " << outputDir << endl;
    }
  }

  if (options.name.length() == 0) {
    cout << "No Resource Pack name provided." << endl;
    throw runtime_error("Invalid resource pack name.");
  }

  const auto outputFilePath = filesystem::path(outputDir) / options.name;
  if (filesystem::exists(outputFilePath)) {
    filesystem::remove(outputFilePath);
    cout << "Already exists resource pack removed so that new can be written: " << outputFilePath << endl;
  }

  ofstream outputFile(outputFilePath, std::ios::binary);
  if (!outputFile) {
    cout << "Failed to create output file (resource pack file): " << outputFilePath << endl;
    throw runtime_error(outputFilePath.string() + "\nFailed to create output file.\n");
  }

  cout << "Output file created: " << outputFilePath << endl;
  cout << "Loading all files in memory ... " << endl;

  resource_pack_header packHeader{};
  packHeader.contentType = options.content;
  packHeader.name = options.name;
  
  vector<pair<unique_ptr<char[]>, size_t>> pItemDataList{};

  if (options.content == content::texture_arrays) {

    unordered_map<string, map<uint32_t, pair<unique_ptr<char[]>, size_t>>> arrayList{};

    for (const auto& record : std::filesystem::recursive_directory_iterator(inputDir)) {
      if (record.is_directory()) {
        cout << "Skipping file as its a directory: " << record.path() << endl;
        continue;
      }

      if (record.is_regular_file() == false) {
        cout << "Skipping file its not regular file: " << record.path().filename() << endl;
        continue;
      }

      int width = 0;
      int height = 0;
      int channels = 0;

      const auto* const pData = stbi_load(record.path().string().c_str(), &width, &height, &channels, 0);
      const uint32_t size = width * height * channels;

      if (pData == nullptr) {
        cout << "Skipping: " << record.path().filename() << " is not suitable for graphics format. Or unable to load file." << endl;
        continue;
      }

      string arrayName = record.path().filename().string().substr(0, record.path().filename().string().find("_index_"));
      uint32_t arrayIndex = stoi(record.path().filename().string().substr(record.path().filename().string().find("_index_") + 7, record.path().filename().string().length() - record.path().filename().string().find(".") -1));

      resource_texture_item_header itemHeader{};
      itemHeader.name = record.path().filename().string();
      itemHeader.size = size;
      itemHeader.width = width;
      itemHeader.height = height;
      itemHeader.channels = channels;

      constexpr const size_t offset = sizeof(resource_texture_item_header);
      itemHeader.endSize = size + offset;

      auto pBytes = make_unique<char[]>(size + offset);
      *((resource_texture_item_header*)(pBytes.get())) = itemHeader;

      std::memcpy(pBytes.get() + offset, pData, size);

      if (arrayList.find(arrayName) == arrayList.cend()) {
        arrayList.insert(make_pair(arrayName, map<uint32_t, pair<unique_ptr<char[]>, size_t>>()));
      }
      arrayList.at(arrayName).insert(make_pair(arrayIndex, make_pair(move(pBytes), itemHeader.endSize)));

      // pItemDataList.emplace_back(std::move(pBytes), itemHeader.endSize);
      // packHeader.num_resources++;

      cout << "Loaded: " << itemHeader.name.string() << endl;
    }

    for (auto it = arrayList.cbegin(); it != arrayList.cend(); ++it) {

      size_t size = 0;
      uint32_t count = 0;
      for (auto entryIt = it->second.cbegin(); entryIt != it->second.cend(); ++entryIt) {
        size += entryIt->second.second;
        ++count;
      }

      constexpr const size_t headerSize = +sizeof(resource_texture_array_item_header);

      unique_ptr<char[]> pBytes = make_unique<char[]>(size + headerSize);
      size_t pinOffset = headerSize;
      for (auto entryIt = it->second.cbegin(); entryIt != it->second.cend(); ++entryIt) {
        std::memcpy(pBytes.get() + pinOffset, entryIt->second.first.get(), entryIt->second.second);
        pinOffset += entryIt->second.second;
      }

      resource_texture_array_item_header itemHeader{};
      itemHeader.name = it->first;
      itemHeader.count = count;
      itemHeader.size = size;
      itemHeader.endSize = size + headerSize;

      *((resource_texture_array_item_header*)(pBytes.get())) = itemHeader;

      pItemDataList.emplace_back(move(pBytes), itemHeader.endSize);
      packHeader.num_resources++;

      cout << "Loaded: " << it->first << " [Texture Array]" << endl;
    }
  }
  else {

    for (const auto& record : std::filesystem::recursive_directory_iterator(inputDir)) {
      if (record.is_directory()) {
        cout << "Skipping file as its a directory: " << record.path() << endl;
        continue;
      }

      if (record.is_regular_file() == false) {
        cout << "Skipping file its not regular file: " << record.path().filename() << endl;
        continue;
      }

      if (options.content == content::textures) {
        int width = 0;
        int height = 0;
        int channels = 0;

        const auto* const pData = stbi_load(record.path().string().c_str(), &width, &height, &channels, 0);
        const uint32_t size = width * height * channels;

        if (pData == nullptr) {
          cout << "Skipping: " << record.path().filename() << " is not suitable for graphics format. Or unable to load file." << endl;
          continue;
        }

        resource_texture_item_header itemHeader{};
        itemHeader.name = record.path().filename().string();
        itemHeader.size = size;
        itemHeader.width = width;
        itemHeader.height = height;
        itemHeader.channels = channels;

        constexpr const size_t offset = sizeof(resource_texture_item_header);
        itemHeader.endSize = size + offset;

        auto pBytes = make_unique<char[]>(size + offset);
        *((resource_texture_item_header*)(pBytes.get())) = itemHeader;

        std::memcpy(pBytes.get() + offset, pData, size);

        pItemDataList.emplace_back(std::move(pBytes), itemHeader.endSize);
        packHeader.num_resources++;

        cout << "Loaded: " << itemHeader.name.string() << endl;
      }
      else if (options.content == content::audios) {
        unsigned int channels = 0;
        unsigned int sampleRate = 0;
        uint64_t frameCount = 0;
        constexpr const uint32_t bytesPerSample = 32 / 8;

        const auto* const pData = drwav_open_file_and_read_pcm_frames_f32(record.path().string().c_str(), &channels, &sampleRate, &frameCount, NULL);
        const uint32_t size = (uint32_t)frameCount * bytesPerSample * channels;

        if (pData == nullptr) {
          cout << "Skipping: " << record.path().filename() << " is not suitable for audio format. Or unable to load file." << endl;
          continue;
        }

        resource_audio_item_header itemHeader{};
        itemHeader.name = record.path().filename().string();
        itemHeader.size = size;
        itemHeader.sampleRate = sampleRate;
        itemHeader.frameCount = frameCount;
        itemHeader.bytesPerSample = bytesPerSample;
        itemHeader.channels = channels;

        constexpr const size_t offset = sizeof(resource_audio_item_header);
        itemHeader.endSize = size + offset;

        auto pBytes = make_unique<char[]>(size + offset);
        *((resource_audio_item_header*)(pBytes.get())) = itemHeader;

        std::memcpy(pBytes.get() + offset, pData, size);

        pItemDataList.emplace_back(std::move(pBytes), itemHeader.endSize);
        packHeader.num_resources++;

        cout << "Loaded: " << itemHeader.name.string() << endl;
      }
      else if (options.content == content::shaders) {

        if (record.path().filename().string().find(".hlsl") == std::string::npos) {
          cout << "Skipping: " << record.path().filename() << " is not suitable for shader format. Or unable to load file." << endl;
          continue;
        }

        std::ifstream shaderFile(record.path(), std::ios::binary);
        const uint32_t size = (uint32_t)record.file_size();

        const auto pData = std::make_unique<char[]>(size);
        shaderFile.read(pData.get(), size);

        resource_shader_item_header itemHeader{};
        itemHeader.name = record.path().filename().string();
        itemHeader.size = size;

        constexpr const size_t offset = sizeof(resource_shader_item_header);
        itemHeader.endSize = size + offset;

        auto pBytes = make_unique<char[]>(size + offset);
        *((resource_shader_item_header*)(pBytes.get())) = itemHeader;

        std::memcpy(pBytes.get() + offset, pData.get(), size);

        pItemDataList.emplace_back(std::move(pBytes), itemHeader.endSize);
        packHeader.num_resources++;

        cout << "Loaded: " << itemHeader.name.string() << endl;
      }
      else {
        assert(false);
      }
    }
  }

  cout << "Total Packed Resources: " << packHeader.num_resources << endl;
  cout << "Writting ..." << endl;

  // writting header
  outputFile.write(reinterpret_cast<const char*>(&packHeader), sizeof(resource_pack_header));
  for (const auto& pDataPair : pItemDataList) {
    outputFile.write(pDataPair.first.get(), pDataPair.second);
  }
  outputFile.close();

  cout << "Resource Pack Generated: " << packHeader.name.string() << endl;
}