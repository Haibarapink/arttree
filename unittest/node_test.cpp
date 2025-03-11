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

  ASSERT_EQ(n4->find_child('a'), &leaf);
  ASSERT_EQ(n4->find_child('b'), &leaf);
  ASSERT_EQ(n4->find_child('c'), &leaf);
  ASSERT_EQ(n4->find_child('d'), &leaf);
  ASSERT_EQ(n4->find_child('e'), nullptr);

  delete n4;
}

// 16
TEST(NodeTest, DISABLED_node16_test) {
  Node *n16 = Node::make_node(NodeType::Node16, "", "");
  Node leaf;
  for (int i = 0; i < 16; i++) {
    n16->add_child('a' + i, &leaf);
  }

  ASSERT_EQ(n16->add_child('z', &leaf), false);

  for (int i = 0; i < 16; i++) {
    ASSERT_EQ(n16->find_child('a' + i), &leaf);
  }

  delete n16;
}

// 48
TEST(NodeTest, DISABLED_node48_test) {
  Node *n48 = Node::make_node(NodeType::Node48, "", "");
  Node leaf;
  for (int i = 0; i < 48; i++) {
    n48->add_child('a' + i, &leaf);
  }

  ASSERT_EQ(n48->add_child('z', &leaf), false);

  for (int i = 0; i < 48; i++) {
    ASSERT_EQ(n48->find_child('a' + i), &leaf);
  }

  delete n48;
}

// 256
TEST(NodeTest, DISABLED_node256_test) {
  Node *n256 = Node::make_node(NodeType::Node256, "", "");
  Node leaf;
  for (int i = 0; i < 256; i++) {
    n256->add_child(i, &leaf);
  }

  ASSERT_EQ(n256->add_child('z', &leaf), false);

  for (int i = 0; i < 256; i++) {
    ASSERT_EQ(n256->find_child(i), &leaf);
  }

  delete n256;
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

  tree.insert("abcd", "abcd");
  ASSERT_EQ(tree.root_->is_leaf(), false);
  ASSERT_EQ(tree.root_->prefix_len, 3);
  ASSERT_EQ(tree.root_->find_child('d')->is_leaf(), true);
  ASSERT_EQ(tree.root_->find_child('a')->is_leaf(), true);

  tree.insert("abcde", "abcde");
  tree.insert("abcdf", "abcdf");



}

int main(int, char **) {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}