/*********************************************************************************************************************************************/
/*  DocWire SDK: Award-winning modern data processing in C++20. SourceForge Community Choice & Microsoft support. AI-driven processing.      */
/*  Supports nearly 100 data formats, including email boxes and OCR. Boost efficiency in text extraction, web data extraction, data mining,  */
/*  document analysis. Offline processing possible for security and confidentiality                                                          */
/*                                                                                                                                           */
/*  Copyright (c) SILVERCODERS Ltd, http://silvercoders.com                                                                                  */
/*  Project homepage: https://github.com/docwire/docwire                                                                                     */
/*                                                                                                                                           */
/*  SPDX-License-Identifier: AGPL-3.0-only OR LicenseRef-DocWire-Commercial                                                                  */
/*********************************************************************************************************************************************/

#ifndef DOCWIRE_OPENROUTER_TEXT_TO_SPEECH_H
#define DOCWIRE_OPENROUTER_TEXT_TO_SPEECH_H

#include "pipeline/element_base.hpp"
#include "pipeline/input_element.hpp"
#include "log_scope.h"
#include "make_error.h"
#include "openrouter_export.h"
#include "pipeline/output_element.hpp"
#include "post.h"
#include "serialization_message.h" // IWYU pragma: keep
#include <boost/json.hpp>
#include <sstream>
#include <string>

namespace docwire {
namespace openrouter {
class DOCWIRE_OPENROUTER_EXPORT text_to_speech : public pipeline::transformer_element<text_to_speech> {
public:
  explicit text_to_speech(const std::string &api_key,
                 const std::string &model = "openai/gpt-4o-mini-tts-2025-12-15",
                 const std::string &voice = "alloy",
                 const std::string &response_format = "mp3")
      : m_api_key(api_key), m_model(model), m_voice(voice),
        m_response_format(response_format) {
    log_scope(model, voice, response_format);
  }

  continuation operator()(message_ptr msg,
                          const message_callbacks &emit_message) {
    log_scope();
    if (!msg->is<data_source>())
      return emit_message(std::move(msg));
    const data_source &data = msg->get<data_source>();
    std::string data_str = data.string();
    std::string content = post_request(
        prepare_query(data_str, m_model, m_voice, m_response_format),
        m_api_key);
    return emit_message(data_source{content});
  }

private:
  std::string m_api_key;
  std::string m_model;
  std::string m_voice;
  std::string m_response_format;

  static std::string prepare_query(const std::string &input,
                                   const std::string &model,
                                   const std::string &voice,
                                   const std::string &response_format) {
    log_scope(input, model, voice, response_format);
    boost::json::object query{{"model", model},
                              {"input", input},
                              {"voice", voice},
                              {"response_format", response_format}};
    return boost::json::serialize(query);
  }

  static std::string post_request(const std::string &query,
                                  const std::string &api_key) {
    log_scope(query);
    std::ostringstream response_stream{};
    try {
      data_source{query, mime_type{"application/json"}, confidence::highest} |
          http::post("https://openrouter.ai/api/v1/audio/speech", api_key) |
          response_stream;
    } catch (const std::exception &e) {
      std::throw_with_nested(make_error(query));
    }
    return response_stream.str();
  }
};
} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_TEXT_TO_SPEECH_H
