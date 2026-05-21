#pragma once
#include "types.hpp"
#include <iostream>

namespace DataCollector::Utils {
/**
 * @brief Выводит форматированное сообщение в консоль с временной меткой.
 * * Автоматически направляет ошибки (уровень "ERROR") в std::cerr,
 * а остальные сообщения (INFO, WARNING) в std::cout.
 * @param component Имя компонента (например, "UdsServer" или "DataCollector").
 * @param level Уровень важности (INFO, WARNING, ERROR).
 * @param msg Текст сообщения.
 */
void log(const str& component, const str& level, const str& msg);
}; // namespace DataCollector::Utils