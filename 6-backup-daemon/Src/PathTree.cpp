#include "PathTree.hpp"

#include "StderrWarning.hpp"

using namespace HwBackup;

PathTree::PathTree() {
  Root = std::make_unique<Node>(".");
  Map[Root->Path] = Root.get();
}

PathTree::Node::Node(const std::string& path) : Path{path} {}

void PathTree::AddPath(const std::string& path, const std::string& name) {
  std::string pathName = path + "/" + name;

  STDERR_WARN("Adding path \"" << pathName);

  auto found = Map.find(pathName);
  if (found != Map.end()) return;

  found = Map.find(path);
  if (found == Map.end()) {
    size_t sepPos = path.rfind('/');
    if (sepPos == std::string::npos) THROW("Separator not found");

    std::string newPath = std::string{path.begin(), path.begin() + sepPos};
    std::string newName = std::string{path.begin() + sepPos + 1, path.end()};

    AddPath(newPath, newName);
    found = Map.find(path);
  }

  if (found == Map.end()) THROW("Unexpected error in path mapping");

  Node* parent = found->second;

  assert(parent);

  Node::PtrT newNode = std::make_unique<Node>(pathName);
  parent->Children.push_back(std::move(newNode));
  Map[pathName] = parent->Children.back().get();
}

void PathTree::VisitPostOrder(VisitF func) const {
  VisitRecursivePostOrder(*Root, func);
}
void PathTree::VisitPreOrder(VisitF func) const {
  VisitRecursivePreOrder(*Root, func);
}

void PathTree::Clear() {
  Root->Children.clear();
  Map.clear();
  Map[Root->Path] = Root.get();
}

void PathTree::VisitRecursivePreOrder(const Node& node, VisitF func) const {
  STDERR_WARN("Visiting \"" << node.Path << "\"");
  func(node.Path);

  for (const auto& childPtr : node.Children)
    VisitRecursivePreOrder(*childPtr, func);
}

void PathTree::VisitRecursivePostOrder(const Node& node, VisitF func) const {
  for (const auto& childPtr : node.Children)
    VisitRecursivePostOrder(*childPtr, func);

  func(node.Path);
}

void PathTree::Dump(std::ostream& os) const {
  size_t indent = 0;

  std::function<void(const Node&)> func;
  func = [&indent, &os, &func](const Node& node) {
    os << std::string(indent, ' ') << node.Path << std::endl;
    indent++;

    for (const auto& child : node.Children) func(*child);
    indent--;
  };

  func(*Root);
}