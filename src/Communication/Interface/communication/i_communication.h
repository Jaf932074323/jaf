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
// 2024-6-16 姜安富
#include "Interface/communication/i_serial_port.h"
#include "Interface/communication/i_tcp_client.h"
#include "Interface/communication/i_tcp_server.h"
#include "Interface/communication/i_udp.h"
#include "run_result.h"
#include "util/co_coroutine.h"
#include <memory>

namespace jaf
{
namespace comm
{

// 通讯接口
class ICommunication
{
public:
    ICommunication() {};
    virtual ~ICommunication() {};

public:
    virtual jaf::Coroutine<RunResult> Run() = 0;
    virtual void Stop()                     = 0;

public:
    virtual std::shared_ptr<ITcpServer> CreateTcpServer()   = 0;
    virtual std::shared_ptr<ITcpClient> CreateTcpClient()   = 0;
    virtual std::shared_ptr<IUdp> CreateUdp()               = 0;
    virtual std::shared_ptr<ISerialPort> CreateSerialPort() = 0;
};

} // namespace comm
} // namespace jaf