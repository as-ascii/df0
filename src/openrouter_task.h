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

#ifndef DOCWIRE_OPENROUTER_TASK_H
#define DOCWIRE_OPENROUTER_TASK_H

#include "audit/auditor_ref.hpp"
#include "audit/with_auditor.hpp"
#include "openrouter_export.h"
#include "pimpl.h"
#include "pipeline/element_base.hpp"
#include <concepts>
#include <string>

namespace docwire {
namespace openrouter {

enum class image_detail { low, high, automatic };

/**
 * @brief Chat chain element backed by OpenRouter (https://openrouter.ai), a
 * single API that routes requests to hundreds of models across many providers
 * (OpenAI, Anthropic, Google, Meta, Mistral, DeepSeek, and more).
 *
 * @tparam auditor_type Injected auditor type. Defaults to the zero-overhead
 * audit::null_auditor.
 */
template <typename auditor_type = audit::null_auditor>
class DOCWIRE_OPENROUTER_EXPORT task
    : public pipeline::transformer_element<task<auditor_type>>,
      public with_pimpl<task<auditor_type>>,
      private audit::with_auditor<auditor_type> {
public:
  /**
   * @brief Constructs the component with the default stateless auditing policy.
   *
   * Available only when the configured auditor is stateless (default-constructible).
   *
   * @param system_message System prompt sent with every request.
   * @param api_key OpenRouter API key (create one at
   * https://openrouter.ai/keys). Never written to audit output.
   * @param model OpenRouter model slug, e.g. "openai/gpt-5" or
   * "anthropic/claude-sonnet-5". Defaults to "openrouter/auto", which lets
   * OpenRouter choose the model.
   * @param temperature Sampling temperature.
   * @param image_detail Vision hint forwarded to models that support image
   * input.
   */
  template <audit::stateless_auditor stateless_auditor_type = auditor_type>
  task(const std::string &system_message, const std::string &api_key,
       const std::string &model = "openrouter/auto", float temperature = 0.5,
       image_detail image_detail = image_detail::automatic)
      : task(system_message, api_key, model, temperature, image_detail,
             audit::auditor_ref<stateless_auditor_type>{}) {}

  /// Constructs the component with an explicitly injected auditor.
  explicit task(const std::string &system_message, const std::string &api_key,
                const std::string &model, float temperature,
                image_detail image_detail,
                audit::auditor_ref<auditor_type> auditor);

  /**
   * @brief Executes transform operation for given node data.
   * @see docwire::message_ptr
   * @see docwire::message_callbacks
   * @param msg The input message to process.
   * @param emit_message Callback invoked to emit produced messages.
   */
  continuation operator()(message_ptr msg,
                          const message_callbacks &emit_message);

protected:
  using audit::with_auditor<auditor_type>::auditor;

private:
  using with_pimpl<task<auditor_type>>::impl;
};

// Deduction guide for CTAD
task(const std::string &, const std::string &, const std::string &, float,
     image_detail) -> task<audit::null_auditor>;

} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_TASK_H
