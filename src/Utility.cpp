#include <string>
#include <stdexcept>

bool replace(std::string &str, const std::string &from, const std::string &to)
{
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::string formatError(std::runtime_error error)
{
  auto errorstr = std::string(error.what());
  replace(errorstr, ": ", "\n");
  replace(errorstr, ": ", "\n");
  replace(errorstr, ": ", "\n");
  return errorstr;
}

void showRuntimeError(std::runtime_error error)
{
  std::cout << "Runtime Error: " + std::string(error.what());
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", formatError(error).c_str(), NULL);
}