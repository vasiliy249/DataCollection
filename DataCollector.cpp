#include "DataCollector.h"
#include <sstream>
#include <codecvt>
#include <fstream>

std::wstring utf8_to_wstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

DWORD RunAdnWait(const std::string& path, const std::string& params) {
  std::ostringstream stm;
  stm << "\"" << path << "\"" << " " << params;
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));
  size_t str_size = static_cast<size_t>(stm.tellp()) + 1;
  std::vector<char> args(str_size);
  strcpy_s(&args[0], str_size, stm.str().c_str());
  ::CreateProcessA(path.c_str(), &args[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
  ::WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 0;
  ::GetExitCodeProcess(pi.hProcess, &code);
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);
  return code;
}

DataCollector::DataCollector() {
  Reset();
}

DataCollector::~DataCollector() {
  
}

void DataCollector::SetUrl(const std::string& url) {
  url_ = url;
}

void DataCollector::Reset() {
  url_.clear();
  request_handler_.reset();
}

bool DataCollector::GetHtmlData(std::wstring& text, Links& links) {
  if (url_.empty()) {
    return false;
  }
  request_handler_.setOpt(cURLpp::Options::Url(url_));
  std::stringstream resp_stream;
  request_handler_.setOpt(cURLpp::Options::WriteStream(&resp_stream));
  request_handler_.perform();

  GumboOutput* output = gumbo_parse(resp_stream.str().c_str());

  if (!GetLinks(output->root, links)) {
    links.clear();
    return false;
  }

  std::string utf8_text;
  if (!GetText(output->root, utf8_text)) {
    return false;
  } else {
    text = utf8_to_wstring(utf8_text);
  }

  gumbo_destroy_output(&kGumboDefaultOptions, output);

  return true;
}

bool DataCollector::GetPdfData(std::string& text) {
  //std::ofstream outfile("new.pdf", std::ofstream::binary);
  //outfile.write(resp.str().c_str(), resp.str().size());

  //RunAdnWait("C:\\externals\\bin\\xpdf\\bin32\\pdftotext.exe", "guide.pdf new.txt");
  return false;
}

bool DataCollector::GetDocDat(std::string& text) {
  return false;
}

bool DataCollector::GetLinks(GumboNode* node, Links& links) {
  if (node == NULL) {
    return false;
  }

  if (node->type != GUMBO_NODE_ELEMENT) {
    return false;
  }

  if (node->v.element.tag == GUMBO_TAG_A) {
    GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
    if (href) {
      links.push_back(href->value);
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    GetLinks(static_cast<GumboNode*>(children->data[i]), links);
  }
  return true;
}

bool DataCollector::GetText(GumboNode* node, std::string& text) {
  if (node == NULL) {
    return false;
  }

  if (node->type == GUMBO_NODE_TEXT) {
    if (!text.empty()) {
      text.append(" ");
    }
    text.append(node->v.text.text);
    return true;
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      GetText(static_cast<GumboNode*>(children->data[i]), text);
    }
    return true;
  } else {
    return false;
  }
}