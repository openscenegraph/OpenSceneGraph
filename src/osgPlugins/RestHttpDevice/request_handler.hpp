//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>

#include <osg/observer_ptr>

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
public:
    class Callback : public osg::Referenced {
        public:
            Callback() : osg::Referenced() {}
            
            virtual bool operator()(const std::string& request_path, reply& rep) = 0;
            virtual std::string applyTemplateVars(const std::string& txt) = 0;
    };
    
  /// Construct with a directory containing files to be served.
  explicit request_handler(const std::string& doc_root);

  /// Handle a request and produce a reply.
  void handle_request(const request& req, reply& rep);
  
  void setCallback(Callback* cb) { _cb = cb; }

private:
  /// The directory containing the files to be served.
  std::string doc_root_;

  /// Perform URL-decoding on a string. Returns false if the encoding was
  /// invalid.
  static bool url_decode(const std::string& in, std::string& out);
  osg::observer_ptr<Callback> _cb;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_REQUEST_HANDLER_HPP
