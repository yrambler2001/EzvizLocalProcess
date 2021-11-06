
#include "cmdline.h"

void CmdLineParser::AddPartern(const std::string& key, size_t valueCount, bool requirement) {
  mKeyValue[key] = Param(key, valueCount, requirement);
}

void CmdLineParser::AddHelp(const std::string& str) {
  mHelpInfo = str;
}

const CmdLineParser::Param& CmdLineParser::GetValue(const std::string& key) {
  return mKeyValue[key];
}

const std::string& CmdLineParser::HelpInfo() {
  return mHelpInfo;
}

bool CmdLineParser::Parse(size_t argc, char** argv) {
  for (size_t i = 1; i < argc; ) {
    std::string str = argv[i];
    auto it = mKeyValue.find(str);
    if (it == mKeyValue.end()) {
      mValue.push_back(str);
      ++i;
    }
    else {
      if (i < argc - it->second.mValueCount) {
        while (i < argc - it->second.mValueCount && it->second.mValues.size() < it->second.mValueCount) {
          it->second.mValues.push_back(argv[i + 1]);
          ++i;
        }
        ++i;
      }
      else {
        it->second.mValues.clear();
        ++i;
      }
    }
  }
  for (auto& p : mKeyValue) {
    if (p.second.mRequirement && p.second.mValues.size() != p.second.mValueCount)
      return false;
  }
  return true;
}