#include "filter.h"

namespace jaf
{
namespace log
{

Filter::Filter(uint32_t min_level)
    : min_level_(min_level)
{
}

bool Filter::Filtration(const IEvent& log_event)
{
    return min_level_ >= log_event.Level();
}

} // namespace log
} // namespace jaf