#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <string_view>

#define ENABLE_LOGGING
#include "logger.hpp"

namespace arttree {

/**
 * \class Bitmap
 * \brief A class to handle bitmap operations.
 */
class Bitmap {
public:
  Bitmap() = default;
  Bitmap(unsigned char *bytes, size_t size) : bytes(bytes), size(size) {}

  /**
   * \brief Get the bit at the specified index.
   * \param index The index of the bit.
   * \return True if the bit is set, false otherwise.
   */
  bool get_bit(size_t index) const {
    if (index / 8 >= size) {
      return false;
    }
    return (bytes[index / 8] & (1 << (index % 8))) != 0;
  }

  /**
   * \brief Set the bit at the specified index.
   * \param index The index of the bit.
   */
  void set_bit(size_t index) {
    if (index / 8 >= size) {
      return;
    }
    bytes[index / 8] |= (1 << (index % 8));
  }

  /**
   * \brief Clear the bit at the specified index.
   * \param index The index of the bit.
   */
  void clear_bit(size_t index) {
    if (index / 8 >= size) {
      return;
    }
    bytes[index / 8] &= ~(1 << (index % 8));
  }

  /**
   * \brief Flip the bit at the specified index.
   * \param index The index of the bit.
   */
  void flip_bit(size_t index) {
    if (index / 8 >= size) {
      return;
    }
    bytes[index / 8] ^= (1 << (index % 8));
  }

private:
  unsigned char *bytes;
  size_t size;
};

/**
 * \struct ArtTreeDefs
 * \brief Definitions for the ART tree.
 */
struct ArtTreeDefs {
  static constexpr int MAX_PREFIX_LEN = 16;
};

/**
 * \enum NodeType
 * \brief Enum for different types of nodes in the ART tree.
 */
enum class NodeType { Node4 = 0, Node16, Node48, Node256, Leaf, Invalid };

struct Node;

/**
 * \struct NodeLeaf
 * \brief A structure representing a leaf node in the ART tree.
 */
struct NodeLeaf {
  size_t key_len, val_len;
  unsigned char *raw;

  NodeLeaf(std::string_view k, std::string_view v) {
    key_len = k.size();
    val_len = v.size();

    raw = new unsigned char[key_len + val_len];
    memcpy(raw, k.data(), key_len);
    memcpy(raw + key_len, v.data(), val_len);
  }

  /**
   * \brief Load the key from the leaf node.
   * \return The key as a string view.
   */
  inline std::string_view load_key() const { return {(char *)raw, key_len}; }

  /**
   * \brief Load the value from the leaf node.
   * \return The value as a string view.
   */
  inline std::string_view load_val() const {
    return {(char *)(raw + key_len), val_len};
  }

  ~NodeLeaf() { delete[] raw; }
};

/**
 * \class Node4
 * \brief A class representing a Node4 in the ART tree.
 */
class Node4 {
public:
  unsigned char key[5]{};
  Node *children[4]{};

  Node4() {
    memset(children, 0, sizeof(Node *) * 4);
    key[4] = 0;
    bitmap_ = Bitmap(key + 4, 1);
  }

  /**
   * \brief Add a child to the node.
   * \param ch The unsigned character key of the child.
   * \param child The child node.
   * \param is_leaf Whether the child is a leaf node.
   * \return True if the child was added, false otherwise.
   */
  inline bool add_child(unsigned char ch, Node *child, bool is_leaf = false) {
    for (size_t i = 0; i < 4; ++i) {
      if (children[i] == nullptr) {
        children[i] = child;
        key[i] = ch;
        if (is_leaf) {
          bitmap_.set_bit(i);
        }
        return true;
      }
    }
    return false;
  }

  /**
   * \brief Find a child node.
   * \param ch The unsigned character key of the child.
   * \param is_leaf Whether the child is a leaf node.
   * \return A pointer to the child node, or nullptr if not found.
   */
  inline Node **find_child(unsigned char ch, bool is_leaf = false) {
    if (is_leaf) {
      for (size_t i = 0; i < 4; ++i) {
        if (key[i] == ch && bitmap_.get_bit(i)) {
          return &children[i];
        }
      }
      return nullptr;
    }

    for (size_t i = 0; i < 4; ++i) {
      if (key[i] == ch) {
        return &children[i];
      }
    }
    return nullptr;
  }

private:
  Bitmap bitmap_;
};

/**
 * \class Node16
 * \brief A class representing a Node16 in the ART tree.
 */
class Node16 {
public:
  unsigned char key[16]{};
  Node *children[16]{};

  Node16() { memset(children, 0, sizeof(Node *) * 16); }

