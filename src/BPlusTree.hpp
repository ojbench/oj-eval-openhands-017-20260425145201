#ifndef BPLUSTREE_HPP
#define BPLUSTREE_HPP
#include <iostream>
#include <fstream>
#include <cstring>
#include "utility.hpp"
namespace sjtu {
template <class Key, class Value, int M = 60>
class BPlusTree {
    struct Node {
        long pos; long parent; long children[M + 1]; Key keys[M]; Value values[M]; int size; bool is_leaf; long next;
        Node() : pos(-1), parent(-1), size(0), is_leaf(true), next(-1) { for (int i = 0; i <= M; ++i) children[i] = -1; }
    };
    char filename[64]; mutable std::fstream file; long root_pos;
    void read_node(long pos, Node &node) const { if (pos == -1) return; file.seekg(pos); file.read(reinterpret_cast<char *>(&node), sizeof(Node)); }
    void write_node(long pos, const Node &node) { file.seekp(pos); file.write(reinterpret_cast<const char *>(&node), sizeof(Node)); }
    long alloc_node() { file.seekp(0, std::ios::end); long pos = file.tellp(); Node node; node.pos = pos; file.write(reinterpret_cast<char *>(&node), sizeof(Node)); return pos; }
public:
    BPlusTree(const char *fname) {
        std::strcpy(filename, fname); file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) { file.open(filename, std::ios::out | std::ios::binary); file.close(); file.open(filename, std::ios::in | std::ios::out | std::ios::binary); root_pos = -1; file.seekp(0); file.write(reinterpret_cast<char *>(&root_pos), sizeof(long)); }
        else { file.seekg(0); file.read(reinterpret_cast<char *>(&root_pos), sizeof(long)); }
    }
    ~BPlusTree() { file.seekp(0); file.write(reinterpret_cast<char *>(&root_pos), sizeof(long)); file.close(); }
    void clear() { file.close(); file.open(filename, std::ios::out | std::ios::binary); file.close(); file.open(filename, std::ios::in | std::ios::out | std::ios::binary); root_pos = -1; file.seekp(0); file.write(reinterpret_cast<char *>(&root_pos), sizeof(long)); }
    bool empty() const { return root_pos == -1; }
    void insert(const Key &key, const Value &value) {
        if (root_pos == -1) { root_pos = alloc_node(); Node root; read_node(root_pos, root); root.is_leaf = true; root.size = 1; root.keys[0] = key; root.values[0] = value; write_node(root_pos, root); return; }
        long curr_pos = root_pos; Node curr;
        while (true) { read_node(curr_pos, curr); if (curr.is_leaf) break; int i = 0; while (i < curr.size && !(key < curr.keys[i])) i++; curr_pos = curr.children[i]; }
        int i = curr.size - 1; while (i >= 0 && key < curr.keys[i]) { curr.keys[i + 1] = curr.keys[i]; curr.values[i + 1] = curr.values[i]; i--; }
        curr.keys[i + 1] = key; curr.values[i + 1] = value; curr.size++;
        if (curr.size < M) write_node(curr.pos, curr); else split_leaf(curr);
    }
    void split_leaf(Node &node) {
        long newNodePos = alloc_node(); Node newNode; read_node(newNodePos, newNode); newNode.is_leaf = true; newNode.parent = node.parent; newNode.next = node.next; node.next = newNodePos;
        int mid = node.size / 2; newNode.size = node.size - mid; node.size = mid;
        for (int i = 0; i < newNode.size; ++i) { newNode.keys[i] = node.keys[mid + i]; newNode.values[i] = node.values[mid + i]; }
        write_node(node.pos, node); write_node(newNodePos, newNode); insert_into_parent(node.pos, newNode.keys[0], newNodePos);
    }
    void insert_into_parent(long left_pos, const Key &key, long right_pos) {
        Node left; read_node(left_pos, left);
        if (left.parent == -1) {
            long newRootPos = alloc_node(); Node newRoot; read_node(newRootPos, newRoot); newRoot.is_leaf = false; newRoot.size = 1; newRoot.keys[0] = key; newRoot.children[0] = left_pos; newRoot.children[1] = right_pos; root_pos = newRootPos; left.parent = newRootPos; write_node(left_pos, left); Node right; read_node(right_pos, right); right.parent = newRootPos; write_node(right_pos, right); write_node(newRootPos, newRoot); return;
        }
        long parent_pos = left.parent; Node parent; read_node(parent_pos, parent);
        int i = parent.size - 1; while (i >= 0 && key < parent.keys[i]) { parent.keys[i + 1] = parent.keys[i]; parent.children[i + 2] = parent.children[i + 1]; i--; }
        parent.keys[i + 1] = key; parent.children[i + 2] = right_pos; parent.size++;
        Node right; read_node(right_pos, right); right.parent = parent_pos; write_node(right_pos, right);
        if (parent.size < M) write_node(parent.pos, parent); else split_internal(parent);
    }
    void split_internal(Node &node) {
        long newNodePos = alloc_node(); Node newNode; read_node(newNodePos, newNode); newNode.is_leaf = false; newNode.parent = node.parent;
        int mid = node.size / 2; Key upKey = node.keys[mid]; newNode.size = node.size - mid - 1; node.size = mid;
        for (int i = 0; i <= newNode.size; ++i) { newNode.children[i] = node.children[mid + 1 + i]; if (i < newNode.size) newNode.keys[i] = node.keys[mid + 1 + i]; Node child; read_node(newNode.children[i], child); child.parent = newNodePos; write_node(newNode.children[i], child); }
        write_node(node.pos, node); write_node(newNodePos, newNode); insert_into_parent(node.pos, upKey, newNodePos);
    }
    bool find(const Key &key, Value &value) const {
        if (root_pos == -1) return false; long curr_pos = root_pos; Node curr;
        while (true) { read_node(curr_pos, curr); if (curr.is_leaf) break; int i = 0; while (i < curr.size && !(key < curr.keys[i])) i++; curr_pos = curr.children[i]; }
        for (int i = 0; i < curr.size; ++i) if (curr.keys[i] == key) { value = curr.values[i]; return true; }
        return false;
    }
    void update(const Key &key, const Value &value) {
        if (root_pos == -1) return; long curr_pos = root_pos; Node curr;
        while (true) { read_node(curr_pos, curr); if (curr.is_leaf) break; int i = 0; while (i < curr.size && !(key < curr.keys[i])) i++; curr_pos = curr.children[i]; }
        for (int i = 0; i < curr.size; ++i) if (curr.keys[i] == key) { curr.values[i] = value; write_node(curr.pos, curr); return; }
    }
    void range_search(const Key &key_low, const Key &key_high, sjtu::vector<sjtu::pair<Key, Value>> &res) const {
        if (root_pos == -1) return; long curr_pos = root_pos; Node curr;
        while (true) { read_node(curr_pos, curr); if (curr.is_leaf) break; int i = 0; while (i < curr.size && !(key_low < curr.keys[i])) i++; curr_pos = curr.children[i]; }
        while (curr_pos != -1) { read_node(curr_pos, curr); for (int i = 0; i < curr.size; ++i) { if (key_high < curr.keys[i]) return; if (!(curr.keys[i] < key_low)) res.push_back(sjtu::pair<Key, Value>(curr.keys[i], curr.values[i])); } curr_pos = curr.next; }
    }
    void remove(const Key &key, const Value &value) {
        if (root_pos == -1) return; long curr_pos = root_pos; Node curr;
        while (true) { read_node(curr_pos, curr); if (curr.is_leaf) break; int i = 0; while (i < curr.size && !(key < curr.keys[i])) i++; curr_pos = curr.children[i]; }
        for (int i = 0; i < curr.size; ++i) if (curr.keys[i] == key && curr.values[i] == value) {
            for (int j = i; j < curr.size - 1; ++j) { curr.keys[j] = curr.keys[j + 1]; curr.values[j] = curr.values[j + 1]; }
            curr.size--; write_node(curr.pos, curr); return;
        }
    }
};
}
#endif
