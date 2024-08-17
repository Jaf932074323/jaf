#pragma once
#include <assert.h>
#include <stddef.h>
#include <utility>

// 红黑树
// 键可以重复
template <typename Key, typename Value>
class RedBlackTree
{
public:
    enum Color
    {
        COLOR_BLACK,
        COLOR_RED,
    };

    struct Node
    {
        Key key_;
        Value value_;
        Color color_;
        Node* parent_{nullptr};
        Node* left_child_{nullptr};
        Node* right_child_{nullptr};
    };

public:
    RedBlackTree()                               = default;
    ~RedBlackTree()                              = default;
    RedBlackTree(const RedBlackTree&)            = delete; // 拷贝较为复杂，目前先禁止
    RedBlackTree& operator=(const RedBlackTree&) = delete; // 拷贝较为复杂，目前先禁止
public:
    void Insert(const Key& key, const Value& value);
    void Erase(const Key& key);
    inline size_t Size()
    {
        return size_;
    }
    inline bool Empty()
    {
        return size_ == 0;
    }

public:
    Node* GetRoot()
    {
        return root_;
    }

public:
    void AdjustAfterInsert(Node* node);      // 插入元素后调整
    void AdjusBeforeDelete(Node* dele_node); // 删除元素之前调整
    inline void LeftRotate(Node* node);      // 左旋
    inline void RightRotate(Node* node);     // 右旋
    inline Node* SeekFormer(Node* node);     // 搜索紧邻的前一个元素
    inline Node* SeekLater(Node* node);      // 搜索紧邻的后一个元素
    inline static Node* Min(Node* node);     // 搜索node下的最小节点
    inline static Node* Max(Node* node);     // 搜索node下的最最大节点

    // 是否是左孩子
    inline static bool IsLeftChild(Node* node)
    {
        assert(node->parent_ != nullptr);
        return node->parent_->left_child_ == node;
    }
    // 是否是右孩子
    inline static bool IsRightChild(Node* node)
    {
        assert(node->parent_ != nullptr);
        return node->parent_->right_child_ == node;
    }
    // 是否是红色节点
    inline static bool IsRed(Node* node)
    {
        return node != nullptr && node->color_ == Color::COLOR_RED;
    }
    // 是否是黑色节点
    inline static bool IsBlack(Node* node)
    {
        return node == nullptr || node->color_ == Color::COLOR_BLACK;
    }

public:
    inline void SwitchNode(Node* node_1, Node* node_2);
    inline static Node* GetBrother(Node* node);

public:
    class Iterator
    {
    public:
        Iterator(RedBlackTree* tree, Node* cur);
        inline Iterator& operator++();
        inline Iterator& operator--();
        //重载==运算符
        inline bool operator==(const Iterator& other) const
        {
            return other.cur_ == cur_;
        }
        //重载!=运算符
        inline bool operator!=(const Iterator& that) const
        {
            return !(*this == that);
        }

        inline Key& operator*() const noexcept
        {
            assert(cur_ != nullptr);
            return cur_->key_;
        }
        Node* GetNode()
        {
            return cur_;
        }

