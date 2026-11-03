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


#ifndef DOCWIRE_OPENROUTER_TRANSCRIBE_H
#define DOCWIRE_OPENROUTER_TRANSCRIBE_H

#include "base64.h"
#include "data_source.h"
#include "document_elements.h"
#include "error_tags.h"
#include "log_entry.h"
#include "log_scope.h"
#include "make_error.h"
#include "pipeline/element_base.hpp"
#include "pipeline/input_element.hpp"
#include "pipeline/output_element.hpp"
#include "post.h"
#include <boost/json.hpp>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

namespace docwire {
namespace openrouter {

namespace transcribe_model {
inline constexpr std::string_view whisper_large_v3 = "openai/whisper-large-v3";
inline constexpr std::string_view whisper_1 = "openai/whisper-1";
inline constexpr std::string_view gpt_4o_transcribe =
    "openai/gpt-4o-transcribe";
inline constexpr std::string_view mai_transcribe_1_5 =
    "microsoft/mai-transcribe-1.5";
inline constexpr std::string_view grok_stt_1_0 = "x-ai/grok-stt-1.0";
} // namespace transcribe_model

class transcribe : public pipeline::transformer_element<transcribe> {
public:
  /**
   * @param api_key OpenRouter API key.
   * @param model. Defaults to model -> `whisper_large_v3
   * (openai/whisper-large-v3)`.
   * @param language Optional ISO-639-1 code (e.g. "en"); empty = auto-detect.
   */
  explicit transcribe(
      std::string api_key,
      std::string model = std::string{transcribe_model::whisper_large_v3},
      std::string language = "")
      : m_api_key(std::move(api_key)), m_model(std::move(model)),
        m_language(std::move(language)) {
    log_scope(m_model, m_language);
  }

  continuation operator()(message_ptr msg,
                          const message_callbacks &emit_message) {
    log_scope();
    if (!msg->is<data_source>())
      return emit_message(std::move(msg));
    log_entry();
    const data_source &data = msg->get<data_source>();

    std::string text = parse_response(post_request(
        prepare_query(audio_format(data), base64::encode(data.span()))));

    auto cont = emit_message(document::document{});
    if (cont == continuation::stop)
      return cont;
    cont = emit_message(document::text{text});
    if (cont == continuation::stop)
      return cont;
    return emit_message(document::close_document{});
  }

private:
  static std::string audio_format(const data_source &data) {
    static const std::map<std::string, std::string> by_extension{
        {".wav", "wav"}, {".mp3", "mp3"}, {".flac", "flac"}, {".m4a", "m4a"},
        {".ogg", "ogg"}, {".oga", "ogg"}, {".webm", "webm"}, {".aac", "aac"}};
    static const std::map<std::string, std::string> by_mime{
        {"audio/wav", "wav"},     {"audio/wave", "wav"},
        {"audio/x-wav", "wav"},   {"audio/mpeg", "mp3"},
        {"audio/mp3", "mp3"},     {"audio/flac", "flac"},
        {"audio/x-flac", "flac"}, {"audio/mp4", "m4a"},
        {"audio/x-m4a", "m4a"},   {"audio/ogg", "ogg"},
        {"audio/webm", "webm"},   {"audio/aac", "aac"}};
    if (auto ext = data.file_extension())
      if (auto it = by_extension.find(ext->string()); it != by_extension.end())
        return it->second;
    if (auto mt = data.highest_confidence_mime_type())
      if (auto it = by_mime.find(mt->v); it != by_mime.end())
        return it->second;
    throw make_error("Unable to determine audio format for OpenRouter "
                     "transcription (unknown file extension and MIME type)",
                     errors::uninterpretable_data{});
  }

  std::string prepare_query(const std::string &format,
                            const std::string &audio_base64) const {
    boost::json::object query{
        {"model", m_model},
        {"input_audio",
         boost::json::object{{"data", audio_base64}, {"format", format}}}};
    if (!m_language.empty())
      query["language"] = m_language;
    return boost::json::serialize(query);
  }

  std::string post_request(const std::string &query) const {
    std::ostringstream response_stream{};
    try {
      data_source{query, mime_type{"application/json"}, confidence::highest} |
          http::post("https://openrouter.ai/api/v1/audio/transcriptions",
                     m_api_key) |
          response_stream;
    } catch (const std::exception &) {
      // the query is deliberately not attached: it holds the whole base64 audio
      std::throw_with_nested(make_error("Error during transcription"));
    }
    return response_stream.str();
  }

  static std::string parse_response(const std::string &response) {
    log_scope();
    try {
      boost::json::object obj = boost::json::parse(response).as_object();
      if (obj.contains("error")) {
        const boost::json::value &err = obj.at("error");
        std::string message =
            err.is_object() && err.as_object().contains("message")
                ? std::string{err.as_object().at("message").as_string().c_str()}
                : "unknown error";
        throw make_error("OpenRouter API returned an error", message);
      }
      return obj.at("text").as_string().c_str();
    } catch (const std::exception &) {
      std::throw_with_nested(make_error(response));
    }
  }

  std::string m_api_key;
  std::string m_model;
  std::string m_language;
};

} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_TRANSCRIBE_H
