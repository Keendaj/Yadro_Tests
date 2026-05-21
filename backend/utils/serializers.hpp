#pragma once
#include <nlohmann/json_fwd.hpp>

#include "data_structures.hpp"
#include "types.hpp"

namespace DataCollector {
void to_json(nlohmann::json& j, const SystemRAMData& ram);
void to_json(nlohmann::json& j, const ProcessData& proc);
void to_json(nlohmann::json& j, const SystemData& data);
}