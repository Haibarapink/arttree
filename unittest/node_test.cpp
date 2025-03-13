#include <gtest/gtest.h>
#define private public
#include "../art.hpp"

using namespace arttree;

TEST(NodeTest, node4_test) {
  Node *n4 = Node::make_node(NodeType::Node4, "", "");
  Node leaf;
  n4->add_child('a', &leaf);
  n4->add_child('b', &leaf);
  n4->add_child('c', &leaf);
  n4->add_child('d', &leaf);

  ASSERT_EQ(*n4->find_child('a'), &leaf);
  ASSERT_EQ(*n4->find_child('b'), &leaf);
  ASSERT_EQ(*n4->find_child('c'), &leaf);
  ASSERT_EQ(*n4->find_child('d'), &leaf);
  ASSERT_EQ(n4->find_child('e'), nullptr);

  delete n4;
}

// 16
TEST(NodeTest, node16_test) {
  Node *n16 = Node::make_node(NodeType::Node16, "", "");
  Node leaf;
  for (int i = 0; i < 16; i++) {
    n16->add_child('a' + i, &leaf);
  }

  ASSERT_EQ(n16->add_child('z', &leaf), false);

  for (int i = 0; i < 16; i++) {
    ASSERT_EQ(*n16->find_child('a' + i), &leaf);
  }

  delete n16;
}

// 48
TEST(NodeTest, node48_test) {
  Node *n48 = Node::make_node(NodeType::Node48, "", "");
  Node leaf;
  for (int i = 0; i < 48; i++) {
    ASSERT_TRUE(n48->add_child('a' + i, &leaf));
  }

  ASSERT_FALSE(n48->add_child('z', &leaf));

  for (int i = 0; i < 48; i++) {
    ASSERT_EQ(*n48->find_child('a' + i), &leaf);
  }

  delete n48;
}

// 256
TEST(NodeTest, node256_test) {
  Node *n256 = Node::make_node(NodeType::Node256, "", "");
  Node leaf;
  for (int i = 0; i < 256; i++) {
    n256->add_child(i, &leaf);
  }

  ASSERT_EQ(n256->add_child('z', &leaf), false);

  for (int i = 0; i < 256; i++) {
    ASSERT_EQ(*n256->find_child(i), &leaf);
  }

  delete n256;
}

TEST(NodeTest, grow_test) {
  Node *n = Node::make_node(NodeType::Node4, "", "");
  // 插满
  std::vector<Node *> children;
  int i = 0;
  for (; i < 4; i++) {
    children.push_back(new Node{});
    n->add_child('a' + i, children[i]);
  }
  n->grow();

  // 迭代检查
  int j = 0;
  for (; j < 4; j++) {
    ASSERT_EQ(*n->find_child('a' + j), children[j]);
  }

  // 4 -> 16
  // 插满
  for (; i < 16; i++) {
    children.push_back(new Node{});
    n->add_child('a' + i, children[i]);
  }
  n->grow();

  // 迭代检查
  for (; j < 16; j++) {
    ASSERT_EQ(*n->find_child('a' + j), children[j]);
  }
  // 16 -> 48
  // 插满
  for (; i < 48; i++) {
    children.push_back(new Node{});
    std::cout << "add child " <<  i << " " << children[i]
              << std::endl;
    if (i == 47) {
      int m = 0;
    }
    n->add_child('a' + i, children[i]);
  }

  n->grow();
  LOG_INFO << "grow 16 -> 48";

  // 迭代检查
  for (; j < 48; j++) {
    ASSERT_EQ(*n->find_child('a' + j), children[j]);
  }

  // 48 -> 256
  // 插满
  for (; i < 256; i++) {
    children.push_back(new Node{});
    n->add_child('a' + i, children[i]);
  }

  // 迭代检查
  for (; j < 256; j++) {
    ASSERT_EQ(*n->find_child('a' + j), children[j]);
  }

  ASSERT_EQ(n->type, NodeType::Node16);

  delete n;
}

TEST(NodeTest, node_leaf_test) {
  Node *leaf = Node::make_node(NodeType::Leaf, "key", "val");
  NodeLeaf *leaf2 = (NodeLeaf *)leaf->inner;
  ASSERT_EQ(leaf->load_key(), "key");
  ASSERT_EQ(leaf2->load_val(), "val");
  delete leaf;
}

TEST(NodeTest, node_insert_test) {
  ArtTree tree;
  tree.insert("abc", "abc");
  ASSERT_EQ(tree.root_->is_leaf(), true);
  ASSERT_EQ(tree.root_->load_key(), "abc");

  //  tree.print();

  tree.insert("abcd", "abcd");
  //  tree.print();
  std::string_view val;
  tree.search("abcd", val);
  ASSERT_EQ(val, "abcd");
  tree.search("abc", val);
  ASSERT_EQ(val, "abc");

  tree.insert("abcde", "abcde");
  tree.insert("abcdf", "abcdf");

  tree.search("abcde", val);
  ASSERT_EQ(val, "abcde");
  tree.search("abcdf", val);
  ASSERT_EQ(val, "abcdf");

  tree.print();
  int i;
}

int main(int, char **) {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}