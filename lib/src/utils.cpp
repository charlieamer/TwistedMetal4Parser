#include "mr_parser/utils.h"

vector<string> splitString(string target, string delim)
{
  vector<string> v;
  if (!target.empty())
  {
    string::size_type start = 0;
    do
    {
      size_t x = target.find(delim, start);
      if (x == string::npos)
        break;

      string strToAdd = target.substr(start, x - start);
      v.push_back(strToAdd);
      start += delim.size() + strToAdd.length();
    } while (true);

    v.push_back(target.substr(start));
  }
  return v;
}
