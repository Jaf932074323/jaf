#pragma once
// MIT License
//
// Copyright(c) 2021 Jaf932074323
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 2024-6-23 姜安富
#include <utility>

namespace jaf
{

enum class NodeColor
{
    NC_RED,
    NC_BLACK,
};

// 红黑树节点
template <typename Key, class Value>
struct RedBlackTreeNode
{
    RedBlackTreeNode<K, V>* left_   = nullptr;
    RedBlackTreeNode<K, V>* right_  = nullptr;
    RedBlackTreeNode<K, V>* parent_ = nullptr;
    std::pair<Key, Value> key_value_;
    NodeColor node_color_;

    RBTreeNode(const pair<K, V>& key_value)
        : key_value_(key_value)
        , node_color_(NodeColor::RED)
    {
    }
};

// 红黑树
template <typename Key, class Value>
class RedBlackTree
{
    typedef RedBlackTreeNode<Key, Value> Node;

public:
    // min_level通过的最低日志等级
    RedBlackTree() {}
    ~RedBlackTree() {}

    bool Insert(const std::pair<Key, Value>& key_value);


private:
    Node* root_ = nullptr;
};

template<typename Key, class Value>
bool RedBlackTree<Key, Value>::Insert(const std::pair<Key, Value>& key_value)
{
    if (root_ == nullptr)
    {
        root_ = new Node(key_value);
        return true;
    }

    Node* parent = nullptr;
    Node* cur = root_;
    while (cur)
    {
        if (cur->key_value_.first < kv.first) // 插入节点比当前节点大往右走, 小往左走
        {
            parent = cur;
            cur = cur->_right;
        }
        else if (cur->_kv.first > kv.first)
        {
            parent = cur;
            cur = cur->_left;
        }
        else
        {
            return false;
        }
    }

    // 链接
    cur = new Node(kv);
    if (parent->_kv.first > kv.first)
    {
        parent->_left = cur;
    }
    else
    {
        parent->_right = cur;
    }

    // new的节点的parent还指向空
    cur->_parent = parent;

    // 插入黑色节点还是红色节点？

    return true;
}

} // namespace jaf