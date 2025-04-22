#include "ModelData.h"

/// <summary>
/// Mtlファイルを読み込む
/// </summary>
/// <param name="directoryPath"></param>
/// <param name="filename"></param>
/// <returns></returns>
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	/*-----------------------------------
		必要な変数の宣言と、ファイルを開く
	-----------------------------------*/

	// マテリアルデータ
	MaterialData materialData;

	// ファイルから読んだ1行を格納するもの
	std::string line;

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

/// <summary>
/// Objファイルを読み込む
/// </summary>
/// <param name="directoryPath"></param>
/// <param name="filename"></param>
/// <returns></returns>
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	/*----------------------------------
	    必要な変数の宣言とファイルを開く
	----------------------------------*/

	// 構築するモデルデータ
	ModelData modelData;

	// 位置
	std::vector<Vector4> positions;

	// 法線
	std::vector<Vector3> normals;

	// テクスチャ座標
	std::vector<Vector2> texcoords;

	// ファイルから読んだ1行を格納するもの
	std::string line;


	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());


	/*--------------------------------------
	    ファイルを読み、モデルデータを構築する
	--------------------------------------*/

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);

		// 先頭の識別子を読む
		s >> identifier;

		// 頂点位置
		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt")
		{
			// テクスチャ座標
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			// 頂点法線
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f")
		{
			// 面

			VertexData triangle[3];

			// 面は三角形限定
			for (int32_t feceVertex = 0; feceVertex < 3; ++feceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;

				// 頂点の要素へのIndexは、位置/UV/法線　で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];

				for (uint32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;

				triangle[feceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			// MaterialTemplateLibraryファイル名を取得する
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}