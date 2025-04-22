#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include "../../Struct.h"

/// <summary>
/// Mtlファイルを読み込む
/// </summary>
/// <param name="directoryPath"></param>
/// <param name="filename"></param>
/// <returns></returns>
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

/// <summary>
/// Objファイルを読み込む
/// </summary>
/// <param name="directoryPath"></param>
/// <param name="filename"></param>
/// <returns></returns>
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);