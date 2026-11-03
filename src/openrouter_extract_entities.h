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

#ifndef DOCWIRE_OPENROUTER_EXTRACT_ENTITIES_H
#define DOCWIRE_OPENROUTER_EXTRACT_ENTITIES_H

#include "audit/auditor_ref.hpp"
#include "audit/scope.hpp"
#include "named.h"
#include "openrouter_task.h"
#include "serialization_enum.h" // IWYU pragma: keep
#include <string>

namespace docwire {
namespace openrouter {

template <typename auditor_type = audit::null_auditor>
class extract_entities : public task<auditor_type> {
public:

  template <audit::stateless_auditor stateless_auditor_type = auditor_type>
  explicit extract_entities(const std::string &api_key,
              const std::string &model = "openrouter/auto", float temperature = 0,
              image_detail image_detail = image_detail::automatic)
      : extract_entities(api_key, model, temperature, image_detail,
           audit::auditor_ref<stateless_auditor_type>{}) {}

  explicit extract_entities(const std::string &api_key, const std::string &model,
              float temperature, image_detail image_detail,
              audit::auditor_ref<auditor_type> auditor)
      : task<auditor_type>("Your task is to find all dates, timespans, addresses, companies "
                           "and person names inside every message. Show it as formatted list.",
                           api_key, model, temperature, image_detail, auditor) {

    audit::scope audit_scope{this->auditor(), [&] {
      return "model"_v = model, "temperature"_v = temperature,
             "image_detail"_v = image_detail;
    }};
  }
};

} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_EXTRACT_ENTITIES_H
