/*********************************************************************************************************************************************/
/*  DocWire SDK: Award-winning modern data processing in C++20. SourceForge
 * Community Choice & Microsoft support. AI-driven processing.      */
/*  Supports nearly 100 data formats, including email boxes and OCR. Boost
 * efficiency in text extraction, web data extraction, data mining,  */
/*  document analysis. Offline processing possible for security and
 * confidentiality                                                          */
/*                                                                                                                                           */
/*  Copyright (c) SILVERCODERS Ltd, http://silvercoders.com */
/*  Project homepage: https://github.com/docwire/docwire */
/*                                                                                                                                           */
/*  SPDX-License-Identifier: AGPL-3.0-only OR LicenseRef-DocWire-Commercial */
/*********************************************************************************************************************************************/

#ifndef DOCWIRE_OPENROUTER_EMBED_H
#define DOCWIRE_OPENROUTER_EMBED_H

#include "ai_elements.h"
#include "pipeline/element_base.hpp"
#include "error_tags.h"
#include "pipeline/input_element.hpp"
#include "log_scope.h"
#include "make_error.h"
#include "openrouter_export.h"
#include "pipeline/output_element.hpp"
#include "post.h"
#include "serialization_message.h" // IWYU pragma: keep
#include "throw_if.h"
#include <boost/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace docwire {
namespace openrouter {
class DOCWIRE_OPENROUTER_EXPORT embed : public pipeline::transformer_element<embed> {
public:
	explicit embed(const std::string &api_key,
	        const std::string &model = "openai/text-embedding-3-small")
	      : m_api_key(api_key), m_model(model) {
	    log_scope(model);
	  }

  continuation operator()(message_ptr msg,
                          const message_callbacks &emit_message) {
    log_scope(msg);
    if (!msg->is<data_source>())
      return emit_message(std::move(msg));
    const data_source &data = msg->get<data_source>();
    throw_if(
        !data.has_highest_confidence_mime_type_in({mime_type{"text/plain"}}),
        errors::program_logic{});
    std::string data_str = data.string();
    std::vector<double> embedding_vector = parse_response(
        post_request(prepare_query(data_str, m_model), m_api_key));
    return emit_message(ai::embedding{std::move(embedding_vector)});
  }


private:
  std::string m_api_key;
  std::string m_model;

  static std::string prepare_query(const std::string &input,
                                   const std::string &model) {
    log_scope(input, model);
    boost::json::object query{{"model", model}, {"input", input}};
    return boost::json::serialize(query);
  }

  static std::string post_request(const std::string &query,
                                  const std::string &api_key) {
    log_scope(query);
    std::ostringstream response_stream{};
    try {
      data_source{query, mime_type{"application/json"}, confidence::highest} |
          http::post("https://openrouter.ai/api/v1/embedddings", api_key) |
          response_stream;
    } catch (const std::exception &e) {
      std::throw_with_nested(make_error(query));
    }
    return response_stream.str();
  }

  static std::vector<double> parse_response(const std::string &response) {
    log_scope(response);
    try {
      boost::json::value response_val = boost::json::parse(response);
      boost::json::object response_obj = response_val.as_object();
      if (response_obj.contains("error")) {
        const boost::json::object &err = response_obj["error"].as_object();
        std::string message = err.contains("message")
                                  ? err.at("message").as_string().c_str()
                                  : "unknown error";
        throw make_error("OpenRouter API returned an error", message);
      }
      const auto &embedding_array = response_val.as_object()
                                        .at("data")
                                        .as_array()
                                        .at(0)
                                        .as_object()
                                        .at("embedding")
                                        .as_array();

      std::vector<double> embedding;
      embedding.reserve(embedding_array.size());

      for (const auto &val : embedding_array) {
        embedding.push_back(val.as_double());
      }
      return embedding;
    } catch (const std::exception &e) {
      std::throw_with_nested(make_error(response));
    }
  }
};
} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_EMBED_H
