//
//  main.cpp
//  BaseHttp
//
//  Created by 胡欣妍 on 2018/10/31.
//  Copyright © 2018 胡欣妍. All rights reserved.
//

#include <iostream>
#include <deque>
#include <sstream>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread.hpp>
#include "AsyncGetClient.hpp"
#include "SyncGetClient.hpp"

class getClient : public AsyncGetClient {
protected:
    std::deque<char> deque;
    std::stringstream ss;
    
    void handle_read_content(const boost::system::error_code& err) override {
        if (!err) {
            // Write all of the data that has been read so far.
            ss << &response_;
            // Continue reading remaining data until EOF.
            boost::asio::async_read(socket_, response_,
                                    boost::asio::transfer_at_least(1),
                                    boost::bind(&getClient::handle_read_content, this,
                                                boost::asio::placeholders::error));
        } else if (err != boost::asio::error::eof) {
            std::cerr << "handle_read_content Error: " << err.message() << std::endl;
        } else if (err == boost::asio::error::eof) {
            std::cout << "read end" << std::endl;
            boost::property_tree::ptree root;
            boost::property_tree::ptree data;
            boost::property_tree::read_json(ss, root);
            
            data = root.get_child("data");
            for (boost::property_tree::ptree::const_iterator it = data.begin(); it != data.end(); ++it) {
                //遍历读出数据
                std::string text = it->second.get<std::string>("text");
                std::string id = it->second.get<std::string>("id");
                std::string pid = it->second.get<std::string>("pid");
                std::cout << text << "\t" << id << "\t" << pid << "\t" << std::endl;
            }
        }
    }
public:
    getClient(boost::asio::io_context& io_context) : AsyncGetClient(io_context) {}
};

int main(int argc, const char * argv[]) {
    try {
        boost::asio::io_context io;
        AsyncGetClient client1(io);
        client1.get("iso.mirrors.ustc.edu.cn", "/qtproject/archive/qt/5.8/5.8.0/qt-opensource-mac-x64-clang-5.8.0.dmg");
        boost::thread t(boost::bind(&boost::asio::io_context::run, &io));
        io.run();
        t.join();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
