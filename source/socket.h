#pragma once


#include <string>
#include <vector>
#include <memory>


using std::string;
using std::vector;
using std::size_t;
using std::shared_ptr;


namespace polygon
{
    namespace network
    {
        class Socket
        {
        public:
            Socket();
            ~Socket();

            void connect(string address,
                         unsigned short port);
            void close();

            size_t send(vector<unsigned char>& data);
            size_t receive(vector<unsigned char>& buffer);


        private:
            class Implementation;
            shared_ptr<Implementation> _pimpl;
        };
    }
}