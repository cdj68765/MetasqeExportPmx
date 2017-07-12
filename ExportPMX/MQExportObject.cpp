#include "MQExportObject.h"

MQExportObject::MQExportObject(MQObject obj, const MSeparateParam& separate_param)
{
	int org_vc = obj->GetVertexCount();
	int i, j;

	// allocate faces
	m_fc = obj->GetFaceCount();
	m_f = new(std::nothrow) MTexFace[m_fc];

	m_vi.resize(m_fc);

	// allocate vertices
	int hashsize = 0;
	for (i = 0; i < m_fc; i++)
	{
		int pn = obj->GetFacePointCount(i);
		hashsize += obj->GetFacePointCount(i);
		m_vi.resizeItem(i, pn);
	}
	m_v = new(std::nothrow) MExportVertex[hashsize];
	m_vc = 0;
	hashsize += org_vc;

	if (m_fc == 0 && org_vc == 0)
		return;

	MQObjNormal* normal = new MQObjNormal(obj);

	// allocate hash
	int hc = org_vc;
	MVertexHash* hash = new(std::nothrow) MVertexHash[hashsize];
	MVertexHash* ch;
	for (i = 0 , ch = hash; i < org_vc; i++ , ch++)
	{
		ch->vi = -1;
		ch->next = -1;
	}

	std::vector<int> expvert_hash;

	for (i = 0; i < m_fc; i++)
	{
		m_f[i].count = obj->GetFacePointCount(i);

		std::vector<int> ptarray(m_f[i].count);
		obj->GetFacePointArray(i, ptarray.data());

		std::vector<MQCoordinate> uvarray(m_f[i].count);
		obj->GetFaceCoordinateArray(i, uvarray.data());

		for (j = 0; j < m_f[i].count; j++)
		{
			int chi = ptarray[j];
			for (; hash[chi].vi >= 0; chi = hash[chi].next)
			{
				bool dif = false;
				if (separate_param.SeparateNormal)
				{
					if (normal->Get(i, j) != hash[chi].normal)
					{
						dif = true;
					}
				}
				if (separate_param.SeparateUV)
				{
					if (uvarray[j] != hash[chi].uv)
					{
						dif = true;
					}
				}
				if (separate_param.SeparateVertexColor)
				{
					if (obj->GetFaceVertexColor(i, j) != hash[chi].col)
					{
						dif = true;
					}
				}
				if (!dif)
				{
					m_vi[i][j] = hash[chi].vi;
					if (!separate_param.SeparateNormal)
					{
						hash[chi].normal += normal->Get(i, j);
					}
					goto NEXT_VERTEX;
				}
				if (hash[chi].next < 0)
				{
					hash[hc].vi = -1;
					hash[hc].next = -1;
					hash[chi].next = hc++;
				}
			}
			hash[chi].vi = m_vc;
			hash[chi].normal = normal->Get(i, j);
			hash[chi].uv = uvarray[j];
			hash[chi].col = obj->GetFaceVertexColor(i, j);
			m_v[m_vc].vi = ptarray[j];
			expvert_hash.push_back(chi);
			m_vi[i][j] = m_vc++;
		NEXT_VERTEX:;
		}
	}

	for (i = 0; i < m_vc; i++)
	{
		int chi = expvert_hash[i];
		m_v[i].normal = hash[chi].normal;
		if (!separate_param.SeparateNormal)
		{
			m_v[i].normal.normalize();
		}
		m_v[i].t = hash[chi].uv;
		m_v[i].col = hash[chi].col;
	}

	vert_faces.resize(m_vc);
	for (i = 0; i < m_fc; i++)
	{
		for (j = 0; j < m_f[i].count; j++)
		{
			vert_faces[m_vi[i][j]].push_back(i);
		}
	}

	delete normal;
	delete[] hash;
}

MQExportObject::~MQExportObject()
{
	delete[] m_v;
	delete[] m_f;
}

int MQExportObject::GetOriginalVertex(int vi)
{
	return m_v[vi].vi;
}

MQPoint MQExportObject::GetVertexNormal(int vi)
{
	return m_v[vi].normal;
}

MQCoordinate MQExportObject::GetVertexCoordinate(int vi)
{
	return m_v[vi].t;
}

DWORD MQExportObject::GetVertexColor(int vi)
{
	return m_v[vi].col;
}

int MQExportObject::GetVertexRelatedFaces(int vi, int* array)
{
	if (array != nullptr)
	{
		for (size_t i = 0; i < vert_faces[vi].size(); i++)
		{
			array[i] = vert_faces[vi][i];
		}
	}
	return (int)vert_faces[vi].size();
}

int MQExportObject::GetFacePointCount(int fi)
{
	return m_f[fi].count;
}

void MQExportObject::GetFacePointArray(int fi, int* array)
{
	for (int i = 0; i < m_f[fi].count; i++)
	{
		array[i] = m_vi[fi][i];
	}
}
