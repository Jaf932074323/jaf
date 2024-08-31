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
// 2024-8-16 ������
#include <assert.h>
#include <stddef.h>
#include <utility>

namespace jaf
{

// �����
// �������ظ�
// �������е�ÿ���ڵ㣬�ڲ���֮��ɾ��֮ǰ�������ֽڵ��ַ��Ч����
// TODO:�кܶ�ϸ�ڻ���Ҫ�Ժ�����
template <typename Key, typename Value>
class RedBlackTree
{
#ifdef TEST_RED_BLACK_TREE
    friend class TestRedBlackTree;
#endif

public:
    enum Color
    {
        COLOR_BLACK,
        COLOR_RED,
    };

    struct Node
    {
        ~Node()
        {
        }
        Key key_;
        Value value_;
        Color color_;
        Node* parent_{nullptr};
        Node* left_child_{nullptr};
        Node* right_child_{nullptr};
    };

public:
    class Iterator
    {
    public:
        Iterator(RedBlackTree* tree, Node* cur);
        inline Iterator& operator++();
        inline Iterator& operator--();
        //����==�����
        inline bool operator==(const Iterator& other) const
        {
            return other.cur_ == cur_;
        }
        //����!=�����
        inline bool operator!=(const Iterator& that) const
        {
            return !(*this == that);
        }

        Node* GetNode() noexcept
        {
            return cur_;
        }
        const Node* GetNode() const noexcept
        {
            return cur_;
        }

        inline Key& GetKey() const noexcept
        {
            assert(cur_ != nullptr);
            return cur_->key_;
        }
        inline Value& GetValue() const noexcept
        {
            assert(cur_ != nullptr);
            return cur_->value_;
        }

    private:
        Node* cur_; // ��ǰ�ڵ�
        RedBlackTree* tree_;
    };


public:
    RedBlackTree()                               = default;
    ~RedBlackTree()                              = default;
    RedBlackTree(const RedBlackTree&)            = delete; // ������Ϊ���ӣ�Ŀǰ�Ƚ�ֹ
    RedBlackTree& operator=(const RedBlackTree&) = delete; // ������Ϊ���ӣ�Ŀǰ�Ƚ�ֹ
public:
    inline size_t Size()
    {
        return size_;
    }
    inline bool Empty()
    {
        return size_ == 0;
    }
    Iterator Insert(const Key& key, const Value& value);
    void Erase(const Key& key);
    Iterator Erase(Node* node);
    Iterator Erase(Iterator& it);
    Iterator Erase(Iterator& first, Iterator& last); // �Ƴ�[first,last)

    Iterator Find(const Key& key);
    Iterator LowerBound(const Key& key); // ��ȡ��һ�����ڵ���key�ڵ�ĵ�����,������ʱ����end()
    Iterator UpperBound(const Key& key); // ��ȡ��һ������key�ڵ�ĵ�����,������ʱ����end()

private:
    Node* LowerBoundNode(const Key& key); // ��ȡ��һ�����ڵ���key�Ľڵ�,������ʱ����end()
    Node* UpperBoundNode(const Key& key); // ��ȡ��һ������key�Ľڵ�,������ʱ����end()

    Node* NestNode(Node* cur_node);
    Node* LastNode(Node* cur_node);


private:
    Node* GetRoot()
    {
        return root_;
    }

private:
    void AdjustAfterInsert(Node* node);      // ����Ԫ�غ����
    void AdjusBeforeDelete(Node* dele_node); // ɾ��Ԫ��֮ǰ����
    inline void LeftRotate(Node* node);      // ����
    inline void RightRotate(Node* node);     // ����
    inline Node* SeekFormer(Node* node);     // �������ڵ�ǰһ��Ԫ��
    inline Node* SeekLater(Node* node);      // �������ڵĺ�һ��Ԫ��
    inline static Node* Min(Node* node);     // ����node�µ���С�ڵ�
    inline static Node* Max(Node* node);     // ����node�µ������ڵ�

    // �Ƿ�������
    inline static bool IsLeftChild(Node* node)
    {
        assert(node->parent_ != nullptr);
        return node->parent_->left_child_ == node;
    }
    // �Ƿ����Һ���
    inline static bool IsRightChild(Node* node)
    {
        assert(node->parent_ != nullptr);
        return node->parent_->right_child_ == node;
    }
    // �Ƿ��Ǻ�ɫ�ڵ�
    inline static bool IsRed(Node* node)
    {
        return node != nullptr && node->color_ == Color::COLOR_RED;
    }
    // �Ƿ��Ǻ�ɫ�ڵ�
    inline static bool IsBlack(Node* node)
    {
        return node == nullptr || node->color_ == Color::COLOR_BLACK;
    }

private:
    inline static Node* GetBrother(Node* node);

public:
    inline Iterator begin()
    {
        return Iterator(this, min_);
    }
    inline Iterator end()
    {
        return Iterator(this, nullptr);
    }

