#include "curlpp/Easy.hpp"
#include "curlpp/cURLpp.hpp"
#include "curlpp/Options.hpp"
#include "gumbo.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <cstdlib>
#include <sstream>

static void search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  
  if (node->v.element.tag == GUMBO_TAG_A) {
    GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
    if (href) {
      std::cout << href->value << std::endl;
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
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