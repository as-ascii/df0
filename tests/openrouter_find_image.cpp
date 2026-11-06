#include "docwire.h"
#include "environment.h"
#include <sstream>

int main(int argc, char* argv[])
{
  using namespace docwire;
  std::stringstream out_stream;

  try
  {
    std::filesystem::path("scene_1.png") | content_type::detector{} | openrouter::find("tree", environment::get("OPENROUTER_API_KEY").value()) | out_stream;
    ensure(fuzzy_match::ratio(out_stream.str(), "2\n- A tree is located on the left side of the image near the people.\n- Another tree is in the background near the center of the image.\n")) > 50;
  }
  catch (const std::exception& e)
  {
    std::cerr << errors::diagnostic_message(e) << std::endl;
    return 1;
  }

  return 0;
}
