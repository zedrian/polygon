#pragma once


#include "socket.h"


namespace polygon
{
    namespace network
    {
        class Socket::Implementation
        {
        public:
            Implementation();
            ~Implementation();

            void connect(string address,
                         unsigned short port);
            void close();

            size_t send(vector<unsigned char>& data);
            size_t receive(vector<unsigned char>& buffer);


        private:
            size_t _net_context;
            size_t _ssl_context;
        };
    }
}