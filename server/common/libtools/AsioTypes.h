#ifndef ASIO_TYPES_H
#define ASIO_TYPES_H

#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <memory>
#include <functional>


#define PLACEHOLDER_ERROR boost::asio::placeholders::error
namespace PLACEHOLDER = std::placeholders;
using TCP_SPACE = boost::asio::ip::tcp ;
using SYSTEM_CODE = boost::system::error_code ;
using IO_CONTEXT = boost::asio::io_context;
using IO_CONTEXT_PTR = std::shared_ptr<boost::asio::io_context>;
#endif