    private:
        Node* cur_; // 当前节点
        RedBlackTree* tree_;
    };

public:
    inline Iterator begin()
    {
        return Iterator(this, min_);
    }
    inline Iterator end()
    {
        return Iterator(this, nullptr);
    }

private:
    Node* root_  = nullptr; // 根节点
    Node* min_   = nullptr; // 最小节点
    Node* max_   = nullptr; // 最大节点
    size_t size_ = 0;
};

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::Insert(const Key& key, const Value& value)
{
    if (root_ == nullptr)
    {
        root_ = new Node{key, value, Color::COLOR_BLACK};
        ++size_;
        min_ = root_;
        max_ = root_;
        return;
    }

    Node* new_node = nullptr;

    // 先插入节点,插入节点只会是叶子节点
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

    // 再调整
    AdjustAfterInsert(new_node);
    ++size_;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::Erase(const Key& key)
{
    // TODO: 先考虑键不重复的情况

    if (root_ == nullptr)
    {
        return;
    }

    // 查找要删除的节点
    Node* del_node = root_;
    while (true)
    {
        if (del_node->key_ == key)
        {
            break;
        }

        if (key < del_node->key_)
        {
            del_node = del_node->left_child_;
        }
        else
        {
            del_node = del_node->right_child_;
        }

        // 不存在对应键的节点
        if (del_node == nullptr)
        {
            return;
        }
    }

    AdjusBeforeDelete(del_node);
    --size_;
    return;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::AdjustAfterInsert(Node* node)
{
    Node* parent      = nullptr; // 父节点
    Node* grandparent = nullptr; // 祖父节点
    Node* uncle_node  = nullptr; // 叔叔节点

    while (true)
    {
        assert(node->parent_ != nullptr); // node不可能是根节点
        if (node->parent_->color_ == Color::COLOR_BLACK)
        {
            return;
        }
        assert(node->parent_->parent_ != nullptr); // 当父节点是红色时，父节点也不可能是根节点

        parent      = node->parent_;
        grandparent = parent->parent_;
        uncle_node  = IsLeftChild(parent) ? grandparent->right_child_ : grandparent->left_child_;

        if (!IsRed(uncle_node))
        {
            break;
        }

        // 叔叔节点是红色时
        parent->color_     = Color::COLOR_BLACK;
        uncle_node->color_ = Color::COLOR_BLACK;
        if (grandparent->parent_ == nullptr)
        {
            // 祖父节点为根节点时，就表示已经调整到位
            return;
        }
        grandparent->color_ = Color::COLOR_RED;
        node                = grandparent;
    }

    // 叔叔节点是黑色时
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
        // 删除节点有2个子节点时

        // 查找前一个节点作为替代节点
        Node* replace_node = SeekFormer(dele_node);
        assert(replace_node != nullptr);
        assert(replace_node->left_child_ == nullptr || replace_node->right_child_ == nullptr);

        // 交换位置
        // 因为node必然会被删除，因此交换位置后，只会临时的导致节点大小顺序不一致，等node删除后恢复正常
        SwitchNode(replace_node, dele_node);
    }

    // 这时候要删除的节点最多只有一个子节点
    assert(dele_node->left_child_ == nullptr || dele_node->right_child_ == nullptr);

    if (IsRed(dele_node))
    {
        // 删除节点为红色时，可以直接删除
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
        delete dele_node;
        return;
    }

    // 删除节点存在一个子节点的情况
    if (dele_node->left_child_ != nullptr)
    {
        // 删除节点的左子节点不为空
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

        delete dele_node;
        return;
    }
    else if (dele_node->right_child_ != nullptr)
    {
        // 删除节点的右子节点不为空
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
        delete dele_node;
        return;
    }

    // 这时，删除节点为黑色，无子节点
    assert(dele_node->left_child_ == nullptr);
    assert(dele_node->right_child_ == nullptr);

    // 删除节点为根节点时
    if (dele_node->parent_ == nullptr)
    {
        assert(root_ == dele_node);
        root_ = nullptr;
        min_  = nullptr;
        max_  = nullptr;
        delete dele_node;
        return;
    }

    // 这时，删除节点为黑色，无子节点，不为根节点

    Node* parent  = dele_node->parent_;
    Node* brother = GetBrother(dele_node);
    assert(brother != nullptr);

    // 兄弟是红色时
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

    // 这时，删除节点为黑色，无子节点，不为根节点,兄弟是黑色

    if (IsRed(brother->left_child_))
    {
        // 兄弟左子节点是红色
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
            delete dele_node;
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
            delete dele_node;
            return;
        }
    }
    else if (IsRed(brother->right_child_))
    {
        // 兄弟右子节点是红色
        if (IsLeftChild(dele_node))
        {
            LeftRotate(parent);
            brother->color_               = parent->color_;
            parent->color_                = Color::COLOR_BLACK;
            brother->right_child_->color_ = Color::COLOR_BLACK;

            parent->left_child_ = nullptr;
            delete dele_node;
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
            delete dele_node;
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
        delete dele_node;
        return;
    }

    // 这时，删除节点、父节点、兄弟节点、兄弟的左右子节点都是黑色
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
    delete dele_node;

    // 这时，这个分支的每条子分支的黑色节点的数量比其它分支少1，需要调整
    Node* need_adjust_node = parent;
    while (true)
    {
        parent = need_adjust_node->parent_;
        if (parent == nullptr)
        {
            // 根节点时，已经调整到位
            return;
        }

        brother = GetBrother(need_adjust_node);
        // 兄弟是红色时
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
            // 兄弟左子节点是红色
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
            // 兄弟右子节点是红色
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
struct RedBlackTree<Key, Value>::Node* RedBlackTree<Key, Value>::SeekFormer(Node* node)
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
struct RedBlackTree<Key, Value>::Node* RedBlackTree<Key, Value>::SeekLater(Node* node)
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
struct RedBlackTree<Key, Value>::Node* RedBlackTree<Key, Value>::Min(Node* node)
{
    assert(node != nullptr);
    while (node->left_child_ != nullptr)
    {
        node = node->left_child_;
    }
    return node;
}

template <typename Key, typename Value>
struct RedBlackTree<Key, Value>::Node* RedBlackTree<Key, Value>::Max(Node* node)
{
    assert(node != nullptr);
    while (node->right_child_ != nullptr)
    {
        node = node->right_child_;
    }
    return node;
}

template <typename Key, typename Value>
void RedBlackTree<Key, Value>::SwitchNode(Node* node_1, Node* node_2)
{
    bool is_left_1 = node_1->parent_ != nullptr && IsLeftChild(node_1);
    bool is_left_2 = node_2->parent_ != nullptr && IsLeftChild(node_2);

    std::swap(node_1->parent_, node_2->parent_);
    std::swap(node_1->left_child_, node_2->left_child_);
    std::swap(node_1->right_child_, node_2->right_child_);
    std::swap(node_1->color_, node_2->color_);

    if (node_1->left_child_ == node_1)
    {
        node_1->left_child_ = node_2;
    }
    else if (node_1->right_child_ == node_1)
    {
        node_1->right_child_ = node_2;
    }
    else if (node_2->left_child_ == node_2)
    {
        node_2->left_child_ = node_1;
    }
    else if (node_2->right_child_ == node_2)
    {
        node_2->right_child_ = node_1;
    }

    if (node_1->parent_ == nullptr)
    {
        root_ = node_1;
    }
    else if (node_1->parent_ == node_1)
    {
        node_1->parent_ = node_2;
    }
    else if (is_left_2)
    {
        node_1->parent_->left_child_ = node_1;
    }
    else
    {
        node_1->parent_->right_child_ = node_1;
    }
    if (node_1->left_child_ != nullptr)
    {
        node_1->left_child_->parent_ = node_1;
    }
    if (node_1->right_child_ != nullptr)
    {
        node_1->right_child_->parent_ = node_1;
    }

    if (node_2->parent_ == nullptr)
    {
        root_ = node_2;
    }
    else if (node_2->parent_ == node_2)
    {
        node_2->parent_ = node_1;
    }
    else if (is_left_1)
    {
        node_2->parent_->left_child_ = node_2;
    }
    else
    {
        node_2->parent_->right_child_ = node_2;
    }
    if (node_2->left_child_ != nullptr)
    {
        node_2->left_child_->parent_ = node_2;
    }
    if (node_2->right_child_ != nullptr)
    {
        node_2->right_child_->parent_ = node_2;
    }
}

template <typename Key, typename Value>
struct RedBlackTree<Key, Value>::Node* RedBlackTree<Key, Value>::GetBrother(Node* node)
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
class RedBlackTree<Key, Value>::Iterator& RedBlackTree<Key, Value>::Iterator::operator++()
{
    assert(cur_ != nullptr); // end迭代器不可以在增加

    if (cur_->right_child_ != nullptr)
    {
        cur_ = RedBlackTree::Min(cur_->right_child_);
        return *this;
    }

    Node* parent_node = cur_->parent_;
    while (parent_node != nullptr && RedBlackTree::IsRightChild(cur_))
    {
        cur_        = parent_node;
        parent_node = cur_->parent_;
    }
    cur_ = parent_node;
    return *this;
}

template <typename Key, typename Value>
class RedBlackTree<Key, Value>::Iterator& RedBlackTree<Key, Value>::Iterator::operator--()
{
    if (cur_ == nullptr)
    {
        cur_ = tree_->max_;
        return *this;
    }

    if (cur_->left_child_ != nullptr)
    {
        cur_ = RedBlackTree::Max(cur_->right_child_);
        return *this;
    }

    Node* parent_node = cur_->parent_;
    while (parent_node != nullptr && RedBlackTree::IsLeftChild(cur_))
    {
        cur_        = parent_node;
        parent_node = cur_->parent_;
    }
    cur_ = parent_node;
    return *this;
}
