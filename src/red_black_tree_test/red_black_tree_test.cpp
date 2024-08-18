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
// 2024-8-16 姜安富
#include "util/red_black_tree.h"
#include <Windows.h>
#include <format>
#include <gtest/gtest.h>
#include <time.h>

namespace jaf
{

class TestRedBlackTree
{
public:
    using Tree = jaf::RedBlackTree<int, int>;

    void ShowNode(Tree::Node* node, uint32_t depth)
    {
        if (node == nullptr)
        {
            return;
        }
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
        SetConsoleTextAttribute(hConsole, Tree::IsRed(node) ? BACKGROUND_RED : csbi.wAttributes);
        std::cout << node->key_ << "----\t";
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);

        ShowNode(node->left_child_, depth + 1);
        std::cout << "\n";
        for (uint32_t i = 0; i < depth; ++i)
        {
            std::cout << "\t";
        }
        std::cout << "  └---\t";
        ShowNode(node->right_child_, depth + 1);
    }
    void CheckNodeRelation(Tree::Node* node, size_t total_count)
    {
        if (node == nullptr)
        {
            return;
        }
        if (node->parent_ != nullptr)
        {
            EXPECT_TRUE(node->parent_->left_child_ == node || node->parent_->right_child_ == node)
                << std::format("key = {}, total_count = {}", node->key_, total_count);
        }
        if (node->left_child_ != nullptr)
        {
            EXPECT_TRUE(node->left_child_->parent_ == node)
                << std::format("key = {}, total_count = {}", node->key_, total_count);
        }
        if (node->right_child_ != nullptr)
        {
            EXPECT_TRUE(node->right_child_->parent_ == node)
                << std::format("key = {}, total_count = {}", node->key_, total_count);
        }
        CheckNodeRelation(node->left_child_, total_count);
        CheckNodeRelation(node->right_child_, total_count);
    }
    size_t GetCount(Tree::Node* node)
    {
        if (node == nullptr)
        {
            return 0;
        }
        size_t count = 1;
        count += GetCount(node->left_child_);
        count += GetCount(node->right_child_);
        return count;
    }
    void CheckBranchBlackAmount(Tree::Node* curr_node, uint32_t branch_black_amount, uint32_t branch_black_count)
    {
        if (curr_node == nullptr)
        {
            EXPECT_EQ(branch_black_count, branch_black_amount);
            return;
        }

        if (Tree::IsBlack(curr_node))
        {
            ++branch_black_count;
        }
        CheckBranchBlackAmount(curr_node->left_child_, branch_black_amount, branch_black_count);
        CheckBranchBlackAmount(curr_node->right_child_, branch_black_amount, branch_black_count);
    }
    void CheckContinuousRedNode(Tree::Node* node)
    {
        if (node == nullptr)
        {
            return;
        }
        if (Tree::IsRed(node))
        {
            EXPECT_TRUE(Tree::IsBlack(node->left_child_));
            EXPECT_TRUE(Tree::IsBlack(node->right_child_));
        }
        CheckContinuousRedNode(node->left_child_);
        CheckContinuousRedNode(node->right_child_);
    }
    void CheckValuesSizeRelation(Tree::Node* node)
    {
        if (node == nullptr)
        {
            return;
        }
        if (node->left_child_ != nullptr)
        {
            EXPECT_TRUE(node->left_child_->key_ <= node->key_);
            CheckValuesSizeRelation(node->left_child_);
        }
        if (node->right_child_ != nullptr)
        {
            EXPECT_TRUE(node->key_ <= node->right_child_->key_);
            CheckValuesSizeRelation(node->right_child_);
        }
    }
    void CheckIterator(Tree& tree)
    {
        size_t count = 0;
        auto it      = tree.begin();
        int last     = *it;
        for (; it != tree.end(); ++it)
        {
            EXPECT_TRUE(last <= *it);
            last = *it;
            ++count;
            if (count > tree.Size())
            {
                break;
            }
        }
        EXPECT_EQ(count, tree.Size());
        EXPECT_EQ(it, tree.end());
    }

    void CheckMinMax(Tree& tree)
    {
        if (tree.Empty())
        {
            return;
        }
        EXPECT_EQ(tree.Min(tree.GetRoot())->key_, tree.begin().operator*());
        EXPECT_EQ(tree.Max(tree.GetRoot())->key_, (--tree.end()).operator*());
    }

