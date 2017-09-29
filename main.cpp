#include "curlpp/Easy.hpp"
#include "curlpp/cURLpp.hpp"
#include "curlpp/Options.hpp"
#include "gumbo.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <cstdlib>
#include <sstream>

static std::vector<std::string> search_for_links(GumboNode* node) {
	std::vector<std::string> out;
  if (node->type != GUMBO_NODE_ELEMENT) {
    return out;
  }
  
  if (node->v.element.tag == GUMBO_TAG_A) {
    GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
    if (href) {
      out.push_back(href->value);
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    std::vector<std::string> out2 = search_for_links(static_cast<GumboNode*>(children->data[i]));
		out.reserve(out.size()+out2.size());
		out.insert(out.end(), out2.begin(), out2.end());
  }
	return out;
}

std::wstring utf8_to_wstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

int main() {
  setlocale(LC_ALL, "Russian");
  curlpp::Cleanup init_handler;
  curlpp::Easy easy_handle;
  easy_handle.setOpt(cURLpp::Options::Url("http://www.apmath.spbu.ru/ru/"));
  std::stringstream resp;
  easy_handle.setOpt(cURLpp::Options::WriteStream(&resp));
  easy_handle.perform();
  
  GumboOutput* output = gumbo_parse(resp.str().c_str());
  search_for_links(output->root);
  gumbo_destroy_output(&kGumboDefaultOptions, output);

  system("pause");
  return 0;
}