  /**
   * \brief Add a child to the node.
   * \param ch The unsigned character key of the child.
   * \param child The child node.
   * \return True if the child was added, false otherwise.
   */
  inline bool add_child(unsigned char ch, Node *child) {
    for (size_t i = 0; i < 16; ++i) {
      if (children[i] == nullptr) {
        children[i] = child;
        key[i] = ch;
        return true;
      }
    }
    return false;
  }

  /**
   * \brief Find a child node.
   * \param ch The unsigned character key of the child.
   * \return A pointer to the child node, or nullptr if not found.
   */
  inline Node **find_child(unsigned char ch) {
    for (size_t i = 0; i < 16; ++i) {
      if (key[i] == ch) {
        return &children[i];
      }
    }
    return nullptr;
  }
};

/**
 * \class Node48
 * \brief A class representing a Node48 in the ART tree.
 */
class Node48 {
public:
  Node *children[48]{};
  uint8_t child_index[256]{};

  Node48() {
    memset(children, 0, sizeof(Node *) * 48);
    memset(child_index, 0xFF, sizeof(child_index));
  }

  /**
   * \brief Add a child to the node.
   * \param ch The unsigned character key of the child.
   * \param child The child node.
   * \return True if the child was added, false otherwise.
   */
  inline bool add_child(unsigned char ch, Node *child) {
    for (size_t i = 0; i < 48; ++i) {
      if (children[i] == nullptr) {
        children[i] = child;
        uint8_t idx = static_cast<uint8_t>(ch);
        child_index[idx] = i;
        return true;
      }
    }
    LOG_WARNING << "Node48 is full";
    return false;
  }

  /**
   * \brief Find a child node.
   * \param ch The unsigned character key of the child.
   * \return A pointer to the child node, or nullptr if not found.
   */
  inline Node **find_child(unsigned char ch) {
    uint8_t index = child_index[static_cast<uint8_t>(ch)];
    if (index == 0xFF) {
      return nullptr;
    }
    return &children[index];
  }
};

/**
 * \class Node256
 * \brief A class representing a Node256 in the ART tree.
 */
class Node256 {
public:
  Node *children[256]{};

  Node256() { memset(children, 0, sizeof(Node *) * 256); }

  /**
   * \brief Add a child to the node.
   * \param ch The unsigned character key of the child.
   * \param child The child node.
   * \return True if the child was added, false otherwise.
   */
  inline bool add_child(unsigned char ch, Node *child) {
    uint8_t index = static_cast<uint8_t>(ch);
    if (children[index] == nullptr) {
      children[index] = child;
      return true;
    }
    return false;
  }

  /**
   * \brief Find a child node.
   * \param ch The unsigned character key of the child.
   * \return A pointer to the child node, or nullptr if not found.
   */
  inline Node **find_child(unsigned char ch) {
    uint8_t index = static_cast<uint8_t>(ch);
    if (children[index] != nullptr) {
      return &children[index];
    }
    return nullptr;
  }
};

/**
 * \struct Node
 * \brief A structure representing a node in the ART tree.
 */
struct Node {
  NodeType type{NodeType::Invalid};
  unsigned char prefix[ArtTreeDefs::MAX_PREFIX_LEN];
  size_t prefix_len{0};
  void *inner{nullptr};

