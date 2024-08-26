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
// 2024-6-16 姜安富
#include "log_head.h"
#include "util/red_black_tree.h"
#include "util/stopwatch.h"
#include <map>
#include <iostream>
#include <format>

void shuffle(std::vector<int> &nums)
{
    srand((unsigned)time(NULL)); // 需要重设随机数种子
    int n = nums.size();
    for (int i = n - 1; i > 0; i--)
    {
        // 最后一个元素不需要交换了
        int randIdx = rand() % i;
        // cout << "randIdx" << randIdx << endl;
        std::swap(nums[i], nums[randIdx]);
    }
}


int main()
{
    std::shared_ptr<jaf::log::ConsoleAppender> appender = std::make_shared<jaf::log::ConsoleAppender>();
    std::shared_ptr<jaf::log::ILogger> logger           = std::make_shared<jaf::log::Logger>(appender);
    jaf::log::CommonLogger::SetDefaultLogger(logger);

    size_t amount = 100000000;
    srand(time(0));
    std::vector<int> arr;
    arr.resize(amount);
    for (size_t i = 0; i < amount; ++i)
    {
        arr[i] = rand();
        //arr[i] = i;
    }
    shuffle(arr);
    const int* ptr_arr = arr.data();

    using Tree = jaf::RedBlackTree<int, int>;
    Tree my_tree;
    std::multimap<int, int> std_map;
    jaf::Stopwatch stopwatch;

    stopwatch.Reset();
    for (size_t i = 0; i < amount; ++i)
    {
        my_tree.Insert(ptr_arr[i], ptr_arr[i]);
    }
    std::chrono::nanoseconds my_tree_insert = stopwatch.Time();
    std::cout << std::format("my_tree_insert = {}", std::chrono::duration_cast<std::chrono::milliseconds>(my_tree_insert).count()) << std::endl;

    stopwatch.Reset();
    for (size_t i = 0; i < amount; ++i)
    {
        std_map.insert(std::make_pair(ptr_arr[i], ptr_arr[i]));
    }
    std::chrono::nanoseconds std_map_insert = stopwatch.Time();

    std::cout << std::format("std_map_insert = {}", std::chrono::duration_cast<std::chrono::milliseconds>(std_map_insert).count()) << std::endl;

    stopwatch.Reset();
    for (size_t i = 0; i < amount; ++i)
    {
        my_tree.Erase(ptr_arr[i]);
    }
    std::chrono::nanoseconds my_tree_erase = stopwatch.Time();
    std::cout << std::format("my_tree_erase = {}", std::chrono::duration_cast<std::chrono::milliseconds>(my_tree_erase).count()) << std::endl;

    stopwatch.Reset();
    for (size_t i = 0; i < amount; ++i)
    {
        std_map.erase(ptr_arr[i]);
    }
    std::chrono::nanoseconds std_map_erase = stopwatch.Time();

    std::cout << std::format("std_map_erase = {}", std::chrono::duration_cast<std::chrono::milliseconds>(std_map_erase).count()) << std::endl;

    return 0;
}