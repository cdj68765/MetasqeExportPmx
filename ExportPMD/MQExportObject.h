#pragma once

#define NOMINMAX
#include <windows.h>
#include "MQPlugin.h"
#include "MQ3DLib.h"
#include <vector>


class MQExportObject {
public:
	struct MExportVertex {
		int vi; // original vertex index
		MQPoint normal;
		MQCoordinate t;
		DWORD col;
	};

	struct MTexFace {
		int count;
	};

	struct MSeparateParam {
		bool SeparateUV;
		bool SeparateVertexColor;
		bool SeparateNormal;

		MSeparateParam(){
			SeparateUV = false;
			SeparateVertexColor = false;
			SeparateNormal = false;
		}
	};

public:
	MQExportObject(MQObject obj, const MSeparateParam& separate_param);
	~MQExportObject();

	int GetVertexCount() const { return m_vc; }
	int GetOriginalVertex(int vi);
	MQPoint GetVertexNormal(int vi);
	MQCoordinate GetVertexCoordinate(int vi);
	DWORD GetVertexColor(int vi);
	int GetVertexRelatedFaces(int vi, int *array);

	int GetFaceCount() const { return m_fc; }
	int GetFacePointCount(int fi);
	void GetFacePointArray(int fi, int *array);

private:
	MExportVertex *m_v;
	MTexFace *m_f;
	MQApexValueBase<int> m_vi;
	std::vector< std::vector<int> > vert_faces;
	int m_vc,m_fc;

private:
	struct MVertexHash {
		int vi;
		MQPoint normal;
		MQCoordinate uv;
		DWORD col;
		int next;
	};
};

