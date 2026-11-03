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

#ifndef DOCWIRE_OPENROUTER_CLASSIFY_H
#define DOCWIRE_OPENROUTER_CLASSIFY_H

#include "audit/auditor_ref.hpp"
#include "audit/scope.hpp"
#include "boost/algorithm/string/join.hpp"
#include "named.h"
#include "openrouter_task.h"
#include "serialization_enum.h" // IWYU pragma: keep
#include <set>
#include <string>

namespace docwire {
namespace openrouter {

template <typename auditor_type = audit::null_auditor>
class classify : public task<auditor_type> {
public:
  template <audit::stateless_auditor stateless_auditor_type = auditor_type>
  explicit classify(const std::set<std::string> &categories,
                    const std::string &api_key,
                    const std::string &model = "openrouter/auto",
                    float temperature = 0,
                    image_detail image_detail = image_detail::automatic)
      : classify(categories, api_key, model, temperature, image_detail,
                 audit::auditor_ref<stateless_auditor_type>{}) {}

  explicit classify(const std::set<std::string> &categories,
                    const std::string &api_key, const std::string &model,
                    float temperature, image_detail image_detail,
                    audit::auditor_ref<auditor_type> auditor)
      : task<auditor_type>(
            "Your task is to classify every message to one of the following "
            "categories: " +
                boost::algorithm::join(categories, ", ") +
                ". Answer with the category name only, exactly as it was "
                "written.",
            api_key, model, temperature, image_detail, auditor) {
    audit::scope audit_scope{this->auditor(), [&] {
      return "categories"_v = categories, "model"_v = model,
             "temperature"_v = temperature, "image_detail"_v = image_detail;
    }};
  }
};

} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_CLASSIFY_H
