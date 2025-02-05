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
// 2025-2-4 姜安富
#include "define_communication_export.h"
#include "define_constant.h"
#include "interface/i_communication.h"
#include "pack.h"
#include "recv_buffer.h"
#include "time/time_head.h"
#include "util/i_thread_pool.h"
#include <memory>
#include "interface/buffer/i_pack.h"
#include "interface/buffer/i_reader.h"
#include "interface/buffer/i_recv_buffer.h"
#include "interface/buffer/i_unpack.h"
#include "interface/buffer/i_writer.h"
#include "interface/buffer/comm_struct.h"

namespace jaf
{
namespace comm
{

std::shared_ptr<ICommunication> API_COMMUNICATION_EXPORT CreateCommunication(std::shared_ptr<IThreadPool> thread_pool = nullptr, std::shared_ptr<jaf::time::ITimer> timer = nullptr);

} // namespace comm
} // namespace jaf