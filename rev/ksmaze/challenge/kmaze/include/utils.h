#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include <optional>

/**
 * Taken from this cool blog:
 * https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
 */
static std::optional<std::string>
get_file_contents(std::filesystem::path file)
{
  std::ifstream in(file, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return {contents};
  }
  return {};
}