  void grow() {
    switch (type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)inner;
      Node16 *n16 = new Node16{};
      for (size_t i = 0; i < 4; i++) {
        n16->add_child(n4->key[i], n4->children[i]);
      }
      delete n4;
      inner = n16;
      type = NodeType::Node16;
    } break;
    case NodeType::Node16: {
      Node16 *n16 = (Node16 *)inner;
      Node48 *n48 = new Node48{};
      for (size_t i = 0; i < 16; i++) {
        if (n16->children[i]) {
          n48->add_child(n16->key[i], n16->children[i]);
        }
      }
      delete n16;
      inner = n48;
      type = NodeType::Node48;
    } break;
    case NodeType::Node48: {
      Node48 *n48 = (Node48 *)inner;
      Node256 *n256 = new Node256{};
      // TODO: fix bug, implement a iterator
      for (size_t i = 0; i < 48; i++) {
        if (n48->children[i] != nullptr) {
          n256->add_child(i, n48->children[i]);
        }
      }
      delete n48;
      inner = n256;
      type = NodeType::Node256;
    } break;
    case NodeType::Node256:
      assert(false && "Node256 can't grow");
      break;
    case NodeType::Leaf:
      assert(false && "Leaf node can't grow");
      break;
    default:
      assert(false && "Invalid node type");
    }
  }

  bool is_full() {
    switch (type) {
    case NodeType::Node4:
      return false;
    case NodeType::Node16:
      return false;
    case NodeType::Node48:
      return false;
    case NodeType::Node256:
      return false;
    case NodeType::Leaf:
      return true;
    default:
      assert(false && "Invalid node type");
    }
    return false;
  }

  ~Node() {
    if (inner) {
      switch (type) {
      case NodeType::Node4:
        delete (Node4 *)inner;
        break;
      case NodeType::Node16:
        delete (Node16 *)inner;
        break;
      case NodeType::Node48:
        delete (Node48 *)inner;
        break;
      case NodeType::Node256:
        delete (Node256 *)inner;
        break;
      case NodeType::Leaf:
        delete (NodeLeaf *)inner;
        break;
      default:
        assert(false && "Invalid node type");
      }
    }
  }

  /**
   * \brief Check if the node is a leaf.
   * \return True if the node is a leaf, false otherwise.
   */
  inline bool is_leaf() const { return type == NodeType::Leaf; }

  /**
   * \brief Load the key from the node.
   * \return The key as a string view.
   */
  inline std::string_view load_key() const {
    assert(this->type == NodeType::Leaf);
    auto leaf = (NodeLeaf *)inner;
    return leaf->load_key();
  }

  /**
   * \brief Check the prefix of the node.
   * \param key The key to check against.
   * \param depth The depth to start checking from.
   * \return The length of the matching prefix.
   */
  size_t check_prefix(std::string_view key, size_t depth) {
    size_t i = depth;

    for (; i < prefix_len && i < key.size() && key[i] == prefix[i]; ++i) {
    }

    return i - depth;
  }

  /**
   * \brief Find a child node.
   * \param ch The unsigned character key of the child.
   * \param is_leaf Whether the child is a leaf node.
   * \return A pointer to the child node, or nullptr if not found.
   */
  Node **find_child(unsigned char ch, bool is_leaf = false) {
    switch (type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)inner;
      return n4->find_child(ch, is_leaf);
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

  /**
   * \brief Add a child node.
   * \param ch The unsigned character key of the child.
   * \param n The child node.
   * \param is_leaf Whether the child is a leaf node.
   * \return True if the child was added, false otherwise.
   */
  bool add_child(unsigned char ch, Node *n, bool is_leaf = false) {
    switch (type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)inner;
      return n4->add_child(ch, n, is_leaf);
    } break;
    case NodeType::Node16: {
      Node16 *n16 = (Node16 *)inner;
      return n16->add_child(ch, n);
    } break;
    case NodeType::Node48: {
      Node48 *n48 = (Node48 *)inner;
      return n48->add_child(ch, n);
    } break;
    case NodeType::Node256: {
      Node256 *n256 = (Node256 *)inner;
      return n256->add_child(ch, n);
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

  /**
   * \brief Create a new node.
   * \param type The type of the node.
   * \param leaf_key The key for the leaf node.
   * \param leaf_val The value for the leaf node.
   * \return A pointer to the new node.
   */
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
      break;
    default:
      delete n;
      assert(false && "Invalid node type");
      return nullptr;
    }
    return n;
  }
};

/**
 * \class ArtTree
 * \brief A class representing an Adaptive Radix Tree (ART).
 */
class ArtTree {
public:
  ~ArtTree() { destory(root_, -1, 0); }

  /**
   * \brief Insert a key-value pair into the ART.
   * \param key The key to insert.
   * \param val The value to insert.
   * \return True if the insertion was successful, false otherwise.
   */
  bool insert(std::string_view key, std::string_view val);

  /**
   * \brief Search for a key in the ART.
   * \param key The key to search for.
   * \param val The value associated with the key.
   * \return True if the key was found, false otherwise.
   */
  bool search(std::string_view key, std::string_view &val) const;

private:
  bool recursive_insert(Node **node_ref, const std::string_view &key,
                        Node *leaf, size_t depth);

