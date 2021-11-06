
#pragma once

#include <map>
#include <string>
#include <vector>

class CmdLineParser {
public:
  class Param {
  public:
    std::string               mKey;
    std::vector<std::string>  mValues;
    size_t                    mValueCount{ 0 };
    bool                      mRequirement{ false };

    Param() = default;
    Param(const std::string& key) : mKey(key) {}
    Param(const std::string& key, int valueCount, bool req) : mKey(key), mValueCount(valueCount), mRequirement(req) {}

    std::string GetValue0() const { return !mValues.empty() ? mValues[0] : std::string(); }
  };

public:
  CmdLineParser() {}

  void                        AddPartern(const std::string& key, size_t valueCount, bool requirement);
  void                        AddHelp(const std::string& str);
  
  const CmdLineParser::Param& GetValue(const std::string& key);
  const std::string&          HelpInfo();
  
  bool                        Parse(size_t argc, char** argv);

private:
  std::map<std::string, Param>  mKeyValue;
  std::vector<std::string>      mValue;
  std::string                   mHelpInfo;
};