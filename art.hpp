#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <string_view>

namespace arttree {

struct ArtTreeDefs {
  static constexpr int MAX_PREFIX_LEN = 16;
};

enum class NodeType { Node4 = 0, Node16, Node48, Node256, Leaf, Invalid };

struct Node;

struct NodeLeaf {
  size_t key_len, val_len;
  char *key;
  char *val;

  NodeLeaf(std::string_view k, std::string_view v) {
    key_len = k.size();
    val_len = v.size();

    key = new char[key_len];
    val = new char[val_len];
  }

  ~NodeLeaf() {
    if (key) {
      delete[] key;
    }

    if (val) {
      delete[] val;
    }
  }
};

struct Node4 {
  char key[4];
  Node *children[4];

  Node4() { memset(children, 0, sizeof(Node *) * 4); }

  inline bool add_child(char ch, Node *child) {
    for (size_t i = 0; i < 4; ++i) {
      if (!children[i]) {
        children[i] = child;
        key[i] = ch;
      }
    }
    return false;
  }

  inline Node *find_child(char ch) {
    for (size_t i = 0; i < 4; ++i) {
      if (key[i] == ch) {
        return children[i];
      }
    }
    return nullptr;
  }
};

struct Node16 {
  inline bool add_child(char ch, Node *child) { return false; }

  inline Node *find_child(char ch) { return nullptr; }
};

struct Node48 {
  inline bool add_child(char ch, Node *child) { return false; }

  inline Node *find_child(char ch) { return nullptr; }
};

struct Node256 {
  inline bool add_child(char ch, Node *child) { return false; }

  inline Node *find_child(char ch) { return nullptr; }
};

struct Node {
  NodeType type{NodeType::Invalid};
  char prefix[ArtTreeDefs::MAX_PREFIX_LEN];
  size_t prefix_len{0};
  void *inner{nullptr};

  inline bool is_leaf() const { return type == NodeType::Leaf; }

  inline std::string_view load_key() const {
    assert(this->type == NodeType::Leaf);
    auto leaf = (NodeLeaf *)inner;
    return std::string_view{leaf->key, leaf->key_len};
  }

  size_t check_prefix(std::string_view key, size_t depth) {
    size_t i = depth;

    for (; i < prefix_len && i < key.size() && key[i] == prefix[i]; ++i) {
    }

    return i - depth;
  }

  Node *find_child(char ch) {
    switch (type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)inner;
      return n4->find_child(ch);
    } break;
    case NodeType::Node16: {
      Node16 *n16 = (Node16 *)inner;
      return n16->find_child(ch);
    } break;
    case NodeType::Node48: {
      Node48 *n48 = (Node48 *)inner;
      return n48->find_child(ch);
    } break;
    case NodeType::Node256: {
      Node256 *n256 = (Node256 *)inner;
      return n256->find_child(ch);
    } break;
    case NodeType::Leaf:
      assert(false && "Leaf node can't add child");
      break;
    default:
      assert(false && "Invalid node type");
    }
    return nullptr;
  }

  bool add_child(char ch, Node *n) {
    switch (type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)inner;
      n4->add_child(ch, n);
    } break;
    case NodeType::Node16: {
      Node16 *n16 = (Node16 *)inner;
      n16->add_child(ch, n);
    } break;
    case NodeType::Node48: {
      Node48 *n48 = (Node48 *)inner;
      n48->add_child(ch, n);
    } break;
    case NodeType::Node256: {
      Node256 *n256 = (Node256 *)inner;
      n256->add_child(ch, n);
    } break;
    case NodeType::Leaf:
      assert(false && "Leaf node can't add child");
      break;
    default:
      assert(false && "Invalid node type");
      return false;
    }
    return true;
  }

  static Node *make_node(NodeType type, std::string_view leaf_key,
                         std::string_view leaf_val) {
    Node *n = new Node{};
    n->type = type;
    switch (type) {
    case NodeType::Node4:
      n->inner = new Node4{};
      break;
    case NodeType::Node16:
      n->inner = new Node16{};
      break;
    case NodeType::Node48:
      n->inner = new Node48{};
      break;
    case NodeType::Node256:
      n->inner = new Node256{};
      break;
    case NodeType::Leaf:
      n->inner = new NodeLeaf{leaf_key, leaf_val};
    default:
      delete n;
      assert(false && "Invalid node type");
      return nullptr;
    }
    return n;
  }
};

class ArtTree {
public:
  bool insert(std::string_view key, std::string_view val);

private:
  bool insert(Node *&cur, const std::string_view &key, Node *leaf,
              size_t depth);

  Node *root_;
};

inline bool ArtTree::insert(std::string_view key, std::string_view val) {
  return false;
}

inline bool ArtTree::insert(Node *&node, const std::string_view &key, Node *leaf,
                     size_t depth) {
  if (node == nullptr) {
    node = leaf;
    return true;
  }

  if (node->is_leaf()) {
    Node *new_node = Node::make_node(NodeType::Node4, "", "");
    std::string_view key2 = node->load_key();
    size_t i = depth;
    for (; key[i] == key2[i]; i++) {
      new_node->prefix[i - depth] = key[i];
    }

    new_node->prefix_len = i - depth;
    depth = depth + new_node->prefix_len;

    new_node->add_child(key[depth], leaf);
    new_node->add_child(key2[depth], node);
    // replace
    node = new_node;
    return true;
  }

  size_t p = node->check_prefix(key, depth);
  if (p != node->prefix_len) {
    // prefix mismatch
    Node *new_node = Node::make_node(NodeType::Node4, "", "");
    new_node->add_child(key[depth + p], leaf);
    new_node->add_child(node->prefix[p], node);

    new_node->prefix_len = p;
    memcpy(new_node->prefix, node->prefix, p);

    node->prefix_len = node->prefix_len - p;
    memmove(node->prefix, node->prefix + p + 1, node->prefix_len);

    // replace
    node = new_node;
    return true;
  }

  // p == node->prefix_len
  depth += node->prefix_len;
  // find next
  Node *next = node->find_child(key[depth]);
  if (next) {
    return insert(next, key, leaf, depth + 1);
  } else {
    // TODO if full grow
    node->add_child(key[depth], leaf);
  }

  return true;
}

} // namespace arttree