  /**
   * \brief Destroy the ART.
   * \param cur The current node.
   * \param parent_id The parent node ID.
   * \param id The current node ID.
   */
  void destory(Node *cur, int parent_id, int id) {
    if (cur == nullptr) {
      return;
    }

    bool is_leaf = cur->is_leaf();
    std::string_view print_value =
        cur->is_leaf() ? cur->load_key()
                       : std::string_view{(char *)cur->prefix, cur->prefix_len};
    std::string_view type = cur->is_leaf() ? "leaf" : "inner";
    LOG_WARNING << type << " pid " << parent_id << " id " << id << " prefix => "
                << print_value;

    switch (cur->type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)cur->inner;
      for (size_t i = 0; i < 4; i++) {
        if (n4->children[i]) {
          destory(n4->children[i], id, id + i + 1);
        }
      }
    } break;
    default:
      // TODO
      break;
    }
    delete cur;
  }

  /**
   * \brief Print the ART.
   */
  void print() { print(root_, -1, 0); }

  /**
   * \brief Print a node in the ART.
   * \param cur The current node.
   * \param parent_id The parent node ID.
   * \param id The current node ID.
   */
  void print(Node *cur, int parent_id, int id) {
    if (cur == nullptr) {
      return;
    }
    bool is_leaf = cur->is_leaf();
    std::string_view print_value =
        cur->is_leaf() ? cur->load_key()
                       : std::string_view{(char *)cur->prefix, cur->prefix_len};
    std::string_view type = cur->is_leaf() ? "leaf" : "inner";
    LOG_INFO << type << " pid " << parent_id << " id " << id << " prefix => "
             << print_value;

    switch (cur->type) {
    case NodeType::Node4: {
      Node4 *n4 = (Node4 *)cur->inner;
      for (size_t i = 0; i < 4; i++) {
        if (n4->children[i]) {
          print(n4->children[i], id, id + i + 1);
        }
      }
    }
    default:
      break;
    }
  }

  Node *root_{nullptr};
};

inline bool ArtTree::search(std::string_view key, std::string_view &val) const {
  Node *cur = root_;
  size_t depth = 0;
  while (cur) {
    if (cur->is_leaf()) {
      if (cur->load_key() == key) {
        auto *leaf = (NodeLeaf *)cur->inner;
        val = leaf->load_val();
        return true;
      }
      return false;
    }

    size_t p = cur->check_prefix(key, depth);
    if (p != cur->prefix_len) {
      return false;
    }

    depth += cur->prefix_len;
    assert(depth <= key.size());
    unsigned char ch = depth < key.size() ? key[depth] : char{0};
    Node **next = cur->find_child(ch, depth == key.size());
    if (next == nullptr) {
      return false;
    }
    cur = *next;
    depth++;
  }

  return false;
}

inline bool ArtTree::insert(std::string_view key, std::string_view val) {
  Node *leaf = Node::make_node(NodeType::Leaf, key, val);
  return recursive_insert(&root_, key, leaf, 0);
}

inline bool ArtTree::recursive_insert(Node **node_ref,
                                      const std::string_view &key, Node *leaf,
                                      size_t depth) {
  if (*node_ref == nullptr) {
    *node_ref = leaf;
    return true;
  }

  Node *node = *node_ref;

  if (node->is_leaf()) {
    if (node->load_key() == key) {
      // TODO just update
      Node *old = node;
      node = leaf;
      delete old;
      return true;
    }

    Node *new_node = Node::make_node(NodeType::Node4, "", "");
    std::string_view key2 = node->load_key();
    size_t i = depth;
    // new_node's prefix is common prefix of key and key2
    for (; key[i] == key2[i]; i++) {
      new_node->prefix[i - depth] = key[i];
    }
    new_node->prefix_len = i - depth;
    depth = depth + new_node->prefix_len;
    // node's prefix is "abc" and recursive_insert "abcd".
    // In this case, new_node's prefix is "abc",
    // key2 is "abc", so new_node's prefix_len is as same as key2.size()
    new_node->add_child(key[depth], leaf, true);
    new_node->add_child(0, node, true);
    // replace
    *node_ref = new_node;
    return true;
  }

  size_t p = node->check_prefix(key, depth);
  if (p != node->prefix_len) {
    // prefix mismatch
    assert(p < node->prefix_len);
    Node *new_node = Node::make_node(NodeType::Node4, "", "");
    new_node->add_child(key[depth + p], leaf, true);
    new_node->add_child(node->prefix[p], node);

    new_node->prefix_len = p;
    memcpy(new_node->prefix, node->prefix, p);

    node->prefix_len = node->prefix_len - p;
    memmove(node->prefix, node->prefix + p + 1, node->prefix_len);

    LOG_INFO << "prefix mismatch, create new node4" << new_node->prefix << " "
             << new_node->prefix_len;
    // replace
    *node_ref = new_node;
    return true;
  }

  // p == node->prefix_len
  depth += node->prefix_len;
  // find next
  Node **next = node->find_child(key[depth]);

  LOG_INFO << "find next node" << key[depth] << " " << depth;
  if (next) {
    return recursive_insert(next, key, leaf, depth + 1);
  } else {
    if (node->is_full()) {
      node->grow();
    }
    node->add_child(key[depth], leaf, true);
  }

  return true;
}

} // namespace arttree