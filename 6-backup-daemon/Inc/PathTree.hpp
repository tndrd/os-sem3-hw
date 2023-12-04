#pragma once

#include <assert.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "HwBackupException.hpp"
#include "StderrWarning.hpp"

namespace HwBackup {

struct PathTree final {
 private:
  struct Node final {
    using PtrT = std::unique_ptr<Node>;

    std::string Path;
    std::vector<PtrT> Children;

    Node(const std::string& path);
  };

 private:
  Node::PtrT Root;
  std::unordered_map<std::string, Node*> Map;

 public:
  using VisitF = std::function<void(const std::string&)>;

  PathTree();

  void AddPath(const std::string& path, const std::string& name);

  void VisitPostOrder(VisitF func) const;
  void VisitPreOrder(VisitF func) const;
  void Clear();

  PathTree(const PathTree&) = delete;
  PathTree& operator=(const PathTree&) = delete;

  PathTree(PathTree&&) = default;
  PathTree& operator=(PathTree&&) = default;

  ~PathTree() = default;

  void Dump(std::ostream& os) const;

 private:
  void VisitRecursivePostOrder(const Node& node, VisitF func) const;
  void VisitRecursivePreOrder(const Node& node, VisitF func) const;
};

}  // namespace HwBackup