    // ��ѡ���ṩһ����Ա�������������涨�����Ԫ����
    void Swap(RedBlackTree& other) noexcept
    {
        std::swap(root_, other.root_);
        std::swap(min_, other.min_);
        std::swap(max_, other.max_);
        std::swap(size_, other.size_);
    }

private:
    Node* root_  = nullptr; // ���ڵ�
    Node* min_   = nullptr; // ��С�ڵ�
    Node* max_   = nullptr; // ���ڵ�
    size_t size_ = 0;
};

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Insert(const Key& key, const Value& value) -> class RedBlackTree<Key, Value>::Iterator
{
    if (root_ == nullptr)
    {
        root_ = new Node{key, value, Color::COLOR_BLACK};
        ++size_;
        min_ = root_;
        max_ = root_;
        return Iterator(this, root_);
    }

    Node* new_node = nullptr;

    // �Ȳ���ڵ�,����ڵ�ֻ����Ҷ�ӽڵ�
    Node* parent = root_;
    while (true)
    {
        if (key < parent->key_)
        {
            if (parent->left_child_ == nullptr)
            {
                new_node            = new Node{key, value, Color::COLOR_RED, parent};
                parent->left_child_ = new_node;
                if (min_ == parent)
                {
                    min_ = new_node;
                }
                break;
            }
            parent = parent->left_child_;
        }
        else
        {
            if (parent->right_child_ == nullptr)
            {
                new_node             = new Node{key, value, Color::COLOR_RED, parent};
                parent->right_child_ = new_node;
                if (max_ == parent)
                {
                    max_ = new_node;
                }
                break;
            }
            parent = parent->right_child_;
        }
    }

    // �ٵ���
    AdjustAfterInsert(new_node);
    ++size_;
    return Iterator(this, new_node);
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::Erase(const Key& key)
{
    if (root_ == nullptr)
    {
        return;
    }

    auto first = LowerBoundNode(key);
    auto last  = UpperBoundNode(key);

    while (first != last)
    {
        assert(first != nullptr);
        Node* next = NestNode(first);
        AdjusBeforeDelete(first);
        delete first;
        --size_;
        first = next;
    }

    return;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Erase(Node* node) -> class RedBlackTree<Key, Value>::Iterator
{
    if (node == nullptr)
    {
        return Iterator(this, nullptr);
    }
    Iterator it(this, node);
    ++it;
    AdjusBeforeDelete(node);
    delete node;
    --size_;
    return it;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Erase(RedBlackTree<Key, Value>::Iterator& it) -> class RedBlackTree<Key, Value>::Iterator
{
    return Erase(it.GetNode());
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Erase(Iterator& first, Iterator& last) -> class RedBlackTree<Key, Value>::Iterator
{
    for (Iterator it = first; it != last;)
    {
        it = Erase(it);
    }
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Find(const Key& key) -> class RedBlackTree<Key, Value>::Iterator
{
    if (root_ == nullptr)
    {
        return Iterator(this, nullptr);
    }

    Node* node = root_;
    while (true)
    {
        if (node->key_ == key)
        {
            return Iterator(this, node);
        }

        if (key < node->key_)
        {
            node = node->left_child_;
        }
        else
        {
            node = node->right_child_;
        }

        // �����ڶ�Ӧ���Ľڵ�
        if (node == nullptr)
        {
            return Iterator(this, nullptr);
        }
    }
} template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::LowerBound(const Key& key) -> class RedBlackTree<Key, Value>::Iterator
{
    return Iterator(this, LowerBoundNode(key));
} template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::UpperBound(const Key& key) -> class RedBlackTree<Key, Value>::Iterator
{
    return Iterator(this, UpperBoundNode(key));
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::LowerBoundNode(const Key& key) -> struct RedBlackTree<Key, Value>::Node*
{
    if (root_ == nullptr)
    {
        return nullptr;
    }

    Node* aim_node = nullptr;
    Node* node     = root_;
    while (node != nullptr)
    {
        if (node->key_ < key)
        {
            node = node->right_child_;
        }
        else
        {
            aim_node = node;
            node     = node->left_child_;
        }
    }

    return aim_node;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::UpperBoundNode(const Key& key) -> struct RedBlackTree<Key, Value>::Node*
{
    if (root_ == nullptr)
    {
        return nullptr;
    }

    Node* aim_node = nullptr;
    Node* node     = root_;
    while (node != nullptr)
    {
        if (key < node->key_)
        {
            aim_node = node;
            node     = node->left_child_;
        }
        else
        {
            node = node->right_child_;
        }
    }

    return aim_node;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::NestNode(Node* cur_node) -> RedBlackTree<Key, Value>::Node*
{
    assert(cur_node != nullptr); // end������������������

    if (cur_node->right_child_ != nullptr)
    {
        return RedBlackTree::Min(cur_node->right_child_);
    }

    while (true)
    {
        if (cur_node->parent_ == nullptr)
        {
            return nullptr;
        }
        if (RedBlackTree::IsLeftChild(cur_node))
        {
            return cur_node->parent_;
        }
        cur_node = cur_node->parent_;
    }
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::LastNode(Node* cur_node) -> RedBlackTree<Key, Value>::Node*
{
    if (cur_node == nullptr)
    {
        return max_;
    }

    if (cur_node->left_child_ != nullptr)
    {
        return RedBlackTree::Max(cur_node->right_child_);
    }

    while (true)
    {
        if (cur_node->parent_ == nullptr)
        {
            return nullptr;
        }
        if (RedBlackTree::IsRightChild(cur_node))
        {
            return cur_node->parent_;
        }
        cur_node = cur_node->parent_;
    }
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::AdjustAfterInsert(Node* node)
{
    Node* parent      = nullptr; // ���ڵ�
    Node* grandparent = nullptr; // �游�ڵ�
    Node* uncle_node  = nullptr; // ����ڵ�

    while (true)
    {
        assert(node->parent_ != nullptr); // node�������Ǹ��ڵ�
        if (node->parent_->color_ == Color::COLOR_BLACK)
        {
            return;
        }
        assert(node->parent_->parent_ != nullptr); // �����ڵ��Ǻ�ɫʱ�����ڵ�Ҳ�������Ǹ��ڵ�

        parent      = node->parent_;
        grandparent = parent->parent_;
        uncle_node  = IsLeftChild(parent) ? grandparent->right_child_ : grandparent->left_child_;

        if (!IsRed(uncle_node))
        {
            break;
        }

        // ����ڵ��Ǻ�ɫʱ
        parent->color_     = Color::COLOR_BLACK;
        uncle_node->color_ = Color::COLOR_BLACK;
        if (grandparent->parent_ == nullptr)
        {
            // �游�ڵ�Ϊ���ڵ�ʱ���ͱ�ʾ�Ѿ�������λ
            return;
        }
        grandparent->color_ = Color::COLOR_RED;
        node                = grandparent;
    }

    // ����ڵ��Ǻ�ɫʱ
    if (IsLeftChild(parent))
    {
        if (IsLeftChild(node))
        {
            RightRotate(grandparent);
            parent->color_      = Color::COLOR_BLACK;
            grandparent->color_ = Color::COLOR_RED;
        }
        else
        {
            LeftRotate(parent);
            RightRotate(grandparent);
            node->color_        = Color::COLOR_BLACK;
            grandparent->color_ = Color::COLOR_RED;
        }
    }
    else
    {
        if (IsLeftChild(node))
        {
            RightRotate(parent);
            LeftRotate(grandparent);
            node->color_        = Color::COLOR_BLACK;
            grandparent->color_ = Color::COLOR_RED;
        }
        else
        {
            LeftRotate(grandparent);
            parent->color_      = Color::COLOR_BLACK;
            grandparent->color_ = Color::COLOR_RED;
        }
    }
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::AdjusBeforeDelete(Node* dele_node)
{
    assert(dele_node != nullptr);
    if (dele_node->left_child_ != nullptr && dele_node->right_child_ != nullptr)
    {
        // ɾ���ڵ���2���ӽڵ�ʱ

        // ����ǰһ���ڵ���Ϊ����ڵ�
        Node* replace_node = dele_node->left_child_;
        if (replace_node->right_child_ == nullptr)
        {
            std::swap(dele_node->color_, replace_node->color_);

            if (dele_node->parent_ == nullptr)
            {
                root_ = replace_node;
            }
            else if (IsLeftChild(dele_node))
            {
                dele_node->parent_->left_child_ = replace_node;
            }
            else
            {
                dele_node->parent_->right_child_ = replace_node;
            }
            dele_node->right_child_->parent_ = replace_node;

            replace_node->parent_ = dele_node->parent_;
            dele_node->parent_    = replace_node;

            dele_node->left_child_    = replace_node->left_child_;
            replace_node->left_child_ = dele_node;

            replace_node->right_child_ = dele_node->right_child_;
            dele_node->right_child_    = nullptr;
        }
        else
        {
            while (replace_node->right_child_ != nullptr)
            {
                replace_node = replace_node->right_child_;
            }
            std::swap(dele_node->color_, replace_node->color_);

            if (dele_node->parent_ == nullptr)
            {
                root_ = replace_node;
            }
            else if (IsLeftChild(dele_node))
            {
                dele_node->parent_->left_child_ = replace_node;
            }
            else
            {
                dele_node->parent_->right_child_ = replace_node;
            }
            dele_node->right_child_->parent_ = replace_node;
            dele_node->left_child_->parent_  = replace_node;

            replace_node->parent_->right_child_ = dele_node;
            if (replace_node->left_child_ != nullptr)
            {
                replace_node->left_child_->parent_ = dele_node;
            }

            std::swap(replace_node->parent_, dele_node->parent_);
            std::swap(replace_node->left_child_, dele_node->left_child_);
            replace_node->right_child_ = dele_node->right_child_;
            dele_node->right_child_    = nullptr;
        }
    }

    // ��ʱ��Ҫɾ���Ľڵ����ֻ��һ���ӽڵ�
    assert(dele_node->left_child_ == nullptr || dele_node->right_child_ == nullptr);

    if (IsRed(dele_node))
    {
        // ɾ���ڵ�Ϊ��ɫʱ������ֱ��ɾ��
        assert(dele_node->left_child_ == nullptr);
        assert(dele_node->right_child_ == nullptr);
        if (IsLeftChild(dele_node))
        {
            dele_node->parent_->left_child_ = nullptr;
            if (dele_node == min_)
            {
                min_ = dele_node->parent_;
            }
        }
        else
        {
            dele_node->parent_->right_child_ = nullptr;
            if (dele_node == max_)
            {
                max_ = dele_node->parent_;
            }
        }
        return;
    }

    // ɾ���ڵ����һ���ӽڵ�����
    if (dele_node->left_child_ != nullptr)
    {
        // ɾ���ڵ�����ӽڵ㲻Ϊ��
        assert(dele_node->right_child_ == nullptr);

        Node* child = dele_node->left_child_;
        assert(IsRed(child));

        if (dele_node->parent_ != nullptr)
        {
            if (IsLeftChild(dele_node))
            {
                dele_node->parent_->left_child_ = child;
            }
            else
            {
                dele_node->parent_->right_child_ = child;
                if (dele_node == max_)
                {
                    max_ = child;
                }
            }
        }
        else
        {
            root_ = child;
            if (dele_node == max_)
            {
                max_ = child;
            }
        }
        child->parent_ = dele_node->parent_;
        child->color_  = Color::COLOR_BLACK;

        return;
    }
    else if (dele_node->right_child_ != nullptr)
    {
        // ɾ���ڵ�����ӽڵ㲻Ϊ��
        assert(dele_node->left_child_ == nullptr);

        Node* child = dele_node->right_child_;
        assert(IsRed(child));

        if (dele_node->parent_ != nullptr)
        {
            if (IsLeftChild(dele_node))
            {
                dele_node->parent_->left_child_ = child;
                if (dele_node == min_)
                {
                    min_ = child;
                }
            }
            else
            {
                dele_node->parent_->right_child_ = child;
            }
        }
        else
        {
            root_ = child;
            if (dele_node == min_)
            {
                min_ = child;
            }
        }
        child->parent_ = dele_node->parent_;
        child->color_  = Color::COLOR_BLACK;
        return;
    }

    // ��ʱ��ɾ���ڵ�Ϊ��ɫ�����ӽڵ�
    assert(dele_node->left_child_ == nullptr);
    assert(dele_node->right_child_ == nullptr);

    // ɾ���ڵ�Ϊ���ڵ�ʱ
    if (dele_node->parent_ == nullptr)
    {
        assert(root_ == dele_node);
        root_ = nullptr;
        min_  = nullptr;
        max_  = nullptr;
        return;
    }

    // ��ʱ��ɾ���ڵ�Ϊ��ɫ�����ӽڵ㣬��Ϊ���ڵ�

    Node* parent  = dele_node->parent_;
    Node* brother = GetBrother(dele_node);
    assert(brother != nullptr);

    // �ֵ��Ǻ�ɫʱ
    if (IsRed(brother))
    {
        parent->color_  = Color::COLOR_RED;
        brother->color_ = Color::COLOR_BLACK;

        if (IsLeftChild(dele_node))
        {
            LeftRotate(parent);
        }
        else
        {
            RightRotate(parent);
        }

        parent  = dele_node->parent_;
        brother = GetBrother(dele_node);
    }

    // ��ʱ��ɾ���ڵ�Ϊ��ɫ�����ӽڵ㣬��Ϊ���ڵ�,�ֵ��Ǻ�ɫ

    if (IsRed(brother->left_child_))
    {
        // �ֵ����ӽڵ��Ǻ�ɫ
        if (IsLeftChild(dele_node))
        {
            RightRotate(brother);
            LeftRotate(parent);
            parent->parent_->color_ = parent->color_;
            parent->color_          = Color::COLOR_BLACK;

            if (min_ == dele_node)
            {
                min_ = parent;
            }
            parent->left_child_ = nullptr;
            return;
        }
        else
        {
            RightRotate(parent);
            brother->color_              = parent->color_;
            parent->color_               = Color::COLOR_BLACK;
            brother->left_child_->color_ = Color::COLOR_BLACK;

            parent->right_child_ = nullptr;
            if (max_ == dele_node)
            {
                max_ = parent;
            }
            return;
        }
    }
    else if (IsRed(brother->right_child_))
    {
        // �ֵ����ӽڵ��Ǻ�ɫ
        if (IsLeftChild(dele_node))
        {
            LeftRotate(parent);
            brother->color_               = parent->color_;
            parent->color_                = Color::COLOR_BLACK;
            brother->right_child_->color_ = Color::COLOR_BLACK;

            parent->left_child_ = nullptr;
            if (min_ == dele_node)
            {
                min_ = parent;
            }
            return;
        }
        else
        {
            LeftRotate(brother);
            RightRotate(parent);
            parent->parent_->color_ = parent->color_;
            parent->color_          = Color::COLOR_BLACK;

            parent->right_child_ = nullptr;
            if (max_ == dele_node)
            {
                max_ = parent;
            }
            return;
        }
    }

    if (IsRed(parent))
    {
        parent->color_  = Color::COLOR_BLACK;
        brother->color_ = Color::COLOR_RED;

        if (IsLeftChild(dele_node))
        {
            parent->left_child_ = nullptr;
            if (min_ == dele_node)
            {
                min_ = parent;
            }
        }
        else
        {
            parent->right_child_ = nullptr;
            if (max_ == dele_node)
            {
                max_ = parent;
            }
        }
        return;
    }

    // ��ʱ��ɾ���ڵ㡢���ڵ㡢�ֵܽڵ㡢�ֵܵ������ӽڵ㶼�Ǻ�ɫ
    brother->color_ = Color::COLOR_RED;
    if (IsLeftChild(dele_node))
    {
        parent->left_child_ = nullptr;
        if (min_ == dele_node)
        {
            min_ = parent;
        }
    }
    else
    {
        parent->right_child_ = nullptr;
        if (max_ == dele_node)
        {
            max_ = parent;
        }
    }

    // ��ʱ�������֧��ÿ���ӷ�֧�ĺ�ɫ�ڵ��������������֧��1����Ҫ����
    Node* need_adjust_node = parent;
    while (true)
    {
        parent = need_adjust_node->parent_;
        if (parent == nullptr)
        {
            // ���ڵ�ʱ���Ѿ�������λ
            return;
        }

        brother = GetBrother(need_adjust_node);
        // �ֵ��Ǻ�ɫʱ
        if (IsRed(brother))
        {
            parent->color_  = Color::COLOR_RED;
            brother->color_ = Color::COLOR_BLACK;

            if (IsLeftChild(need_adjust_node))
            {
                LeftRotate(parent);
            }
            else
            {
                RightRotate(parent);
            }

            parent  = need_adjust_node->parent_;
            brother = GetBrother(need_adjust_node);
        }

        if (IsRed(brother->left_child_))
        {
            // �ֵ����ӽڵ��Ǻ�ɫ
            if (IsLeftChild(need_adjust_node))
            {
                RightRotate(brother);
                LeftRotate(parent);
                parent->parent_->color_ = parent->color_;
                parent->color_          = Color::COLOR_BLACK;

                return;
            }
            else
            {
                RightRotate(parent);
                brother->color_              = parent->color_;
                parent->color_               = Color::COLOR_BLACK;
                brother->left_child_->color_ = Color::COLOR_BLACK;

                return;
            }
        }
        else if (IsRed(brother->right_child_))
        {
            // �ֵ����ӽڵ��Ǻ�ɫ
            if (IsLeftChild(need_adjust_node))
            {
                LeftRotate(parent);
                brother->color_               = parent->color_;
                parent->color_                = Color::COLOR_BLACK;
                brother->right_child_->color_ = Color::COLOR_BLACK;

                return;
            }
            else
            {
                LeftRotate(brother);
                RightRotate(parent);
                parent->parent_->color_ = parent->color_;
                parent->color_          = Color::COLOR_BLACK;

                return;
            }
        }

        if (IsRed(need_adjust_node->parent_))
        {
            brother->color_ = Color::COLOR_RED;
            parent->color_  = Color::COLOR_BLACK;
            return;
        }

        brother->color_  = Color::COLOR_RED;
        need_adjust_node = need_adjust_node->parent_;
    }
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::LeftRotate(Node* node)
{
    Node* left_child  = node->left_child_;
    Node* right_child = node->right_child_;
    Node* parent      = node->parent_;
    assert(right_child != nullptr);

    if (parent == nullptr)
    {
        root_ = right_child;
    }
    else
    {
        if (IsLeftChild(node))
        {
            parent->left_child_ = right_child;
        }
        else
        {
            parent->right_child_ = right_child;
        }
    }

    node->parent_      = right_child;
    node->right_child_ = right_child->left_child_;
    if (right_child->left_child_ != nullptr)
    {
        node->right_child_->parent_ = node;
    }

    right_child->left_child_ = node;
    right_child->parent_     = parent;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::RightRotate(Node* node)
{
    Node* left_child  = node->left_child_;
    Node* right_child = node->right_child_;
    Node* parent      = node->parent_;
    assert(left_child != nullptr);

    if (parent == nullptr)
    {
        root_ = left_child;
    }
    else
    {
        if (IsLeftChild(node))
        {
            parent->left_child_ = left_child;
        }
        else
        {
            parent->right_child_ = left_child;
        }
    }

    node->parent_     = left_child;
    node->left_child_ = left_child->right_child_;
    if (left_child->right_child_ != nullptr)
    {
        node->left_child_->parent_ = node;
    }

    left_child->right_child_ = node;
    left_child->parent_      = parent;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::SeekFormer(Node* node) -> struct RedBlackTree<Key, Value>::Node*
{
    assert(node != nullptr);

    if (node->left_child_ == nullptr)
    {
        return nullptr;
    }

    Node* tmp = node->left_child_;
    while (tmp->right_child_ != nullptr)
    {
        tmp = tmp->right_child_;
    }

    return tmp;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::SeekLater(Node* node) -> struct RedBlackTree<Key, Value>::Node*
{
    assert(node != nullptr);

    if (node->right_child_ == nullptr)
    {
        return nullptr;
    }

    Node* tmp = node->right_child_;
    while (tmp->left_child_ != nullptr)
    {
        tmp = tmp->left_child_;
    }

    return tmp;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Min(Node* node) -> struct RedBlackTree<Key, Value>::Node*
{
    assert(node != nullptr);
    while (node->left_child_ != nullptr)
    {
        node = node->left_child_;
    }
    return node;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Max(Node* node) -> struct RedBlackTree<Key, Value>::Node*
{
    assert(node != nullptr);
    while (node->right_child_ != nullptr)
    {
        node = node->right_child_;
    }
    return node;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::GetBrother(Node* node) -> struct RedBlackTree<Key, Value>::Node*
{
    assert(node->parent_ != nullptr);
    return IsLeftChild(node) ? node->parent_->right_child_ : node->parent_->left_child_;
}

template <typename Key, typename Value>
RedBlackTree<Key, Value>::Iterator::Iterator(RedBlackTree* tree, Node* cur)
    : tree_(tree), cur_(cur)
{
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Iterator::operator++() -> class RedBlackTree<Key, Value>::Iterator&
{
    assert(cur_ != nullptr); // end������������������

    cur_ = tree_->NestNode(cur_);
    return *this;
}

template <typename Key, typename Value>
auto RedBlackTree<Key, Value>::Iterator::operator--() -> class RedBlackTree<Key, Value>::Iterator&
{
    cur_ = tree_->LastNode(cur_);
    return *this;
}


} // namespace jaf