    // 获取一条支路的黑色节点数量
    uint32_t GetBranchBlackAmount(Tree::Node* root)
    {
        uint32_t branch_black_amount = 0;
        for (Tree::Node* node = root; node != nullptr; node = node->left_child_)
        {
            if (Tree::IsBlack(node))
            {
                ++branch_black_amount;
            }
        }
        return branch_black_amount;
    }

    void Check(jaf::RedBlackTree<int, int>& rb_tree)
    {
        Tree::Node* root = rb_tree.GetRoot();
        if (root == nullptr)
        {
            return;
        }
        EXPECT_EQ(root->color_, Tree::Color::COLOR_BLACK);

        uint32_t branch_black_amount = GetBranchBlackAmount(root);
        CheckBranchBlackAmount(root, branch_black_amount, 0);
        CheckContinuousRedNode(root);
        CheckValuesSizeRelation(root);
        CheckIterator(rb_tree);
        CheckMinMax(rb_tree);
    }

    void TestTree(int* arr, size_t len)
    {
        jaf::RedBlackTree<int, int> rb_tree;
        for (size_t i = 0; i < len; ++i)
        {
            rb_tree.Insert(arr[i], arr[i]);
            size_t total_count = GetCount(rb_tree.GetRoot());
            EXPECT_EQ(total_count, i + 1);
            EXPECT_EQ(total_count, rb_tree.Size());

            CheckNodeRelation(rb_tree.GetRoot(), total_count);
            Check(rb_tree);
        }

        //Tree::Node* node_1 = rb_tree.GetRoot();
        //Tree::Node* node_2 = node_1->left_child_;
        //rb_tree.SwitchNode(node_1, node_2);
        //std::cout << std::endl << "--------------------" << std::endl;
        //ShowNode(rb_tree.GetRoot(), 0);
        //std::cout << std::endl << "--------------------" << std::endl;

        for (size_t i = 0; i < len; ++i)
        {
            //std::cout << "delete " << arr[i] << std::endl;
            rb_tree.Erase(arr[i]);

            //std::cout << std::endl << "--------------------" << std::endl;
            //ShowNode(rb_tree.GetRoot(), 0);
            //std::cout << std::endl << "--------------------" << std::endl;

            size_t total_count = GetCount(rb_tree.GetRoot());
            EXPECT_EQ(total_count, rb_tree.Size());

            CheckNodeRelation(rb_tree.GetRoot(), total_count);
            Check(rb_tree);
        }
    }
};

}

TEST(red_black_tree, normal)
{
    jaf::TestRedBlackTree test_red_black_tree;

    int arr1[] = {33, 13, 95, 1, 55, 47, 19, 23, 11, 18, 20, 123};
    int arr2[] = {16, 3, 7, 11, 9, 26, 18, 14, 15};
    int arr3[] = {24, 25, 13, 35, 23, 26, 67, 47, 38, 98, 20, 19, 17, 49, 12, 21, 9, 18, 14, 15};
    test_red_black_tree.TestTree(arr1, sizeof(arr1) / sizeof(int));
    test_red_black_tree.TestTree(arr2, sizeof(arr2) / sizeof(int));
    test_red_black_tree.TestTree(arr3, sizeof(arr3) / sizeof(int));

    //int arr[] =
    //{
    //    9254,9254
    //};
    //test_red_black_tree.TestTree(arr, sizeof(arr) / sizeof(int));
}

void Show(int* arr, size_t len)
{
    std::cout << "int arr[] = {";

    std::cout << arr[0];
    size_t index = 1;
    while (index < len)
    {
        for (size_t i = 1; i < 10 && index < len; ++i, ++index)
        {
            std::cout << "," << arr[index];
        }
        std::cout << "\r\n";
    }
    std::cout << "};" << std::endl;
}


TEST(red_black_tree, random)
{
    jaf::TestRedBlackTree test_red_black_tree;
    size_t amount = 100;
    srand(time(0));
    std::vector<int> arr;
    arr.resize(amount);
    for (size_t i = 0; i < amount; ++i)
    {
        arr[i] = rand();
    }
    test_red_black_tree.TestTree(arr.data(), amount);

    // 获取当前测试实例
    const ::testing::UnitTest& unit_test = *::testing::UnitTest::GetInstance();
    // 检查是否有失败的测试
    if (unit_test.failed_test_count() > 0)
    {
        Show(arr.data(), amount);
    }
}