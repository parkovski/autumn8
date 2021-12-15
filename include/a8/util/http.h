#ifndef A8_UTIL_HTTP_H_
#define A8_UTIL_HTTP_H_

#include <utility>
#include <string_view>

namespace a8::http {

// From https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
using http_status_t = std::pair<int, std::string_view>;

#define DEFINE_STATUS(Code, Reason) \
  static constexpr http_status_t HTTP_##Code##_##Reason{Code, #Reason}
#define DEFINE_STATUS2(Code, Reason, TextReason) \
  static constexpr http_status_t HTTP_##Code##_##Reason{Code, TextReason}
#define DEFINE_WEBDAV(Code, Reason, TextReason) \
  static constexpr http_status_t WebDav_##Code##_##Reason{Code, TextReason}

  DEFINE_STATUS (100, Continue);
  DEFINE_STATUS2(101, Switching_Protocols, "Switching Protocols");
  DEFINE_WEBDAV (102, Processing, "Processing");
  DEFINE_STATUS2(103, Early_Hints, "Early Hints");

  DEFINE_STATUS (200, OK);
  DEFINE_STATUS (201, Created);
  DEFINE_STATUS (202, Accepted);
  DEFINE_STATUS2(203, NonAuthoritative_Information, "Non-Authoritative Information");
  DEFINE_STATUS2(204, No_Content, "No Content");
  DEFINE_STATUS2(205, Reset_Content, "Reset Content");
  DEFINE_STATUS2(206, Partial_Content, "Partial Content");
  DEFINE_WEBDAV (207, MultiStatus, "Multi-Status");
  DEFINE_WEBDAV (208, Already_Reported, "Already Reported");
  DEFINE_STATUS2(226, IM_Used, "IM Used");

  DEFINE_STATUS2(300, Multiple_Choice, "Multiple Choice");
  DEFINE_STATUS2(301, Moved_Permanently, "Moved Permanently");
  DEFINE_STATUS (302, Found);
  DEFINE_STATUS2(303, See_Other, "See Other");
  DEFINE_STATUS2(304, Not_Modified, "Not Modified");
  DEFINE_STATUS2(305, Use_Proxy, "Use Proxy");
  DEFINE_STATUS2(307, Temporary_Redirect, "Temporary Redirect");
  DEFINE_STATUS2(308, Permanent_Redirect, "Permanent Redirect");

  DEFINE_STATUS2(400, Bad_Request, "Bad Request");
  DEFINE_STATUS (401, Unauthorized);
  DEFINE_STATUS2(402, Payment_Required, "Payment Required");
  DEFINE_STATUS (403, Forbidden);
  DEFINE_STATUS2(404, Not_Found, "Not Found");
  DEFINE_STATUS2(405, Method_Not_Allowed, "Method Not Allowed");
  DEFINE_STATUS2(406, Not_Acceptable, "Not Acceptable");
  DEFINE_STATUS2(407, Proxy_Authentication_Required, "Proxy Authentication Required");
  DEFINE_STATUS2(408, Request_Timeout, "Request Timeout");
  DEFINE_STATUS (409, Conflict);
  DEFINE_STATUS (410, Gone);
  DEFINE_STATUS2(411, Length_Required, "Length Required");
  DEFINE_STATUS2(412, Precondition_Failed, "Precondition Failed");
  DEFINE_STATUS2(413, Payload_Too_Large, "Payload Too Large");
  DEFINE_STATUS2(414, URI_Too_Long, "URI Too Long");
  DEFINE_STATUS2(415, Unsupported_Media_Type, "Unsupported Media Type");
  DEFINE_STATUS2(416, Range_Not_Satisfiable, "Range Not Satisfiable");
  DEFINE_STATUS2(417, Expectation_Failed, "Expectation Failed");
  DEFINE_STATUS2(418, Im_a_teapot, "I'm a teapot");
  DEFINE_STATUS2(421, Misdirected_Request, "Misdirected Request");
  DEFINE_WEBDAV (422, Unprocessable_Entity, "Unprocessable Entity");
  DEFINE_WEBDAV (423, Locked, "Locked");
  DEFINE_WEBDAV (424, Failed_Dependency, "Failed Dependency");
  DEFINE_STATUS2(425, Too_Early, "Too Early");
  DEFINE_STATUS2(426, Upgrade_Required, "Upgrade Required");
  DEFINE_STATUS2(428, Precondition_Required, "Precondition Required");
  DEFINE_STATUS2(429, Too_Many_Requests, "Too Many Requests");
  DEFINE_STATUS2(431, Request_Header_Fields_Too_Large, "Request Header Fields Too Large");
  DEFINE_STATUS2(451, Unavailable_For_Legal_Reasons, "Unavailable For Legal Reasons");

  DEFINE_STATUS2(500, Internal_Server_Error, "Internal Server Error");
  DEFINE_STATUS2(501, Not_Implemented, "Not Implemented");
  DEFINE_STATUS2(502, Bad_Gateway, "Bad Gateway");
  DEFINE_STATUS2(503, Service_Unavailable, "Service Unavailable");
  DEFINE_STATUS2(504, Gateway_Timeout, "Gateway Timeout");
  DEFINE_STATUS2(505, HTTP_Version_Not_Supported, "HTTP Version Not Supported");
  DEFINE_STATUS2(506, Variant_Also_Negotiates, "Variant Also Negotiates");
  DEFINE_WEBDAV (507, Insufficient_Storage, "Insufficient Storage");
  DEFINE_WEBDAV (508, Loop_Detected, "Loop Detected");
  DEFINE_STATUS2(510, Not_Extended, "Not Extended");
  DEFINE_STATUS2(511, Network_Authentication_Required, "Network Authentication Required");

#undef DEFINE_STATUS
#undef DEFINE_STATUS2

} // namespace a8::http

#endif // A8_UTIL_HTTP_H_
