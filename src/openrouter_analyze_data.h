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

#ifndef DOCWIRE_OPENROUTER_ANALYZE_DATA_H
#define DOCWIRE_OPENROUTER_ANALYZE_DATA_H

#include "log_scope.h"
#include "openrouter_task.h"
#include "serialization_enum.h" // IWYU pragma: keep
#include <string>
namespace docwire {
namespace openrouter {

class DOCWIRE_OPENROUTER_EXPORT analyze_data : public task {
public:
  	explicit analyze_data(const std::string &api_key,
               const std::string &model = "openrouter/auto",
               float temperature = 0,
               image_detail image_detail = image_detail::automatic)
      : task("Your task is analyze data in every message and create a summary "
             "highlighting the most important insights, trends, key patterns, "
             "statistics, findings and other revelant information. Include "
             "conclusions that are usually made for type of data you will find "
             "in message.",
             api_key, model, temperature, image_detail) {
    DOCWIRE_LOG_SCOPE(model, temperature, image_detail);
  }
};

} // namespace openrouter
} // namespace docwire

#endif // DOCWIRE_OPENROUTER_ANALYZE_DATA_H
